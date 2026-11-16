/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:35:05 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 19:06:39 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.h"
#include "../includes/utils.h"

void	token_add_back(t_token **head, t_token **current,
					t_toktype type, char *value)
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

static int	handle_operator(char *line, int i,
					t_token **head, t_token **current)
{
	if (line[i] == '|' && line[i + 1] == '|')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_OR, NULL);
		return (2);
	}
	else if (line[i] == '|')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_PIPE, NULL);
		return (1);
	}
	else if (line[i] == '&' && line[i + 1] == '&')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_AND, NULL);
		return (2);
	}
	else if (line[i] == '&' && (i == 0 || line[i - 1] != '>'))
	{
		if (head && current)
			token_add_back(head, current, TOKEN_BG, NULL);
		return (1);
	}
	else if (line[i] == '<' && line[i + 1] == '<')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_HEREDOC, NULL);
		return (2);
	}
	else if (line[i] == '<')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_REDIR_IN, NULL);
		return (1);
	}
	else if (line[i] == '>' && line[i + 1] == '|')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_REDIR_CLOBBER, NULL);
		return (2);
	}
	else if (line[i] == '>' && line[i + 1] == '>')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_REDIR_APPEND, NULL);
		return (2);
	}
	else if (line[i] == '>')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_REDIR_OUT, NULL);
		return (1);
	}
	else if (line[i] == '(')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_LPAREN, NULL);
		return (1);
	}
	else if (line[i] == ')')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_RPAREN, NULL);
		return (1);
	}
	else if (line[i] == ';' && line[i + 1] == ';')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_DSEMI, NULL);
		return (2);
	}
	else if (line[i] == ';')
	{
		if (head && current)
			token_add_back(head, current, TOKEN_SEMI, NULL);
		return (1);
	}
	return (0);
}

static int	skip_construct(char *line, int pos)
{
	int	next;

	if (line[pos] == '\'' || line[pos] == '"')
		next = skip_quote(line, pos);
	else if (line[pos] == '`')
		next = skip_backtick(line, pos);
	else
		next = skip_dollar_paren(line, pos);
	if (next == -1)
		return ((int)ft_strlen(line));
	return (next);
}

static char	*extract_word(char *line, int *i, int len)
{
	char	*word;
	int		p;

	p = *i + len;
	while (line[p] && !ft_isspace(line[p]) && !handle_operator(line, p, NULL,
			NULL))
	{
		if (line[p] == '\\' && line[p + 1])
			p += 2;
		else if (line[p] == '\'' || line[p] == '"' || line[p] == '`'
			|| (line[p] == '$' && line[p + 1] == '('))
			p = skip_construct(line, p);
		else
			p++;
	}
	len = p - *i;
	if (len == 0)
		return (NULL);
	word = malloc(len + 1);
	if (!word)
		return (NULL);
	ft_memcpy(word, &line[*i], len);
	word[len] = '\0';
	*i += len;
	return (word);
}

static char	*copy_range(const char *src, size_t n)
{
	char	*out;

	out = malloc(n + 1);
	if (!out)
		return (NULL);
	ft_memcpy(out, src, n);
	out[n] = '\0';
	return (out);
}

static int	fd_redirection(char *line, int *i, t_token **head, t_token **cur)
{
	int	digits;
	int	op_len;

	digits = 0;
	while (ft_isdigit(line[*i + digits]))
		digits++;
	if (digits == 0 || (line[*i + digits] != '>' && line[*i + digits] != '<'))
		return (0);
	op_len = handle_operator(line, *i + digits, head, cur);
	if (op_len > 0 && *cur)
		(*cur)->value = copy_range(line + *i, digits);
	*i += digits + op_len;
	return (op_len);
}

static int	continues_command(t_token *tok)
{
	return (tok->type == TOKEN_PIPE || tok->type == TOKEN_AND
		|| tok->type == TOKEN_OR || tok->type == TOKEN_SEMI
		|| tok->type == TOKEN_BG || tok->type == TOKEN_LPAREN
		|| tok->type == TOKEN_DSEMI);
}

t_token	*tokenize_line(char *line)
{
	t_token	*head;
	t_token	*current;
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
		if (line[i] == '\n')
		{
			if (current && !continues_command(current))
				token_add_back(&head, &current, TOKEN_SEMI, NULL);
			i++;
			continue ;
		}
		if (ft_isspace(line[i]))
		{
			i++;
			continue ;
		}
		if (line[i] == '#')
		{
			while (line[i] && line[i] != '\n')
				i++;
			continue ;
		}
		op_len = fd_redirection(line, &i, &head, &current);
		if (op_len > 0)
			continue ;
		op_len = handle_operator(line, i, &head, &current);
		if (op_len > 0)
		{
			i += op_len;
			continue ;
		}
		word = extract_word(line, &i, 0);
		if (word)
			token_add_back(&head, &current, TOKEN_WORD, word);
		else
			i++;
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
