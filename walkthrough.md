# 🧠 Walkthrough – Parte do Luiz (Minishell com Bônus + Estrutura)

---

<!-- ## 🔹 Etapa 1 — Reestruturação e arquitetura base

### 1. Criar estrutura de pastas

```
minishell/
├── includes/
│   ├── minishell.h
│   ├── parser.h
│   ├── executor.h
│   ├── env.h
│   ├── signals.h
│   ├── utils.h
├── src/
│   ├── main.c
│   ├── parser/
│   ├── executor/
│   ├── env/
│   ├── signals/
│   ├── utils/
│   └── bonus/
└── Makefile
``` -->

<!-- ### 2. Ajustar o Makefile

* Flags: `-Wall -Wextra -Werror -g -fsanitize=address`
* Targets: `all`, `clean`, `fclean`, `re`, `bonus`
* Adicionar variável `SRCS` com todos os `.c` das tuas pastas
 -->
<!-- ### 3. Criar header central (`minishell.h`)

* Incluir todos os outros headers
* Definir struct global do shell:

  ```c
  typedef struct s_shell {
      t_env   *env;
      int     exit_status;
      int     running;
  } t_shell;
  ```
* Adicionar includes padrão (`unistd.h`, `stdlib.h`, `stdio.h`, `readline`, `signal.h`, etc.)

---
 -->
<!-- ## 🔹 Etapa 2 — Parsing Lexical + Sintático

### 4. Tokenização avançada (`parser/tokenizer.c`)

* Ler linha e gerar lista de tokens:

  * `|`, `>`, `>>`, `<`, `<<`, `&&`, `||`, `(`, `)`
  * Strings entre aspas simples e duplas
  * Expansão de variáveis básicas `$VAR` e `$?`
* Struct:

  ```c
  typedef enum e_toktype {
      WORD, PIPE, REDIR_IN, REDIR_OUT, REDIR_APPEND,
      AND, OR, LPAREN, RPAREN
  } t_toktype;

  typedef struct s_token {
      t_toktype type;
      char *value;
      struct s_token *next;
  } t_token;
  ```
 -->
### 5. Parser recursivo (`parser/parser.c`)

* Converter lista de tokens em árvore binária:

  ```c
  typedef struct s_node {
      t_toktype type;
      char **argv;
      struct s_node *left;
      struct s_node *right;
  } t_node;
  ```
* Regras:

  * `PIPE` → esquerda e direita
  * `AND` / `OR` → esquerda e direita
  * Parênteses → subárvore
  * `WORD` → comando

### 6. Implementar funções auxiliares

* `split_argv()` → cria array `char **` a partir dos tokens WORD
* `free_tokens()` / `free_tree()` / `print_tree_debug()`

---

## 🔹 Etapa 3 — Execução Recursiva e Pipes

### 7. Execução por árvore (`executor/exec_tree.c`)

Função principal:

```c
int exec_tree(t_node *node, t_shell *sh);
```

Casos:

* `WORD` → comando simples ou builtin
* `PIPE` → cria pipe, fork nos dois lados
* `AND` → executa direita só se esquerda teve sucesso
* `OR` → executa direita só se esquerda falhou
* `LPAREN` → executa subárvore (fork separado)

### 8. Gerenciar redirecionamentos (`executor/redir.c`)

* Aplicar `<`, `>`, `>>` com `open()` + `dup2()`
* Reverter FDs após execução do comando

### 9. Resolver path (`executor/resolve_path.c`)

* Verificar se `argv[0]` é builtin → executa direto
* Senão:

  * Buscar em `PATH`
  * Usar `execve()`
  * Retornar erro formatado (`minishell: cmd: command not found`)

---

## 🔹 Etapa 4 — Sinais, Histórico e Controle

### 10. Sinais (`signals/signals.c`)

* Desativar `readline` signals: `rl_catch_signals = 0`
* `SIGINT` → nova linha + prompt
* `SIGQUIT` → ignorar
* `SIGTERM` → sair gracioso
* `signal(SIGINT, sigint_handler);`
* `signal(SIGQUIT, SIG_IGN);`

### 11. Histórico (`signals/history.c`)

* Ativar `add_history(line)` se a linha não estiver vazia
* Manter histórico persistente opcional (salvar em `.minishell_history`)

---

## 🔹 Etapa 5 — Expansões e Polimento

### 12. Expansão de variáveis (`parser/expand.c`)

* Substituir `$VAR` por valor em `t_env`
* Substituir `$?` pelo último `exit_status`
* Manter `$'string'` intacto (sem expandir)

### 13. Wildcards (`bonus/wildcards.c`)

* Usar `opendir()` + `readdir()` para listar arquivos do diretório atual
* Comparar com padrão `*`, ex: `*.c`, `test*`, etc.
* Substituir token por múltiplos WORDs

### 14. Subshells (`bonus/subshell.c`)

* Detectar `(` e `)` no parser
* Executar subárvore dentro de `fork()` e `waitpid()`
* Retornar o `exit_status` da subexecução

### 15. Operadores lógicos (`bonus/and_or.c`)

* `cmd1 && cmd2` → executa cmd2 apenas se cmd1 == 0
* `cmd1 || cmd2` → executa cmd2 apenas se cmd1 != 0

---

## 🔹 Etapa 6 — Tratamento de Erros e Cleanup

### 16. Erros (`utils/errors.c`)

* Mensagens padronizadas:

  * `minishell: command not found`
  * `minishell: syntax error near unexpected token`
* Retornos corretos em `$?`
* Função: `int err(char *msg, int code);`

### 17. Liberação de memória (`utils/free_all.c`)

* `free_env()`, `free_tokens()`, `free_tree()`, `free_argv()`
* `cleanup_shell()` → limpa tudo antes de `exit()`

### 18. Exit elegante (`utils/exit.c`)

* `exit` builtin deve liberar memória e fechar FDs
* `Ctrl+D` → mesmo comportamento de `exit 0`

---

## 🔹 Etapa 7 — Testes e Debug

### 19. Comandos de teste

* `ls | grep .c | wc -l`
* `echo hello && echo world`
* `(ls | grep .c) > out.txt`
* `echo $USER`
* `cat < in.txt | grep test > out.txt`
* `echo *`
* `sleep 2 && echo done`
* `echo "HOME=$HOME"`

### 20. Debug rápido

* Implementar `print_tokens()` e `print_tree_debug()`
* Variável global `DEBUG=1` ativa prints de parsing e execução

---

## 🔹 Etapa 8 — Bônus finais (opcional)

### 21. `heredoc` (`<<`)

* Ler input até delimitador
* Usar pipe temporário para enviar ao comando

### 22. Scripts `.sh`

* Se argumento for arquivo `.sh`, abrir e executar linha a linha

### 23. Colorir prompt

* Ex: `minishell ➜ ` em verde se último exit == 0, vermelho se != 0

---

## ✅ Resultado Esperado

Teu minishell, ao final:

* Lê entrada com histórico e sinais limpos
* Faz parsing de operadores complexos
* Executa pipelines, subshells e operadores lógicos
* Trata variáveis, wildcards e redirecionamentos
* É robusto, limpo e modular

---
