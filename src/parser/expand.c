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

static int	is_special(char c)
{
	return (c && ft_strchr("?#@*$!", c));
}

static int	name_len(const char *s, int braced)
{
	int	i;

	if (is_special(s[0]))
		return (1);
	i = 0;
	if (ft_isdigit(s[0]))
	{
		if (!braced)
			return (1);
		while (ft_isdigit(s[i]))
			i++;
		return (i);
	}
	while (ft_isalnum(s[i]) || s[i] == '_')
		i++;
	return (i);
}

static int	brace_offset(const char *s)
{
	int	n;

	if (s[0] != '{')
		return (0);
	n = name_len(s + 1, 1);
	return (n > 0 && s[1 + n] == '}');
}

static int	is_ref(const char *s)
{
	return (brace_offset(s) || name_len(s, 0) > 0);
}

static int	ref_length(char *s)
{
	if (brace_offset(s))
		return (name_len(s + 1, 1) + 2);
	return (name_len(s, 0));
}

static char	*join_params(t_shell *shell)
{
	char	*out;
	char	*tmp;
	int		i;

	out = ft_strdup("");
	i = 0;
	while (i < shell->nparams)
	{
		tmp = ft_strjoin(out, i ? " " : "");
		free(out);
		out = ft_strjoin(tmp, shell->params[i++]);
		free(tmp);
	}
	return (out);
}

static char	*special_value(const char *name, t_shell *shell)
{
	int	n;

	if (name[0] == '?')
		return (ft_itoa(shell->exit_status));
	if (name[0] == '#')
		return (ft_itoa(shell->nparams));
	if (name[0] == '@' || name[0] == '*')
		return (join_params(shell));
	if (name[0] == '$')
		return (ft_itoa((int)getpid()));
	if (name[0] == '!')
		return (shell->last_bg ? ft_itoa((int)shell->last_bg) : ft_strdup(""));
	if (!ft_isdigit(name[0]))
		return (NULL);
	n = ft_atoi(name);
	if (n == 0)
		return (ft_strdup(shell->argv0));
	if (n <= shell->nparams)
		return (ft_strdup(shell->params[n - 1]));
	return (ft_strdup(""));
}

static char	*get_var_value(char *var_name, t_shell *shell)
{
	int		len;
	char	*name;
	char	*value;

	len = brace_offset(var_name);
	var_name += len;
	len = name_len(var_name, len);
	if (len == 0)
		return (ft_strdup("$"));
	name = ft_substr(var_name, 0, len);
	if (!name)
		return (NULL);
	value = special_value(name, shell);
	if (!value && env_get(shell->env, name))
		value = ft_strdup(env_get(shell->env, name));
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
		if (token[i] == '$' && is_ref(&token[i + 1]))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				size += ft_strlen(var_value);
				free(var_value);
			}
			i += ref_length(&token[i + 1]) + 1;
		}
		else
		{
			size++;
			i++;
		}
	}
	return (size + 1);
}

typedef struct s_buf
{
	char	*str;
	size_t	len;
	size_t	cap;
}	t_buf;

static int	buf_put(t_buf *buf, const char *src, size_t n)
{
	char	*bigger;
	size_t	cap;

	if (buf->len + n + 1 > buf->cap)
	{
		cap = buf->cap ? buf->cap : 64;
		while (buf->len + n + 1 > cap)
			cap *= 2;
		bigger = malloc(cap);
		if (!bigger)
			return (0);
		ft_memcpy(bigger, buf->str, buf->len);
		free(buf->str);
		buf->str = bigger;
		buf->cap = cap;
	}
	ft_memcpy(buf->str + buf->len, src, n);
	buf->len += n;
	buf->str[buf->len] = '\0';
	return (1);
}
static int	heredoc_escape(const char *s)
{
	return (s[0] == '\\' && s[1] && ft_strchr("$\\`", s[1]));
}


static int	heredoc_chunk(t_buf *buf, char *line, int *i, t_shell *shell)
{
	char	*value;
	int		start;

	start = *i;
	while (line[*i] && !(line[*i] == '$' && is_ref(&line[*i + 1]))
		&& !heredoc_escape(&line[*i]))
		(*i)++;
	if (*i > start && !buf_put(buf, line + start, *i - start))
		return (0);
	if (heredoc_escape(&line[*i]))
	{
		*i += 2;
		return (buf_put(buf, line + *i - 1, 1));
	}
	if (!line[*i])
		return (1);
	value = get_var_value(&line[*i + 1], shell);
	*i += ref_length(&line[*i + 1]) + 1;
	if (!value || !buf_put(buf, value, ft_strlen(value)))
		return (free(value), 0);
	return (free(value), 1);
}

char	*expand_heredoc_line(char *line, t_shell *shell)
{
	t_buf	buf;
	int		i;

	buf.str = NULL;
	buf.len = 0;
	buf.cap = 0;
	i = 0;
	while (line[i])
	{
		if (!heredoc_chunk(&buf, line, &i, shell))
			return (free(buf.str), NULL);
	}
	if (!buf.str)
		return (ft_strdup(""));
	return (buf.str);
}


# define SPLIT 1
# define QUOTED 2
# define SEP 4

typedef struct s_word
{
	char	*str;
	char	*mask;
	int		len;
	size_t	cap;
	int		at_empty;
}	t_word;

static int	word_reserve(t_word *w, size_t extra)
{
	char	*str;
	char	*mask;
	size_t	cap;

	if (w->len + extra <= w->cap)
		return (1);
	cap = w->cap ? w->cap : 64;
	while (cap < w->len + extra)
		cap *= 2;
	str = ft_calloc(cap + 1, 1);
	mask = ft_calloc(cap + 1, 1);
	if (!str || !mask)
		return (free(str), free(mask), 0);
	ft_memcpy(str, w->str, w->len);
	ft_memcpy(mask, w->mask, w->len);
	free(w->str);
	free(w->mask);
	w->str = str;
	w->mask = mask;
	w->cap = cap;
	return (1);
}

static void	put_value(t_word *w, char *val, int flags)
{
	int	k;

	k = 0;
	if (val && word_reserve(w, ft_strlen(val)))
	{
		while (val[k])
		{
			w->str[w->len] = val[k++];
			w->mask[w->len++] = flags;
		}
	}
	free(val);
}

static void	put_char(t_word *w, char c, int flags)
{
	if (!word_reserve(w, 1))
		return ;
	w->str[w->len] = c;
	w->mask[w->len++] = flags;
}

static int	is_at_ref(const char *s)
{
	return (s[0] == '@' || (s[0] == '{' && s[1] == '@'));
}

static void	put_at_words(t_word *w, t_shell *shell)
{
	int	i;

	if (shell->nparams == 0)
		w->at_empty = 1;
	i = 0;
	while (i < shell->nparams)
	{
		if (i)
			put_char(w, ' ', SEP | QUOTED);
		put_value(w, ft_strdup(shell->params[i]), QUOTED);
		i++;
	}
}

static int	put_dollar(t_word *w, char *raw, int quote, t_shell *shell)
{
	if (quote == '"' && is_at_ref(raw + 1))
		put_at_words(w, shell);
	else
		put_value(w, get_var_value(raw + 1, shell), quote ? QUOTED : SPLIT);
	return (ref_length(raw + 1) + 1);
}

static char	*unescape_backtick(const char *src, int len)
{
	char	*out;
	int		i;
	int		j;

	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	j = 0;
	while (i < len)
	{
		if (src[i] == '\\' && i + 1 < len && ft_strchr("`\\$", src[i + 1]))
			i++;
		out[j++] = src[i++];
	}
	out[j] = '\0';
	return (out);
}

char	*expand_subst_at(const char *s, int *consumed, t_shell *shell)
{
	char	*text;
	char	*out;
	int		end;

	*consumed = 0;
	if (s[0] == '`')
		end = skip_backtick(s, 0);
	else if (s[0] == '$' && s[1] == '(')
		end = skip_dollar_paren(s, 0);
	else
		return (NULL);
	if (end == -1)
		return (NULL);
	*consumed = end;
	if (s[0] == '`')
		return (run_substitution(shell, unescape_backtick(s + 1, end - 2)));
	if (s[2] == '(' && end >= 6 && s[end - 2] == ')')
	{
		text = ft_substr(s, 3, end - 5);
		out = expand_variables(text, shell);
		free(text);
		text = out;
		out = arith_string(text ? text : "", shell);
		return (free(text), out);
	}
	return (run_substitution(shell, ft_substr(s, 2, end - 3)));
}

static int	put_subst(t_word *w, const char *s, int quote, t_shell *shell)
{
	char	*out;
	int		used;

	out = expand_subst_at(s, &used, shell);
	if (!out)
		return (0);
	put_value(w, out, quote ? QUOTED : SPLIT);
	return (used);
}

static int	put_escape(t_word *w, const char *raw, int quote)
{
	if (!raw[1])
		return (0);
	if (raw[1] == '\n')
		return (2);
	if (quote == '"' && !ft_strchr("$\"\\`", raw[1]))
		return (0);
	put_char(w, raw[1], QUOTED);
	return (2);
}

static int	put_special(t_word *w, char *raw, int quote, t_shell *shell)
{
	if (raw[0] == '\\' && quote != '\'')
		return (put_escape(w, raw, quote));
	if (quote != '\'' && (raw[0] == '`' || (raw[0] == '$' && raw[1] == '(')))
		return (put_subst(w, raw, quote, shell));
	return (0);
}

static void	fill_word(t_word *w, char *raw, t_shell *shell)
{
	int		i;
	int		n;
	int		quote;
	char	*home;

	i = 0;
	quote = 0;
	home = env_get(shell->env, "HOME");
	if (raw[0] == '~' && (!raw[1] || raw[1] == '/') && home)
	{
		put_value(w, ft_strdup(home), QUOTED);
		i = 1;
	}
	while (raw[i])
	{
		n = put_special(w, raw + i, quote, shell);
		if (n)
			i += n;
		else if ((raw[i] == '\'' || raw[i] == '"')
			&& (!quote || quote == raw[i]))
			quote = (quote == 0) * raw[i++];
		else if (raw[i] == '$' && quote != '\'' && is_ref(&raw[i + 1]))
			i += put_dollar(w, &raw[i], quote, shell);
		else
			put_char(w, raw[i++], quote ? QUOTED : 0);
	}
}

static void	emit_one(t_word *w, int start, int end, t_token **tok[2])
{
	char	*pattern;
	int		k;
	int		j;
	int		has_glob;

	pattern = malloc((end - start) * 2 + 1);
	j = 0;
	has_glob = 0;
	k = start;
	while (pattern && k < end)
	{
		if (w->mask[k] & QUOTED && ft_strchr("*?[\\", w->str[k]))
			pattern[j++] = '\\';
		else if (ft_strchr("*?[", w->str[k]))
			has_glob = 1;
		pattern[j++] = w->str[k++];
	}
	if (pattern)
		pattern[j] = '\0';
	if (!has_glob || !pattern || !expand_glob(pattern, tok[0], tok[1]))
		token_add_back(tok[0], tok[1], TOKEN_WORD,
			ft_substr(w->str, start, end - start));
	free(pattern);
}

static int	emit_fields(t_word *w, t_token **tok[2])
{
	int	start;
	int	k;
	int	count;

	start = -1;
	count = 0;
	k = 0;
	while (k <= w->len)
	{
		if (k == w->len || (w->mask[k] & SEP) || ((w->mask[k] & SPLIT) && ft_isspace(w->str[k])))
		{
			if (start >= 0 && ++count)
				emit_one(w, start, k, tok);
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
	t_token	**tok[2];
	char	*home;
	size_t	cap;

	home = env_get(shell->env, "HOME");
	cap = calculate_expanded_size(raw, shell) + (home ? ft_strlen(home) : 0);
	w.len = 0;
	w.cap = cap;
	w.at_empty = 0;
	w.str = ft_calloc(cap + 1, 1);
	w.mask = ft_calloc(cap + 1, 1);
	tok[0] = head;
	tok[1] = cur;
	if (w.str && w.mask)
	{
		fill_word(&w, raw, shell);
		if (!emit_fields(&w, tok)
			&& !w.at_empty && (ft_strchr(raw, '\'') || ft_strchr(raw, '"')))
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

char	*expand_variables(char *token, t_shell *shell)
{
	t_word	w;
	char	*home;
	char	*result;
	size_t	cap;

	home = env_get(shell->env, "HOME");
	cap = calculate_expanded_size(token, shell) + (home ? ft_strlen(home) : 0);
	w.len = 0;
	w.cap = cap;
	w.at_empty = 0;
	w.str = ft_calloc(cap + 1, 1);
	w.mask = ft_calloc(cap + 1, 1);
	result = NULL;
	if (w.str && w.mask)
	{
		fill_word(&w, token, shell);
		result = ft_strdup(w.str);
	}
	free(w.str);
	free(w.mask);
	return (result);
}
