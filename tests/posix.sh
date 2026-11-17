#!/bin/bash
# posix.sh - testador interno do minishell.
#
# Executa casos de teste no formato .tst (um script de shell que chama
# test_oE, test_OE etc.) contra o binario e relata o que passou e o que falhou.
#
# Uso: tests/posix.sh [-b binario] [-t segundos] [-k filtro] [-v] [-l] [arquivos.tst]
#   -b  binario a testar (padrao: ./minishell)
#   -t  tempo maximo por caso, em segundos (padrao: 5)
#   -k  roda so os casos cujo nome contem o filtro
#   -v  mostra a diferenca esperada/obtida de cada falha
#   -l  lista cada falha (arquivo:linha: nome e motivo)
#
# Formato de um caso:
#   test_oE -e 0 'nome do caso'      (o = confere stdout, O = stdout vazio,
#   comandos do teste                 e = confere stderr, E = stderr vazio,
#   __IN__                            x = nao confere nenhum dos dois)
#   saida esperada
#   __OUT__
# Opcoes do caso: -e N (status), -e n (qualquer status != 0), -e SINAL, -d (exige
# mensagem de erro em stderr).

BIN=./minishell
TIMEOUT=5
FILTER=
VERBOSE=0
LIST=0
ROOT=$(cd "$(dirname "$0")/.." && pwd)

while getopts b:t:k:vl opt; do
	case $opt in
		b) BIN=$OPTARG ;;
		t) TIMEOUT=$OPTARG ;;
		k) FILTER=$OPTARG ;;
		v) VERBOSE=1 ;;
		l) LIST=1 ;;
		*) exit 64 ;;
	esac
done
shift $((OPTIND - 1))

BIN=$(realpath "$BIN") || exit 2
[ -x "$BIN" ] || { echo "binario nao encontrado: $BIN"; exit 2; }
if [ $# -eq 0 ]; then
	set -- "$ROOT"/tests/*.tst
fi

RUN=$(mktemp -d /tmp/minishell_posix.XXXXXX)
RESULTS="$RUN/results.txt"
: > "$RESULTS"
: > "$RUN/detalhes.txt"
trap 'chmod -R u+rwx "$RUN" 2> /dev/null; rm -rf "$RUN"' EXIT

testee() { "$BIN" "$@"; }
SETUP=
setup() {
	case "${1--}" in
		-)  setup "$(cat)" ;;
		-d) setup "$(printf '%s\n' \
				"_empty= _sp=' ' _tab='	' _nl='" "'" \
				'echoraw() { printf "%s\n" "$*"; }' \
				'bracket() { if [ $# -gt 0 ]; then printf "[%s]" "$@"; fi; echo; }')" ;;
		*)  SETUP="$SETUP
$1" ;;
	esac
}

record() {
	printf "%s\t%s\t%s\t%s\t%s\n" "$1" "$_t_file" "$2" "$3" "$4" >> "$RESULTS"
}

check_status() {
	local want=$1 got=$2 sig
	case $want in
		'') return 0 ;;
		n)  [ "$got" -ne 0 ] ;;
		[A-Za-z]*)
			[ "$got" -gt 128 ] || return 1
			sig=$(kill -l "$got" 2> /dev/null)
			[ "$sig" = "$want" ] ;;
		*)  [ "$got" -eq "$want" ] ;;
	esac
}

testcase() {
	local _t_line=$1 _t_diag=0 _t_want='' _t_name _t_reasons='' _t_got
	shift
	OPTIND=1
	while getopts de: o; do
		case $o in
			d) _t_diag=1 ;;
			e) _t_want=$OPTARG ;;
		esac
	done
	shift $((OPTIND - 1))
	_t_name=$1
	shift
	{ [ -n "$SETUP" ] && printf '%s\n' "$SETUP"; cat <&3; } > "in.$_t_line"
	chmod u+rw "in.$_t_line" 2> /dev/null
	: > "expected_out.$_t_line"
	: > "expected_err.$_t_line"
	local _t_cmp_out=0 _t_cmp_err=0
	if { : <&4; } 2> /dev/null; then
		cat <&4 > "expected_out.$_t_line"
		_t_cmp_out=1
	fi
	if { : <&5; } 2> /dev/null; then
		cat <&5 > "expected_err.$_t_line"
		_t_cmp_err=1
	fi
	chmod u+rw "expected_out.$_t_line" "expected_err.$_t_line" 2> /dev/null
	if [ -n "${skip-}" ] || { [ -n "$FILTER" ] && [[ $_t_name != *"$FILTER"* ]]; }; then
		record SKIP "$_t_line" "$_t_name"
		return 0
	fi
	( [ ${#_t_umask[@]} -gt 0 ] && builtin umask "${_t_umask[@]}"
	  exec timeout -k 1 "$TIMEOUT" "$TESTEE" "$@" ) < "in.$_t_line" \
		> "out.$_t_line" 2> "err.$_t_line"
	_t_got=$?
	chmod u+rw "out.$_t_line" "err.$_t_line" 2> /dev/null
	[ "$_t_got" -eq 124 ] && _t_reasons="tempo esgotado"
	check_status "$_t_want" "$_t_got" \
		|| _t_reasons="$_t_reasons status esperado=$_t_want obtido=$_t_got"
	if [ "$_t_cmp_out" -eq 1 ] && ! cmp -s "out.$_t_line" "expected_out.$_t_line"; then
		_t_reasons="$_t_reasons stdout"
	fi
	if [ "$_t_cmp_err" -eq 1 ] && ! cmp -s "err.$_t_line" "expected_err.$_t_line"; then
		_t_reasons="$_t_reasons stderr"
	fi
	if [ "$_t_diag" -eq 1 ] && [ ! -s "err.$_t_line" ]; then
		_t_reasons="$_t_reasons sem-mensagem-de-erro"
	fi
	if [ -z "$_t_reasons" ]; then
		record PASS "$_t_line" "$_t_name"
		return 0
	fi
	record FAIL "$_t_line" "$_t_name" "$_t_reasons"
	if [ "$VERBOSE" -eq 1 ]; then
		{ printf "%s:%s: %s (%s)\n" "$_t_file" "$_t_line" "$_t_name" "$_t_reasons"
		  diff "expected_out.$_t_line" "out.$_t_line" 2> /dev/null | head -6
		  diff "expected_err.$_t_line" "err.$_t_line" 2> /dev/null | head -4; } >> "$RUN/detalhes.txt"
	fi
}

shopt -s expand_aliases
alias test_x='testcase "$LINENO" 3<<\__IN__'
alias test_o='testcase "$LINENO" 3<<\__IN__ 4<<\__OUT__'
alias test_O='testcase "$LINENO" 3<<\__IN__ 4</dev/null'
alias test_e='testcase "$LINENO" 3<<\__IN__ 5<<\__ERR__'
alias test_oe='testcase "$LINENO" 3<<\__IN__ 4<<\__OUT__ 5<<\__ERR__'
alias test_Oe='testcase "$LINENO" 3<<\__IN__ 4</dev/null 5<<\__ERR__'
alias test_E='testcase "$LINENO" 3<<\__IN__ 5</dev/null'
alias test_oE='testcase "$LINENO" 3<<\__IN__ 4<<\__OUT__ 5</dev/null'
alias test_OE='testcase "$LINENO" 3<<\__IN__ 4</dev/null 5</dev/null'

run_file() {
	local file work
	file=$(realpath "$1")
	work="$RUN/work_$(basename "$1" .tst)"
	mkdir -p "$work"
	(
		cd "$work" || exit 2
		ln -s "$BIN" sh
		export TESTEE="$PWD/sh"
		export LC_ALL=C
		_t_file=$(basename "$file")
		SETUP=
		exec < /dev/null 3>&- 4>&- 5>&-
		_t_umask=()
		umask() { _t_umask=("$@"); }
		. "$file"
	) > /dev/null 2>&1
}

for f in "$@"; do
	run_file "$f"
done

awk -F'\t' '
	{ n[$2]++; c[$1 SUBSEP $2]++; tot[$1]++ }
	END {
		for (f in n) {
			p = c["PASS" SUBSEP f]; k = c["SKIP" SUBSEP f]; t = n[f] - k
			printf "%5.1f%%  %3d/%-3d  %s\n", (t ? 100 * p / t : 0), p, t, f
		}
	}' "$RESULTS" | sort -rn

total=$(awk -F'\t' '$1!="SKIP"' "$RESULTS" | wc -l)
pass=$(awk -F'\t' '$1=="PASS"' "$RESULTS" | wc -l)
skip=$(awk -F'\t' '$1=="SKIP"' "$RESULTS" | wc -l)
echo
echo "passaram $pass de $total casos (ignorados: $skip)"
if [ "$LIST" -eq 1 ]; then
	echo; awk -F"\t" '$1=="FAIL" {printf "FALHOU %s:%s: %s (%s)\n", $2, $3, $4, $5}' "$RESULTS"
fi
[ "$VERBOSE" -eq 1 ] && [ -f "$RUN/detalhes.txt" ] && cat "$RUN/detalhes.txt"
[ "$pass" -eq "$total" ]
