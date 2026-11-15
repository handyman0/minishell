#!/bin/bash
# Verifica vazamentos de memoria com valgrind. Uso: tests/leaks.sh [binario]

SHELL_BIN=$(realpath "${1:-./minishell}")
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=$(mktemp -d /tmp/minishell_leaks.XXXXXX)
BAD=0
TOTAL=0

command -v valgrind > /dev/null || { echo "valgrind nao encontrado"; exit 2; }
cd "$WORK" || exit 2
touch a.txt b.txt c.log f
mkdir -p sub && touch sub/x.c

VG="valgrind -q --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
--error-exitcode=99 --trace-children=yes --trace-children-skip=/usr/bin/*,/bin/*,/usr/local/bin/*"

while IFS= read -r line; do
	TOTAL=$((TOTAL + 1))
	timeout 60 $VG "$SHELL_BIN" -c "$line" < /dev/null > /dev/null 2> vg.txt
	rc=$?
	if [ $rc -eq 99 ] || grep -q "definitely lost\|indirectly lost\|Invalid\|uninitialised" vg.txt; then
		BAD=$((BAD + 1))
		echo "VAZAMENTO/ERRO: $line"
		grep -m3 "lost\|Invalid\|uninitialised" vg.txt
	fi
done < "$ROOT/tests/cases.txt"

printf 'alias ll="echo LL"\nll 1 | cat\nunalias ll\nexport A=1\ncat <<EOF\nv=$A\nEOF\n(echo a; echo b) | cat\ntrue &\necho fim\n' > session.txt
TOTAL=$((TOTAL + 1))
timeout 60 $VG "$SHELL_BIN" < session.txt > /dev/null 2> vg.txt
if [ $? -eq 99 ] || grep -q "lost\|Invalid\|uninitialised" vg.txt; then
	BAD=$((BAD + 1)); echo "VAZAMENTO/ERRO na sessao de script"; head -20 vg.txt
fi

echo "valgrind: $TOTAL execucoes, $BAD com vazamento ou erro"
[ "$BAD" -eq 0 ]
