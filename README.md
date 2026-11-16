<div align="center">

# minishell

**Um shell Unix escrito do zero em C, feito para uso diário.**
Abre em ~2 ms, edita a linha com sugestões e destaque de sintaxe, controla jobs
e foi testado contra o bash, com fuzz e com Valgrind.

![C](https://img.shields.io/badge/Language-C-blue.svg)
![Deps](https://img.shields.io/badge/Deps-libc%20%2B%20mylib-success)
![Binary](https://img.shields.io/badge/Binary-~850%20KB%20est%C3%A1tico-informational)
![License](https://img.shields.io/badge/License-42%20School-lightgrey)

[Destaques](#-destaques) • [Instalação](#-instalação) • [Linguagem](#-linguagem) • [Editor de linha](#-editor-de-linha) • [Configuração](#-configuração) • [Testes](#-testes) • [Roteiro](#-roteiro)

</div>

---

## ✨ Destaques

- **Rápido de abrir.** Cerca de 2,3 ms por execução, contra ~2,7 ms do bash e ~40 ms
  do fish (medido nesta máquina com `-c true`, melhor de 5 rodadas de 500).
- **Leve e sem dependências externas.** Só a libc e a **mylib**, a biblioteca própria
  do projeto (derivada da libft, que continua intacta em outro repositório). O editor
  de linha, o casador de padrões, a saída de mensagens e o leitor de linhas são código
  próprio, e o binário é estático (~850 KB).
- **Editor de linha completo, escrito do zero.** Sugestão em cinza a partir do
  histórico, destaque de sintaxe em tempo real, `Tab` para comandos, arquivos e
  variáveis, `Ctrl+R`, histórico filtrado pelo que você digitou e comandos em
  várias linhas.
- **Job control de verdade.** `Ctrl+Z`, `jobs`, `fg`, `bg`, `&` e aviso quando um
  job termina.
- **Testado a sério.** 207 casos comparados com o bash, fuzz por gramática,
  mutação e ruído binário, entradas de 1 MB, teclas aleatórias no editor e
  Valgrind. Os testes já acharam e consertaram bugs reais (veja o histórico).

## 📦 Instalação

```sh
make            # gera ./minishell (estático)
make install    # copia para ~/.local/bin/minishell
./minishell     # shell interativo
./minishell -c 'echo oi | tr a-z A-Z'
```

Requisitos: `cc`, `make` e uma libc com suporte a link estático. Os testes usam
também `bash`, `valgrind` e `script`.

Para abrir o minishell direto no lugar de outro shell, chame
`exec ~/.local/bin/minishell` no `config.fish` ou no `.bashrc`, só em sessões
interativas.

## 🧠 Linguagem

| Recurso | Exemplos |
|---|---|
| Pipes e listas | `a \| b`, `a ; b`, `a && b`, `a \|\| b`, `a &`, `( a ; b )` |
| Redirecionamentos | `< > >> <<`, `2>`, `2>>`, `>&2`, `2>&1` |
| Aspas e escape | `'literal'`, `"com $VAR"`, `\$`, `\ ` |
| Variáveis | `$VAR`, `${VAR}`, `$?`, `~`, `A=1`, `A=1 cmd`, `export` |
| Substituição | `$(cmd)`, `` `cmd` ``, `$((1+2*3))` |
| Curingas | `*`, `?`, `[a-z]`, `[!x]`, `[[:alpha:]]`, `src/*.c` |
| Controle de fluxo | `if/elif/else`, `while`, `until`, `for`, `case`, `{ ...; }`, `!` |
| Funções | `nome() { ...; }`, `local`, `return`, `$1`..`$9`, `$#`, `$@`, `$*` |
| Expansão de parâmetros | `${x:-d}`, `${x:=v}`, `${x:+a}`, `${x:?m}`, `${#x}`, `${x#p}`, `${x##p}`, `${x%s}`, `${x%%s}` |
| Várias linhas | o Enter continua se falta fechar aspas, parênteses, ou há `\`, `\|`, `&&`, `\|\|` no fim |
| Comentários | `# até o fim da linha` |

**Builtins:** `echo` (`-n`), `cd`, `pwd`, `export`, `unset` (`-f`, `-v`), `readonly`,
`env`, `exit`, `alias`, `unalias`, `source` e `.`, `eval`, `exec`, `command`, `type`,
`which`, `test` e `[`, `read`, `shift`, `set --`, `umask`, `times`, `true`, `false`,
`:`, `local`, `return`, `break`, `continue`, `trap`, `wait`, `kill`, `getopts`, `jobs`, `fg`, `bg`.

## ⌨️ Editor de linha

- **Edição:** setas, `Home`/`End`, `Ctrl+A/E/B/F/K/U/W/L`, `Alt+B/F`, `Delete`, UTF-8.
- **Histórico:** salvo em `~/.minishell_history`; `↑`/`↓` filtram pelo que já foi
  digitado; `Ctrl+R` busca de forma incremental (`Ctrl+S` busca para a frente).
- **Sugestão:** texto cinza a partir do histórico; `→` ou `End` aceitam.
- **Destaque:** comando válido em verde, inválido em vermelho; operadores, aspas
  e variáveis têm cores próprias.
- **Tab:** completa comandos, aliases, arquivos, diretórios e `$VAR`.
- **Colar:** o terminal avisa quando é uma colagem, então o `Tab` não dispara no meio.

## 🎛️ Configuração

O `~/.minishellrc` é lido a cada shell interativo, uma linha por comando:

```sh
alias ll="ls -l"
export EDITOR=vim
export PS1='\e[32m\u\e[0m@\e[97m\h\e[0m \w> '
```

O prompt padrão é `usuario@host dir>`. Em `PS1` valem `\u` usuário, `\h` host,
`\w` diretório, `\W` só o nome do diretório, `\$`, `\?` (status do último
comando), `\n`, `\e` (cores) e `\\`.

## 🗂️ Estrutura

```
src/
  minishell.c   laço principal
  parser/       tokenizer, parser, expansão, curingas, alias, aritmética
  executor/     execução da árvore, pipes, jobs, heredoc, substituição
  builtins/     comandos internos
  lineedit/     editor: teclas, tela, histórico, Tab, Ctrl+R, destaque
  env/          variáveis (exportadas e só do shell)
  prompt/       montagem do PS1
mylib/          biblioteca própria: strings, memória, leitor de linhas (ml_*)
tests/          corpus, fuzz e scripts
```

## 🧪 Testes

```sh
make compare    # compara saída e status com o bash (tests/cases.txt)
make stress     # fuzz, entradas gigantes e teclas aleatórias, com ASan e UBSan
make leaks      # vazamentos de memória com Valgrind

tests/posix.sh   # testador interno de conformidade POSIX (usa casos .tst locais, fora do Git)
```

O `make stress` gera linhas por gramática, por mutação do corpus e por ruído
binário; roda linhas de 1 MB, 100 pipes, 200 jobs e 10 mil arquivos num glob;
confere vazamento de descritores e processos zumbis; e envia teclas aleatórias ao
editor dentro de um terminal simulado.

## 🛣️ Roteiro

Próximos passos, em ordem de impacto para o uso diário: `ulimit`, `alias` completo e
mensagens de erro no formato POSIX; `{a,b}`, `[[ ... ]]` e arrays; `history` e `echo -e`; `Tab` com
opções de comandos e subcomandos.
