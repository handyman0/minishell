#!/bin/bash
# Testa o Tab de opcoes e subcomandos num terminal simulado (script).
# Uso: tests/complete.sh [binario]
BIN=$(realpath "${1:-./minishell}")
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 2
printf 'all: x\nclean:\n\t@:\nbuild-it:\n.PHONY: all\nVAR = a:b\n' > Makefile
fail=0
total=0

caso() {
	local entrada=$1 esperado=$2 saida
	total=$((total + 1))
	saida=$( (sleep 0.5; printf "$entrada"; sleep 2; printf '\003\003exit\r') \
		| HOME="$WORK" timeout 15 script -qec "$BIN" /dev/null 2>&1 \
		| sed 's/\x1b\[[0-9;?]*[a-zA-Z]//g' | tr -d '\r')
	if grep -q 'Sanitizer\|runtime error' <<< "$saida"; then
		echo "FALHOU: sanitizer em '$entrada'"
		grep -m3 'SUMMARY\|ERROR' <<< "$saida"
		fail=$((fail + 1))
	elif ! grep -qF -- "$esperado" <<< "$saida"; then
		echo "FALHOU: '$entrada' nao mostrou '$esperado'"
		fail=$((fail + 1))
	fi
}

caso 'ls --almost-a\t' 'ls --almost-all'
caso 'ls --co\t\t' '--context'
caso 'grep --colo\t\t' '--colour'
caso 'cd -\t\t' '-L'
caso 'echo -\t\t' '-n'
caso 'make cl\t' 'make clean'
caso 'make \t\t' 'build-it'
if command -v git > /dev/null; then
	caso 'git sta\t\t' 'stash'
	caso 'git commit --am\t' 'git commit --amend'
fi
echo "complete: $((total - fail)) de $total"
[ $fail -eq 0 ]
