/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:30:10 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:29:06 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include <stdlib.h>
# include "minishell.h"

/* ==================================== */
/* TOKEN TYPES - Para o lexer/tokenizer */
/* ==================================== */

typedef enum e_toktype
{
	TOKEN_WORD, // palavra normal: "ls", "-la", "file.txt"
	TOKEN_PIPE, // |
	TOKEN_REDIR_IN, // <
	TOKEN_REDIR_OUT, // >
	TOKEN_REDIR_APPEND, // >>
	TOKEN_HEREDOC, // <<
	TOKEN_AND, // &&
	TOKEN_OR, // ||
	TOKEN_LPAREN, // (
	TOKEN_RPAREN, // )
	TOKEN_SEMI, // ;
	TOKEN_BG, // &
	TOKEN_DSEMI, // ;;
} t_toktype;

/**
 * @struct s_token
 * @brief Representa um token extraído da linha de comando.
 */
typedef struct s_token
{
	t_toktype		type; /**< Tipo do token. */
	char			*value; /**< Texto do token (apenas TOKEN_WORD). */
	struct s_token	*next; /**< Próximo token na lista. */
} t_token;

/* ========================================= */
/* AST NODE TYPES -  para a arvore sintatica */
/* ========================================= */

typedef enum e_redir_type
{
	REDIR_IN, // <
	REDIR_OUT, // >
	REDIR_APPEND, // >>
	REDIR_HEREDOC, // <<
	REDIR_DUP, // >&N
} t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	int				fd;
	int				hd_fd;
	char			*file;
	struct s_redir	*next;
} t_redir;

typedef enum e_nodetype
{
	NODE_CMD, // para comandos simples
	NODE_PIPE, // |
	NODE_AND, // &&
	NODE_OR, // ||
	NODE_PAREN, // ( ... )
	NODE_SEQ, // ;
	NODE_BG, // &
	NODE_IF,
	NODE_WHILE,
	NODE_FOR,
	NODE_CASE,
	NODE_GROUP,
	NODE_FUNC,
} t_nodetype;

typedef struct s_case
{
	char			**patterns;
	struct s_node	*body;
	struct s_case	*next;
}	t_case;

typedef struct s_node
{
	t_nodetype	type;
	union {
		// para comandos simples
		struct {
			char **argv;
			char **assigns;
			t_redir *redirs;
		} cmd;
		// para operadores binarios
		struct {
			struct s_node *left;
			struct s_node *right;
		} op;
		// comandos compostos: if, while/until, for, case, { }, funcao
		struct {
			struct s_node *a;
			struct s_node *b;
			struct s_node *c;
			char *name;
			char **words;
			t_case *cases;
			t_redir *redirs;
			int flag;
		} ctl;
	} data;
} t_node;

/* =============================== */
/* FUNÇOES DO PARSER - Declarações */
/* =============================== */

// tokenizer.c
/**
 * @brief Converte a linha de comando em uma lista de tokens.
 * @param line Linha de entrada do usuário.
 * @return Ponteiro para o primeiro token ou NULL em caso de erro.
 */
t_token	*tokenize_line(char *line);
/**
 * @brief Libera a lista de tokens.
 * @param tokens Lista de tokens.
 */
void	free_tokens(t_token *tokens);

/**
 * @brief Adiciona um token ao final da lista.
 * @param head Ponteiro para o primeiro token.
 * @param current Ponteiro para o token atual.
 * @param type Tipo do token.
 * @param value Valor do token.
 */
void	token_add_back(t_token **head, t_token **current, t_toktype type, char *value);

// parser.c
/**
 * @brief Cria a árvore sintática abstrata a partir dos tokens.
 * @param tokens Lista de tokens.
 * @return Nó raiz da árvore AST.
 */
t_node	*parse_tokens(t_token **tokens);
/**
 * @brief Libera todos os nós da árvore sintática.
 * @param node Nó raiz da árvore.
 */
void	free_ast(t_node *node);

/**
 * @brief Parseia uma expressão com suporte a parênteses.
 * @param tokens Lista de tokens.
 * @return Nó AST da expressão.
 */
t_node	*parse_expression(t_token **tokens);
/**
 * @brief Parseia operadores lógicos AND/OR.
 * @param tokens Lista de tokens.
 * @return Nó AST correspondente.
 */
t_node	*parse_and_or(t_token **tokens);

/**
 * @brief Parseia uma sequência de comandos conectados por pipes.
 * @param tokens Lista de tokens.
 * @return Nó AST da pipeline.
 */
t_node	*parse_pipeline(t_token **tokens);

// expand.c
/**
 * @brief Expande variáveis de ambiente em um token.
 * @param token Token original.
 * @param shell Estado global do shell.
 * @return String expandida.
 */
char	*expand_variables(char *token, t_shell *shell);

char	*expand_heredoc_line(char *line, t_shell *shell);
void	expand_tokens(t_token **tokens, t_shell *shell);

// expand_wildcards.c
int		expand_glob(char *pattern, t_token **head, t_token **cur);

void	expand_aliases(t_token **tokens, t_shell *shell);

int		is_ctl_node(t_node *node);
void	free_ctl(t_node *node);
t_node	*parse_list(t_token **tokens, int (*stop)(t_token *));
t_node	*parse_compound(t_token **tokens);
int		parse_failed(void);
int		parse_hit_eof(void);
void	parse_reset(void);
void	parse_report(t_shell *shell);
t_node	*make_node(t_nodetype type, t_node *left, t_node *right);
const char	*token_text(t_token *tok);
void	*parse_fail_at(t_token *tok);

// syntax_check.c
/**
 * @brief Verifica a sintaxe dos tokens.
 * @param tokens Lista de tokens.
 * @return 0 se válido, outro valor em caso de erro.
 */
int		check_syntax(t_token *tokens);
int		check_quotes(char *line);
char	open_quote(const char *line);
int		skip_dollar_paren(const char *s, int i);
int		skip_backtick(const char *s, int i);
int		skip_quote(const char *s, int i);
char	*arith_string(const char *text, t_shell *shell);
char	*expand_subst_at(const char *s, int *consumed, t_shell *shell);
int		line_incomplete(char *line);


// prototipos temporarios
/**
 * @brief Retorna se o token é uma redireção.
 * @param token Token a verificar.
 * @return 1 se for redireção, 0 caso contrário.
 */
int		is_redirection_token(t_token *token);

/**
 * @brief Trata um token de redirecionamento e anexa à lista de redirecionamentos.
 * @param tokens Ponteiro para o token atual.
 * @param redirs Lista de redirecionamentos do comando.
 * @return 1 em sucesso, 0 em erro.
 */
int		handle_redirection(t_token **tokens, t_redir **redirs);

/**
 * @brief Libera a lista de redirecionamentos.
 * @param redirs Lista de redirecionamentos.
 */
void	free_redirs(t_redir *redirs);

#endif
