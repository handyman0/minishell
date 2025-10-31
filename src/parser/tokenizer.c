/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:35:05 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/31 19:30:30 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.h"
#include "../includes/utils.h"

static t_token	*create_token(t_toktype type, char *value)
{
	t_token *token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->value = value;
	token->next = NULL;
	return (token);
}

static void	add_token(t_token **head, t_token **current, t_toktype type, char *value)
{
	t_token	*new_token;

	new_token = create_token(type, value);
	if (!new_token)
		return ;

	if (*head == NULL)
		*head = new_token;
	else
		(*current)->next = new_token;
	*current = new_token;
}

static int	handle_operator(char *line, int i, t_token **head, t_token **current)
{
	if (line[i] == '|' && line[i + 1] == '|')
	{
		add_token(head, current, TOKEN_OR, NULL);
		return (2);
	}
	else if (line[i] == '|')
	{
		add_token(head, current, TOKEN_PIPE, NULL);
		return (1);
	}
	else if (line[i] == '&' && line[i + 1] == '&')
	{
		add_token(head, current, TOKEN_AND, NULL);
		return (2);
	}
	else if (line[i] == '<' && line[i + 1] == '<')
	{
		add_token(head, current, TOKEN_HEREDOC, NULL);
		return (2);
	}
	else if (line[i] == '<')
	{
		add_token(head, current, TOKEN_REDIR_IN, NULL);
		return (1);
	}
	else if (line[i] == '>' && line[i + 1] == '>')
	{
		add_token(head, current, TOKEN_REDIR_APPEND, NULL);
		return (2);
	}
	else if (line[i] == '>')
	{
		add_token(head, current, TOKEN_REDIR_OUT, NULL);
		return (1);
	}
	else if (line[i] == '(')
	{
		add_token(head, current, TOKEN_LPAREN, NULL);
		return (1);
	}
	else if (line[i] == ')')
	{
		add_token(head, current, TOKEN_RPAREN, NULL);
		return (1);
	}
	return (0);
}

static char	*extract_word(char *line, int *i)
{
	int		len;
	char	*word;

	len = 0;
	while (line[*i + len] && !ft_isspace(line[*i + len]) &&
			!handle_operator(line, *i + len, NULL, NULL))
		len++;
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
			add_token(&head, &current, TOKEN_WORD, word);
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
			free(tokens->value);
		free(tokens);
		tokens = tmp;
	}
}
