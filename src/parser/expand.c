/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 19:02:25 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 14:34:56 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/minishell.h"
#include "../../includes/env.h"

static int	var_name_length(char *str)
{
	int	i = 0;

	if (str[0] == '?')
		return (1);
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	return (i);
}

static char	*get_var_value(char *var_name, t_shell *shell)
{
	int		len;
	char	*name;
	char	*value;

	len = var_name_length(var_name);
	if (len == 0)
		return (ft_strdup("$"));
	name = ft_substr(var_name, 0, len);
	if (!name)
		return (NULL);
	if (ft_strcmp(name, "?") == 0)
		value = ft_itoa(shell->exit_status);
	else
	{
		value = env_get(shell->env, name);
		if (value)
			value = ft_strdup(value);
	}
	free(name);
	return (value ? value : ft_strdup(""));
}

static size_t	calculate_expanded_size(char *token, t_shell *shell)
{
	size_t	size;
	int		i;
	char	*var_value;

	size = 0;
	i = 0;
	while (token[i])
	{
		if (token[i] == '$' && (ft_isalnum(token[i+1]) || token[i+1] == '?' || token[i+1] == '_'))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				size += ft_strlen(var_value);
				free(var_value);
			}
			i += var_name_length(&token[i+1]) + 1;
		}
		else
		{
			size++;
			i++;
		}
	}
	return (size + 1);
}

static char	*process_token_with_quotes(char *token, t_shell *shell)
{
	char	*result;
	size_t	size;
	int		i;
	int		j;
	int		in_squote;
	int		in_dquote;
	char	*var_value;

	if (!token)
		return (NULL);
	size = calculate_expanded_size(token, shell);
	result = malloc(size);
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	in_squote = 0;
	in_dquote = 0;
	while (token[i] && j < (int)size - 1)
	{
		if (token[i] == '\'' && !in_dquote)
		{
			in_squote = !in_squote;
			i++;
		}
		else if (token[i] == '"' && !in_squote)
		{
			in_dquote = !in_dquote;
			i++;
		}
		else if (token[i] == '$' && !in_squote && (ft_isalnum(token[i+1]) || token[i+1] == '?' || token[i+1] == '_'))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				int k = 0;
				while (var_value[k] && j < (int)size - 1)
					result[j++] = var_value[k++];
				free(var_value);
				i += var_name_length(&token[i+1]) + 1;
			}
			else
				i += var_name_length(&token[i+1]) + 1;
		}
		else
			result[j++] = token[i++];
	}
	result[j] = '\0';
	return (result);
}

char	*expand_variables(char *token, t_shell *shell)
{
	char	*result;
	char	*home;
	char	*joined;

	result = process_token_with_quotes(token, shell);
	if (!result || token[0] != '~' || (token[1] && token[1] != '/'))
		return (result);
	home = env_get(shell->env, "HOME");
	if (!home)
		return (result);
	joined = ft_strjoin(home, result + 1);
	if (!joined)
		return (result);
	free(result);
	return (joined);
}

char	*remove_quotes(char *token)
{
	return (ft_strdup(token));
}

void	process_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	char	*processed;

	current = tokens;
	while (current)
	{
		if (current->type == TOKEN_WORD && current->value)
		{
			processed = process_token_with_quotes(current->value, shell);
			if (processed)
			{
				free(current->value);
				current->value = processed;
			}
		}
		current = current->next;
	}
}

static char	*append_str(char *dst, char *src)
{
	char	*joined;

	joined = ft_strjoin(dst, src);
	free(dst);
	free(src);
	return (joined);
}

char	*expand_heredoc_line(char *line, t_shell *shell)
{
	char	*result;
	int		i;

	result = ft_strdup("");
	i = 0;
	while (result && line[i])
	{
		if (line[i] == '$' && (ft_isalnum(line[i + 1]) || line[i + 1] == '?'
				|| line[i + 1] == '_'))
		{
			result = append_str(result, get_var_value(&line[i + 1], shell));
			i += var_name_length(&line[i + 1]) + 1;
		}
		else
			result = append_str(result, ft_substr(line, i++, 1));
	}
	return (result);
}

typedef struct s_word
{
	char	*str;
	char	*mask;
	int		len;
}	t_word;

static void	put_value(t_word *w, char *val, int splittable)
{
	int	k;

	k = 0;
	while (val && val[k])
	{
		w->str[w->len] = val[k++];
		w->mask[w->len++] = splittable;
	}
	free(val);
}

static int	put_dollar(t_word *w, char *raw, int quote, t_shell *shell)
{
	put_value(w, get_var_value(raw + 1, shell), quote == 0);
	return (var_name_length(raw + 1) + 1);
}

static void	fill_word(t_word *w, char *raw, t_shell *shell)
{
	int		i;
	int		quote;
	char	*home;

	i = 0;
	quote = 0;
	home = env_get(shell->env, "HOME");
	if (raw[0] == '~' && (!raw[1] || raw[1] == '/') && home)
	{
		put_value(w, ft_strdup(home), 0);
		i = 1;
	}
	while (raw[i])
	{
		if ((raw[i] == '\'' || raw[i] == '"') && (!quote || quote == raw[i]))
			quote = (quote == 0) * raw[i];
		else if (raw[i] == '$' && quote != '\'' && (ft_isalnum(raw[i + 1])
				|| raw[i + 1] == '?' || raw[i + 1] == '_'))
		{
			i += put_dollar(w, &raw[i], quote, shell);
			continue ;
		}
		else
			put_value(w, ft_substr(raw, i, 1), 0);
		i++;
	}
}

static int	emit_fields(t_word *w, t_token **head, t_token **cur)
{
	int	start;
	int	k;
	int	count;

	start = -1;
	count = 0;
	k = 0;
	while (k <= w->len)
	{
		if (k == w->len || (w->mask[k] && ft_isspace(w->str[k])))
		{
			if (start >= 0)
			{
				token_add_back(head, cur, TOKEN_WORD,
					ft_substr(w->str, start, k - start));
				count++;
			}
			start = -1;
		}
		else if (start < 0)
			start = k;
		k++;
	}
	return (count);
}

static void	expand_word(char *raw, t_shell *shell, t_token **head,
				t_token **cur)
{
	t_word	w;
	char	*home;
	size_t	cap;

	home = env_get(shell->env, "HOME");
	cap = calculate_expanded_size(raw, shell) + (home ? ft_strlen(home) : 0);
	w.len = 0;
	w.str = malloc(cap + 1);
	w.mask = ft_calloc(cap + 1, 1);
	if (w.str && w.mask)
	{
		fill_word(&w, raw, shell);
		if (!emit_fields(&w, head, cur)
			&& (ft_strchr(raw, '\'') || ft_strchr(raw, '"')))
			token_add_back(head, cur, TOKEN_WORD, ft_strdup(""));
	}
	free(w.str);
	free(w.mask);
}

void	expand_tokens(t_token **tokens, t_shell *shell)
{
	t_token	*head;
	t_token	*cur;
	t_token	*old;
	t_token	*next;

	head = NULL;
	cur = NULL;
	old = *tokens;
	while (old)
	{
		next = old->next;
		if (old->type == TOKEN_WORD && old->value)
			expand_word(old->value, shell, &head, &cur);
		free(old->value);
		free(old);
		old = next;
	}
	*tokens = head;
}
