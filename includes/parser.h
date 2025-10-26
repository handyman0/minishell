/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:30:10 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:08:14 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include <stdlib.h>
# include <stdio.h>

typedef enum e_toktype
{
	WORD, PIPE, REDIR_IN, REDIR_OUT, REDIR_APPEND,
	AND, OR, LPAREN, RPAREN
}	t_toktype;

typedef struct s_token
{
	t_toktype		type;
	char			*value;
	struct s_token	*next;
}	t_token;

typedef struct s_node
{
	t_toktype		type;
	char			**argv;
	struct s_node	*left;
	struct s_node	*right;
}	t_node;

#endif
