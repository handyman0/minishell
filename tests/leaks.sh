#!/bin/bash
# Verifica vazamentos de memoria com valgrind. Uso: tests/leaks.sh [binario]
# O binario precisa ser dinamico (make dynamic): o valgrind nao intercepta o
# malloc de um binario estatico.
#
# Dois passes por caso:
#  1) processo principal, estrito: qualquer bloco nao liberado e erro;
#  2) com filhos (pipes, subshells, $(...)): so vazamentos reais (perdidos,
#     indiretos, possiveis). A memoria ainda alcancavel de um filho que acabou
#     de nascer de um fork nao e vazamento.

SHELL_BIN=$(realpath "${1:-./minishell}")
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=$(mktemp -d /tmp/minishell_leaks.XXXXXX)
BAD=0
TOTAL=0

command -v valgrind > /dev/null || { echo "valgrind nao encontrado"; exit 2; }
cd "$WORK" || exit 2
touch a.txt b.txt c.log f
mkdir -p sub && touch sub/x.c

SKIP="--trace-children-skip=/usr/bin/*,/bin/*,/usr/local/bin/*"
VG_MAIN="valgrind -q --leak-check=full --show-leak-kinds=all \
--errors-for-leak-kinds=all --error-exitcode=99 --child-silent-after-fork=yes"
VG_KIDS="valgrind -q --leak-check=full --show-leak-kinds=definite,indirect,possible \
--errors-for-leak-kinds=definite,indirect,possible --error-exitcode=99 \
--trace-children=yes $SKIP"

failed() {
	[ "$1" -eq 99 ] || grep -q "lost\|Invalid\|uninitialised\|reachable" "$2"
}

check_case() {
	local label=$1 input=$2 mode=$3
	TOTAL=$((TOTAL + 1))
	if [ "$mode" = "-c" ]; then
		timeout 60 $VG_MAIN "$SHELL_BIN" -c "$input" < /dev/null > /dev/null 2> vg1.txt
		rc1=$?
		timeout 60 $VG_KIDS "$SHELL_BIN" -c "$input" < /dev/null > /dev/null 2> vg2.txt
		rc2=$?
	else
		timeout 60 $VG_MAIN "$SHELL_BIN" < "$input" > /dev/null 2> vg1.txt
		rc1=$?
		timeout 60 $VG_KIDS "$SHELL_BIN" < "$input" > /dev/null 2> vg2.txt
		rc2=$?
	fi
	if failed $rc1 vg1.txt || failed $rc2 vg2.txt; then
		BAD=$((BAD + 1))
		echo "VAZAMENTO/ERRO: $label"
		grep -h -m3 "lost\|Invalid\|uninitialised\|reachable" vg1.txt vg2.txt
	fi
}

while IFS= read -r line; do
	[ -z "$line" ] && continue
	check_case "$line" "$line" -c
done < "$ROOT/tests/cases.txt"

printf 'alias ll="echo LL"\nll 1 | cat\nunalias ll\nexport A=1\ncat <<EOF\nv=$A\nEOF\n(echo a; echo b) | cat\ntrue &\necho $(echo sub)\nA=$(echo x)\necho fim\n' > session.txt
check_case "sessao de script" session.txt "<"

echo "valgrind: $TOTAL execucoes, $BAD com vazamento ou erro"
[ "$BAD" -eq 0 ]
