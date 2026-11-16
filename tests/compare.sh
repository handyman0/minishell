#!/bin/bash
# Compara a saida e o status do minishell com o bash. Uso: tests/compare.sh [binario]

SHELL_BIN=$(realpath "${1:-./minishell}")
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=$(mktemp -d /tmp/minishell_cmp.XXXXXX)
export LC_ALL=C
pass=0
fail=0

cd "$WORK" || exit 2
touch a.txt b.txt c.log f
mkdir -p sub && touch sub/x.c sub/y.c

while IFS= read -r line; do
	[ -z "$line" ] && continue
	expected=$(bash -c "$line" 2> /dev/null < /dev/null; echo "rc=$?")
	got=$(timeout 10 "$SHELL_BIN" -c "$line" 2> /dev/null < /dev/null; echo "rc=$?")
	if [ "$expected" = "$got" ]; then
		pass=$((pass + 1))
	else
		fail=$((fail + 1))
		echo "DIFERE: $line"
		echo "   bash: $(echo "$expected" | tr '\n' '|')"
		echo "   mini: $(echo "$got" | tr '\n' '|')"
	fi
done < "$ROOT/tests/cases.txt"

echo "comparacao com o bash: $pass iguais, $fail diferentes"
[ "$fail" -eq 0 ]
