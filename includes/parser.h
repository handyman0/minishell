/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:30:10 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/31 19:31:03 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include <stdlib.h>
# include <stdio.h>

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
} t_toktype;

typedef struct s_token
{
	t_toktype		type; // tipo do token
	char			*value; //string do token (para WORD)
	struct s_token	*next; //proximo token
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
} t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
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
} t_nodetype;

typedef struct s_node
{
	t_nodetype	type;
	union {
		// para comandos simples
		struct {
			char **argv;
			t_redir *redirs;
		} cmd;
		// para operadores binarios
		struct {
			struct s_node *left;
			struct s_node *right;
		} op;
		// para subshell -  usamos op.left para o conteudo
	} data;
} t_node;

/* =============================== */
/* FUNÇOES DO PARSER - Declarações */
/* =============================== */

// tokenizer.c
t_token	*tokenize_line(char *line);
void	free_tokens(t_token *tokens);

// parser.c
t_node	*parse_tokens(t_token **tokens);
void	free_ast(t_node *node);

// syntax_check.c
int		check_syntax(t_token *tokens);

// expand.c
void	expand_tokens(t_token *tokens);

// debug_print.c (sera movido para utils depois)
void	print_tokens(t_token *tokens);
/* void	print_ast(t_node *node, int); */

#endif
