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
