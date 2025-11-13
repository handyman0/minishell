/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:35:05 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 16:36:39 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.h"
#include "../includes/utils.h"

void	token_add_back(t_token **head, t_token **current, t_toktype type, char *value)
{
	t_token	*new_token;

	new_token = malloc(sizeof(t_token));
	if (!new_token)
		return ;
	new_token->type = type;
	new_token->value = value;
	new_token->next = NULL;

	if (*head == NULL)
		*head = new_token;
	else if (*current)
		(*current)->next = new_token;
	*current = new_token;
}

static int	handle_operator(char *line, int i, t_token **head, t_token **current)
{
	if (line[i] == '|' && line[i + 1] == '|')
	{
		token_add_back(head, current, TOKEN_OR, NULL);
		return (2);
	}
	else if (line[i] == '|')
	{
		token_add_back(head, current, TOKEN_PIPE, NULL);
		return (1);
	}
	else if (line[i] == '&' && line[i + 1] == '&')
	{
		token_add_back(head, current, TOKEN_AND, NULL);
		return (2);
	}
	else if (line[i] == '<' && line[i + 1] == '<')
	{
		token_add_back(head, current, TOKEN_HEREDOC, NULL);
		return (2);
	}
	else if (line[i] == '<')
	{
		token_add_back(head, current, TOKEN_REDIR_IN, NULL);
		return (1);
	}
	else if (line[i] == '>' && line[i + 1] == '>')
	{
		token_add_back(head, current, TOKEN_REDIR_APPEND, NULL);
		return (2);
	}
	else if (line[i] == '>')
	{
		token_add_back(head, current, TOKEN_REDIR_OUT, NULL);
		return (1);
	}
	else if (line[i] == '(')
	{
		token_add_back(head, current, TOKEN_LPAREN, NULL);
		return (1);
	}
	else if (line[i] == ')')
	{
		token_add_back(head, current, TOKEN_RPAREN, NULL);
		return (1);
	}
	return (0);
}

static char	*extract_word(char *line, int *i)
{
	int		len;
	char	*word;
	char	quote;

	len = 0;
	while (line[*i + len] && !ft_isspace(line[*i + len]) &&
			!handle_operator(line, *i + len, NULL, NULL))
	{
		if (line[*i + len] == '\'' || line[*i + len] == '\"')
		{
			quote = line[*i + len];
			len++;

			while (line[*i + len] && line[*i + len] != quote)
				len++;
			if (line[*i + len] == quote)
				len++;
			else
			{
				break ;
			}
		}
		else
		{
			len++;
		}
	}
	if (len == 0)
		return (NULL);
	word = malloc(len + 1);
	if (!word)
		return (NULL);
	ft_strlcpy(word, &line[*i], len + 1);
	*i += len;
	return (word);
}

t_token	*tokenize_line(char *line)
{
	t_token *head;
	t_token *current;
	int		i;
	char	*word;
	int		op_len;

	if (!line)
		return (NULL);
	head = NULL;
	current = NULL;
	i = 0;
	while (line[i])
	{
		if (ft_isspace(line[i]))
		{
			i++;
			continue ;
		}
		op_len = handle_operator(line, i, &head, &current);
		if (op_len > 0)
		{
			i += op_len;
			continue ;
		}
		word = extract_word(line, &i);
		if (word)
			token_add_back(&head, &current, TOKEN_WORD, word);
	}
	return (head);
}

void	free_tokens(t_token *tokens)
{
	t_token	*tmp;

	while (tokens)
	{
		tmp = tokens->next;
		if (tokens->value)
		{
			free(tokens->value);
			tokens->value = NULL;
		}
		free(tokens);
		tokens = tmp;
	}
}
