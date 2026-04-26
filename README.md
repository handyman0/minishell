# minishell

<div align="center">

![C](https://img.shields.io/badge/Language-C-blue.svg)
![License](https://img.shields.io/badge/License-42%20School-lightgrey)
![Status](https://img.shields.io/badge/Status-Complete-brightgreen)

**Uma implementação minimalista de um shell Unix em C**

[Características](#características) • [Instalação](#instalação) • [Uso](#uso) • [Estrutura](#estrutura-do-projeto) • [Regras](#regras) • [Recursos](#recursos)

</div>

---

## 📋 Índice

- [Visão Geral](#visão-geral)
- [Características](#características)
- [Requisitos](#requisitos)
- [Instalação](#instalação)
- [Uso](#uso)
- [Exemplos de Uso](#exemplos-de-uso)
- [Estrutura do Projeto](#estrutura-do-projeto)
- [Arquitetura](#arquitetura)
- [Regras](#regras)
- [Funções Proibidas](#funções-proibidas)
- [Melhorias e Extensões](#melhorias-e-extensões)
- [Troubleshooting](#troubleshooting)
- [Recursos](#recursos)
- [Autor](#autor)

---

## 🎯 Visão Geral

**minishell** é uma implementação educacional de um shell Unix minimalista, seguindo as normas do currículo 42. O projeto demonstra compreensão profunda de conceitos fundamentais do Unix como:

- **Processamento de linhas de comando**: Tokenização e parsing
- **Expansão de variáveis**: Substituição de variáveis de ambiente
- **Redirecionamentos**: I/O redirection (`<`, `>`, `>>`, `<<`)
- **Pipes**: Conexão entre processos (`|`)
- **Operadores lógicos**: Execução condicional (`&&`, `||`)
- **Gestão de processos**: Fork, execução e sincronização
- **Sinais**: Tratamento de SIGINT e SIGQUIT

O shell implementa um subconjunto essencial de funcionalidades de bash/sh, ideal para aprendizado e demonstração de competências em C de baixo nível.

---

## ✨ Características

### Análise e Parsing
- ✅ **Tokenização**: Quebra a linha em tokens reconhecendo operadores e palavras
- ✅ **Árvore Sintática Abstrata (AST)**: Construção de AST para representação hierárquica de comandos
- ✅ **Suporte a Parênteses**: Agrupamento de comandos com `(` e `)`
- ✅ **Expansão de Wildcards**: Padrão `*` em nomes de arquivo

### Expansão
- ✅ **Variáveis de Ambiente**: Expansão de `$VAR` e `$?` (status de saída)
- ✅ **Remoção de Aspas**: Processamento correto de `'` e `"`
- ✅ **Tratamento de Espaços**: Splits e preservação em aspas

### Redirecionamentos
- ✅ **Entrada**: `<` (redireção de arquivo para stdin)
- ✅ **Saída**: `>` (redireção de stdout para arquivo)
- ✅ **Append**: `>>` (append de stdout em arquivo)
- ✅ **Heredoc**: `<<` (entrada multi-linha até delimitador)

### Pipes e Operadores
- ✅ **Pipes**: `|` (conexão entre processos)
- ✅ **AND Lógico**: `&&` (executa direita se esquerda sucede)
- ✅ **OR Lógico**: `||` (executa direita se esquerda falha)
- ✅ **Combinações**: Suporte a múltiplos operadores e pipes

### Builtins (Comandos Internos)
- ✅ `echo` com flag `-n`
- ✅ `cd` com suporte a `~` e `-`
- ✅ `pwd` (print working directory)
- ✅ `export` (definir variáveis exportadas)
- ✅ `unset` (remover variáveis)
- ✅ `env` (listar ambiente)
- ✅ `exit` com código de saída

### Gestão de Sinais
- ✅ `SIGINT` (Ctrl+C): Interrompe comando sem sair da shell
- ✅ `SIGQUIT` (Ctrl+\): Interrompe com core dump (em crianças)
- ✅ Histórico com `readline`

---

## 📦 Requisitos

### Sistema Operacional
- Linux (testado em Linux 5.x+)
- macOS (compatível com Homebrew)

### Bibliotecas Obrigatórias
- `readline` (para entrada interativa com histórico)
- `libc` (padrão do sistema)

### Ferramentas de Desenvolvimento
- `gcc` ou `clang`
- `make`
- `ar` (para criar bibliotecas estáticas)

### Instalação de Dependências

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential libreadline-dev
```

**macOS (Homebrew):**
```bash
brew install readline
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc make readline-devel
```

---

## 🔧 Instalação

### Clone e Compilação

```bash
# Clonar o repositório
git clone <repo-url> minishell
cd minishell

# Compilar o projeto
make

# Executar
./minishell
```

### Makefile Targets

```bash
make              # Compilar minishell
make clean        # Remover arquivos objeto
make fclean       # Remover executável e objeto
make re           # Limpar e recompilar
make libft        # Compilar libft (feito automaticamente)
```

### Estrutura de Diretórios após Make

```
minishell/
├── minishell              # Executável compilado
├── obj/                   # Arquivos objeto (.o)
│   ├── builtins/
│   ├── env/
│   ├── executor/
│   ├── parser/
│   ├── signals/
│   └── utils/
└── libft.a               # Biblioteca estática
```

---

## 🚀 Uso

### Iniciar a Shell

```bash
./minishell
```

Você verá o prompt:
```
minishell>
```

### Comandos Básicos

```bash
# Comando simples
minishell> ls -la

# Com redirecionamento
minishell> echo "Hello World" > output.txt

# Com pipe
minishell> cat file.txt | grep "pattern" | sort

# Operadores lógicos
minishell> test -f file && echo "Existe" || echo "Não existe"

# Variáveis de ambiente
minishell> echo $HOME
minishell> echo $?

# Wildcards
minishell> ls *.c

# Heredoc
minishell> cat << EOF
> Linha 1
> Linha 2
> EOF
```

### Sair da Shell

```bash
minishell> exit
minishell> exit 42           # Com código de saída
minishell> Ctrl+D            # EOF
```

---

## 📝 Exemplos de Uso

### Exemplo 1: Pipeline com Múltiplos Comandos
```bash
minishell> echo "apple banana cherry" | tr ' ' '\n' | sort
apple
banana
cherry
```

### Exemplo 2: Redirecionamento de Arquivo
```bash
minishell> cat << EOF > script.sh
> #!/bin/bash
> echo "Hello"
> EOF
minishell> chmod +x script.sh
minishell> ./script.sh
Hello
```

### Exemplo 3: Usando Variáveis de Ambiente
```bash
minishell> export USER_NAME="Alice"
minishell> echo "Name: $USER_NAME"
Name: Alice
minishell> echo $?
0
```

### Exemplo 4: Combinação de Operadores
```bash
minishell> test -d /tmp && echo "Temp existe" || echo "Não encontrado"
Temp existe

minishell> ls /nonexistent && echo "OK" || echo "Diretório não encontrado"
ls: cannot access '/nonexistent': No such file or directory
Diretório não encontrado
```

### Exemplo 5: Wildcards
```bash
minishell> touch file1.txt file2.txt file3.txt
minishell> rm file*.txt
minishell> ls file*.txt
ls: cannot access 'file*.txt': No such file or directory
```

---

## 🏗️ Estrutura do Projeto

```
minishell/
│
├── src/                      # Código-fonte principal
│   ├── minishell.c          # Main e inicialização
│   ├── parser/              # Parser e tokenizer
│   │   ├── tokenizer.c      # Conversão linha → tokens
│   │   ├── parser.c         # Tokens → AST
│   │   ├── expand.c         # Expansão de variáveis
│   │   ├── expand_wildcards.c
│   │   └── syntax_check.c   # Verificação de sintaxe
│   ├── executor/            # Execução da AST
│   │   ├── executor.c       # Orquestrador de execução
│   │   ├── exec_tree.c      # Recursão na AST
│   │   ├── exec_path.c      # Busca de comando no PATH
│   │   ├── exec_heredoc.c   # Implementação heredoc
│   │   ├── exec_utils.c     # Utilitários
│   │   └── subshell.c       # Suporte a subshells
│   ├── env/                 # Gestão de ambiente
│   │   ├── env_init.c       # Inicialização
│   │   ├── env_getset.c     # Get/Set de variáveis
│   │   ├── env_utils.c      # Utilitários
│   │   └── env_minimal.c    # Ambiente mínimo
│   ├── builtins/            # Comandos builtin
│   │   ├── echo.c
│   │   ├── cd.c
│   │   ├── pwd.c
│   │   ├── export.c
│   │   ├── unset.c
│   │   ├── env.c
│   │   └── exit.c
│   ├── signals/             # Tratamento de sinais
│   │   ├── signals.c        # Handlers SIGINT/SIGQUIT
│   │   └── history.c        # Integração readline
│   └── utils/               # Funções auxiliares
│       ├── debug_print.c
│       ├── errors.c
│       ├── free_all.c
│       └── split_quotes.c
│
├── includes/                # Headers
│   ├── minishell.h         # Header principal
│   ├── parser.h            # Definições de parse
│   ├── executor.h          # Definições de execução
│   ├── env.h               # Definições de ambiente
│   ├── builtins.h          # Protótipos de builtins
│   ├── signals.h           # Handlers de sinais
│   └── utils.h             # Funções auxiliares
│
├── libft/                   # Biblioteca libft (42)
│   ├── Makefile
│   ├── include/
│   ├── src/
│   ├── get_next_line/
│   └── ft_printf/
│
├── pipex/                   # Projeto pipex (bonus/referência)
│   └── (códigos auxiliares)
│
├── minishell_tester/        # Tester do projeto
│   ├── tester             # Script de teste
│   ├── redirects/         # Testes de redirecionamento
│   ├── pipes/             # Testes de pipes
│   └── ...
│
├── Makefile               # Build configuration
├── README.md             # Este arquivo
└── walkthrough.md        # Documentação técnica

```

---

## 🔄 Arquitetura

### Fluxo de Execução

```
┌─────────────────────────────────────────────────────────┐
│                    readline("minishell> ")               │
└────────────────────┬────────────────────────────────────┘
                     │
                     ↓
         ┌───────────────────────┐
         │  tokenize_line(line)  │  ← Quebra em tokens
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │ expand_wildcards()    │  ← Expande padrões *
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │process_tokens_before  │  ← Expande variáveis
         │_parsing()             │    Remove tokens vazios
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │  parse_tokens()       │  ← Constrói AST
         │  (parse_expression)   │    Reconhece operadores
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │  execute_tree()       │  ← Percorre AST
         │  (switch por tipo)    │    Executa comandos
         └────────┬──────────────┘
                  │
              ┌───┴───┐
              │       │
              ↓       ↓
    ┌──────────────┐ ┌──────────────┐
    │  Builtin?    │ │  Externo?    │
    │  Executa     │ │  Fork+Execve │
    │  direto      │ │              │
    └──────┬───────┘ └──────┬───────┘
           │                │
           └────────┬───────┘
                    │
                    ↓
         ┌───────────────────────┐
         │ free_ast(ast)         │  ← Limpa memória
         │ free_tokens(tokens)   │
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │ shell->exit_status    │  ← Armazena status
         └────────┬──────────────┘
                  │
                  ↓
         ┌───────────────────────┐
         │ Loop novamente...     │
         └───────────────────────┘
```

### Estrutura de Dados Principal

#### Token
```c
typedef struct s_token
{
    t_toktype    type;     // TOKEN_WORD, TOKEN_PIPE, etc.
    char         *value;   // Valor do token
    struct s_token *next;  // Próximo token (lista encadeada)
} t_token;
```

#### AST Node
```c
typedef struct s_node
{
    t_nodetype type;  // NODE_CMD, NODE_PIPE, NODE_AND, NODE_OR
    union {
        struct {
            char **argv;       // Argumentos do comando
            t_redir *redirs;   // Redirecionamentos
        } cmd;
        struct {
            struct s_node *left;
            struct s_node *right;
        } op;  // Para operadores (pipe, &&, ||)
    } data;
} t_node;
```

#### Ambiente
```c
typedef struct s_env
{
    char *key;              // Nome da variável
    char *value;            // Valor
    struct s_env *next;     // Próximo nó
} t_env;
```

---

## 📐 Regras

### Norma 42
- ✅ Máximo 80 colunas por linha
- ✅ Máximo 25 linhas por função
- ✅ Sem variáveis globais
- ✅ Sem funções estáticas (exceto as permitidas)
- ✅ Sem C99 (compatível com C89)

### Funções Permitidas (com restricções)
- `malloc`, `free`
- `write`, `read`
- `open`, `close`, `opendir`, `readdir`, `closedir`
- `chdir`, `getcwd`
- `access`, `execve`, `fork`, `wait`, `waitpid`
- `pipe`, `dup`, `dup2`
- `readline`, `add_history` (para histórico)
- `signal`, `sigaction` (gestão de sinais)

### Funções Proibidas
- ❌ `system()`
- ❌ `popen()` / `pclose()`
- ❌ `strtok()` (use alternativas)
- ❌ Variáveis globais
- ❌ Funções C99+ (exceto as explicitamente permitidas)

### Comportamento Esperado
- ✅ Tratamento correto de Ctrl+C (não sai da shell)
- ✅ Tratamento correto de Ctrl+D (sai como EOF)
- ✅ Suporte a histórico com seta para cima/baixo
- ✅ Expande `$HOME`, `$USER`, etc.
- ✅ Status de saída retorna corretamente

---

## 🔧 Melhorias e Extensões

### Funcionalidades Implementadas
- ✅ Expansão de variáveis `$VAR` e `$?`
- ✅ Suporte completo a redirecionamentos
- ✅ Pipes múltiplos
- ✅ Operadores `&&` e `||`
- ✅ Parênteses para agrupamento
- ✅ Wildcards com `*`
- ✅ Builtins completos

### Possíveis Extensões (não-obrigatórias)
- [ ] Suporte a aliases (`alias cmd="command"`)
- [ ] Histórico persistente em arquivo
- [ ] Substituição de histórico `!!`
- [ ] Expansão de tilde `~`
- [ ] Variáveis locais (sem export)
- [ ] Funções shell
- [ ] Background jobs
- [ ] Ctrl+Z para suspender

---

## 🐛 Troubleshooting

### Problema: "readline.h: No such file"
**Solução:**
```bash
# Ubuntu/Debian
sudo apt-get install libreadline-dev

# macOS
brew install readline

# Compile com:
make clean && make
```

### Problema: Shell não responde após Ctrl+C
**Causa:** Handlers de sinal não configurados
**Solução:** Verificar `setup_signals()` em `signals.c`

### Problema: Variáveis de ambiente não expandem
**Causa:** `expand_variables()` não está sendo chamado
**Solução:** Verificar `process_tokens_before_parsing()` em `parser.c`

### Problema: Redirecionamento não funciona
**Causa:** `handle_redirection()` retorna erro
**Solução:** Verificar permissões de arquivo e espaço em disco

### Problema: Comando não encontrado
**Causa:** PATH vazio ou comando inválido
**Solução:** Usar caminho absoluto `/bin/ls` ou exportar `export PATH=/bin:/usr/bin`

### Problema: Compilação falha com avisos
**Solução:**
```bash
make clean
make
# Se persistir: gcc -Wall -Wextra -Werror src/*.c ...
```

---

## 📚 Recursos

### Documentação Oficial
- [POSIX Shell Command Language](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/contents.html)
- [Bash Manual](https://www.gnu.org/software/bash/manual/)
- [Linux man pages](https://man7.org/)

### Referências de Implementação
- Sintaxe de parsing: `parse_tokens()` em `src/parser/parser.c`
- Execução: `execute_tree()` em `src/executor/exec_tree.c`
- Builtins: `src/builtins/`

### Ferramentas Úteis
- `strace ./minishell` - Rastreia chamadas de sistema
- `valgrind ./minishell` - Detecta memory leaks
- `gcc -fsanitize=address` - Sanitizador de endereço

### Testes
```bash
# Rodar tester incluído
cd minishell_tester
./tester redirects
./tester pipes
./tester syntax

# Testes manuais
echo "ls -la" | ./minishell
echo "echo 'Hello' | cat" | ./minishell
```

---

## 📖 Documentação Técnica

Para documentação técnica detalhada, consulte:
- [walkthrough.md](walkthrough.md) - Explicação passo-a-passo
- [DOXYGEN_IMPROVEMENTS.md](DOXYGEN_IMPROVEMENTS.md) - Referência de funções

---

## 📄 Licença

Este projeto segue as regras e licenças da **École 42**.

```
Copyright © 2026 École 42
Made during 42 Cursus
```

---

## 👤 Autor

**lmelo-do** (Luiz Melo)
- Escola: 42 São Paulo
- Projeto: minishell (4º Círculo)
- Status: ✅ Completo

---

## 🙏 Agradecimentos

- **42 Community** pelos testes e feedback
- **Readline** developers
- **Linux Kernel** documentação
- Outros estudantes 42 pelas discussões

---

<div align="center">

**Made with ❤️ at 42 São Paulo**

[⬆ Voltar ao topo](#minishell)

</div>
