#!/bin/bash
# Teste de estresse do minishell. Uso: tests/stress.sh [binario] [iteracoes]
# Rode com o binario compilado por "make asan" para pegar erros de memoria.

SHELL_BIN=$(realpath "${1:-./minishell_asan}")
ITER=${2:-1500}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=$(mktemp -d /tmp/minishell_stress.XXXXXX)
FAILS="$WORK/failures.txt"
TOTAL=0
BAD=0

export ASAN_OPTIONS=detect_leaks=1:exitcode=99
export UBSAN_OPTIONS=print_stacktrace=1

cc -O1 -o "$WORK/fuzz" "$ROOT/tests/fuzz.c" || exit 2
cd "$WORK" || exit 2
touch a.txt b.txt c.log f
mkdir -p sub && touch sub/x.c sub/y.c

check() {
	local label=$1 rc=$2 errfile=$3
	TOTAL=$((TOTAL + 1))
	if grep -q "Sanitizer\|runtime error" "$errfile" 2>/dev/null \
		|| [ "$rc" -eq 124 ] || [ "$rc" -eq 99 ] || [ "$rc" -eq 134 ] \
		|| [ "$rc" -eq 136 ] || [ "$rc" -eq 139 ]; then
		BAD=$((BAD + 1))
		{
			echo "--- [$label] rc=$rc"
			head -c 300 "$errfile" | grep -m3 "SUMMARY\|runtime error\|ERROR"
		} >> "$FAILS"
		return 1
	fi
	return 0
}

run_line() {
	local label=$1 line=$2
	timeout 10 "$SHELL_BIN" -c "$line" < /dev/null > /dev/null 2> err.txt
	local rc=$?
	if ! check "$label" $rc err.txt; then
		printf '%s\n' "$line" | head -c 400 >> "$FAILS"
		echo >> "$FAILS"
	fi
}

section() { echo; echo "== $1"; }

section "1. corpus de casos"
while IFS= read -r line; do
	run_line corpus "$line"
done < "$ROOT/tests/cases.txt"
echo "corpus: $TOTAL executados, $BAD problemas"

section "2. fuzz por gramatica ($ITER linhas)"
"$WORK/fuzz" gen 1 "$ITER" > gen.txt
while IFS= read -r line; do run_line gen "$line"; done < gen.txt
echo "acumulado: $TOTAL executados, $BAD problemas"

section "3. fuzz por mutacao do corpus ($ITER linhas)"
"$WORK/fuzz" mutate 2 "$ITER" "$ROOT/tests/cases.txt" > mut.txt
while IFS= read -r line; do run_line mutate "$line"; done < mut.txt
echo "acumulado: $TOTAL executados, $BAD problemas"

section "4. ruido binario ($((ITER / 2)) linhas)"
"$WORK/fuzz" noise 3 $((ITER / 2)) > noise.txt
while IFS= read -r line; do run_line noise "$line"; done < noise.txt
echo "acumulado: $TOTAL executados, $BAD problemas"

section "5. sessao unica com estado acumulado (fuzz via stdin)"
"$WORK/fuzz" gen 4 "$ITER" > session.txt
timeout 120 "$SHELL_BIN" < session.txt > /dev/null 2> err.txt
check sessao $? err.txt || echo "sessao falhou"
"$WORK/fuzz" mutate 5 "$ITER" "$ROOT/tests/cases.txt" > session2.txt
timeout 120 "$SHELL_BIN" < session2.txt > /dev/null 2> err.txt
check sessao2 $? err.txt || echo "sessao2 falhou"
echo "acumulado: $TOTAL executados, $BAD problemas"

section "6. limites e entradas gigantes"
big() {
	local label=$1; shift
	printf "%s\n" "$1" | timeout 60 "$SHELL_BIN" > /dev/null 2> err.txt
	local rc=$?
	check "$label" $rc err.txt || { echo "FALHOU: $label (rc=$rc)"; echo "$label" >> "$FAILS"; }
}
LONG=$(head -c 1000000 /dev/zero | tr '\0' 'a')
big "linha de 1MB" "echo $LONG"
big "1MB entre aspas" "echo \"$LONG\""
big "variavel de 1MB" "export V=$LONG; echo \$V | wc -c; echo \"\$V\" | wc -c"
ARGS=$(seq 1 50000 | tr '\n' ' ')
big "50000 argumentos" "echo $ARGS | wc -w"
big "50000 args com glob" "echo $ARGS * | wc -w"
big "parenteses aninhados 400" "$(printf '(%.0s' $(seq 400))echo ok$(printf ')%.0s' $(seq 400))"
big "parenteses desbalanceados" "$(printf '(%.0s' $(seq 2000))"
big "2000 pontos e virgulas" "$(printf 'echo a;%.0s' $(seq 2000))"
big "2000 && encadeados" "true$(printf ' && true%.0s' $(seq 2000))"
big "2000 || encadeados" "false$(printf ' || false%.0s' $(seq 2000))"
big "1000 redirecionamentos" "echo x $(printf '> sub/o%.0s ' $(seq 1000))"
big "5000 aspas abertas" "$(printf "'%.0s" $(seq 5001))"
big "cifrao repetido" "echo $(printf '$%.0s' $(seq 20000))"
big "chaves repetidas" "echo $(printf '${%.0s' $(seq 20000))"
big "20000 variaveis" "echo $(printf '$HOME%.0s' $(seq 20000)) | wc -c"
big "20000 aliases na linha" "alias e=echo; $(printf 'e a;%.0s' $(seq 20000))"
big "wildcards patologicos" "echo $(printf '*%.0s' $(seq 200))a"
big "wildcards em colchete" "echo $(printf '[a-z]*%.0s' $(seq 50))"
big "ruido de operadores" "$(printf '|&;<>()%.0s' $(seq 3000))"
(
	ulimit -u 3000 2>/dev/null
	big "pipeline com 100 estagios" "echo oi$(printf ' | cat%.0s' $(seq 100)) | wc -c"
	big "200 jobs em background" "$(printf 'true & %.0s' $(seq 200))echo ok"
	big "subshells 100 niveis" "$(printf '(%.0s' $(seq 100))echo x$(printf ')%.0s' $(seq 100))"
)
mkdir -p many && (cd many && seq 1 10000 | sed 's/^/arq_/' | xargs touch)
big "glob de 10000 arquivos" "cd many; echo arq_* | wc -w; echo arq_1* | wc -w; ls | wc -l"
echo "acumulado: $TOTAL executados, $BAD problemas"

section "7. alias recursivo e cadeias"
big "alias circular" "alias a=b; alias b=a; a"
big "alias auto-referente" "alias ls='ls -d'; ls"
big "cadeia de 50 aliases" "$(for i in $(seq 1 50); do printf 'alias a%d=a%d;' $i $((i + 1)); done) a1"

section "8. descritores de arquivo e zumbis"
FIFO="$WORK/fifo"
mkfifo "$FIFO"
"$SHELL_BIN" < "$FIFO" > /dev/null 2> err_fd.txt &
SPID=$!
exec 3> "$FIFO"
sleep 1
FD_BEFORE=$(ls /proc/$SPID/fd | wc -l)
for i in $(seq 1 300); do
	echo "echo $i | cat > /dev/null; true && false || echo x > /dev/null; (echo y) > /dev/null; true & " >&3
done
echo "echo heredoc <<EOF > /dev/null" >&3
echo "x" >&3
echo "EOF" >&3
sleep 4
FD_AFTER=$(ls /proc/$SPID/fd | wc -l)
ZOMBIES=$(ps --ppid $SPID -o stat= | grep -c '^Z')
echo "fds antes=$FD_BEFORE depois=$FD_AFTER zumbis=$ZOMBIES"
TOTAL=$((TOTAL + 1))
if [ "$FD_AFTER" -gt $((FD_BEFORE + 2)) ]; then
	BAD=$((BAD + 1)); echo "VAZAMENTO DE FD: $FD_BEFORE -> $FD_AFTER" | tee -a "$FAILS"
fi
echo "exit" >&3
exec 3>&-
wait $SPID
check fd_sessao $? err_fd.txt || echo "sessao fd falhou"

section "9. editor de linha (teclas aleatorias em pty)"
if command -v script > /dev/null; then
	mkdir -p "$WORK/home"
	for seed in 1 2 3 4; do
		"$WORK/fuzz" keys $seed 2500 > keys.bin
		size=$(stat -c %s keys.bin)
		(
			o=0
			while [ $o -lt $size ]; do
				tail -c +$((o + 1)) keys.bin | head -c 48
				sleep 0.004
				o=$((o + 48))
			done
			printf '\003\003\003exit\r'
		) | HOME="$WORK/home" timeout 90 script -qec "$SHELL_BIN" /dev/null > keys_out.bin 2> keys_err.txt
		rc=$?
		cat keys_out.bin >> keys_err.txt
		check "teclas-$seed" $rc keys_err.txt || echo "editor: semente $seed falhou (rc=$rc)"
	done
	echo "acumulado: $TOTAL executados, $BAD problemas"
else
	echo "script nao encontrado, secao ignorada"
fi

section "RESULTADO"
echo "executados: $TOTAL   problemas: $BAD"
if [ "$BAD" -gt 0 ]; then
	echo "detalhes em $FAILS"
	cat "$FAILS" | head -80
	exit 1
fi
echo "limpo. Trabalho em $WORK"
exit 0
