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
#include "../../includes/ctl.h"
#include "../../includes/output.h"

static int	is_special(char c)
{
	return (c && ft_strchr("?#@*$!-", c));
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

static int	brace_end(const char *s)
{
	int	i;
	int	depth;
	int	next;

	i = 1;
	depth = 1;
	while (s[i])
	{
		next = i + 1;
		if (s[i] == '\\' && s[i + 1])
			next = i + 2;
		else if (s[i] == '\'' || s[i] == '"')
			next = skip_quote(s, i);
		else if (s[i] == '`')
			next = skip_backtick(s, i);
		else if (s[i] == '$' && s[i + 1] == '(')
			next = skip_dollar_paren(s, i);
		else if (s[i] == '{')
			depth++;
		else if (s[i] == '}' && --depth == 0)
			return (i);
		if (next == -1)
			return (-1);
		i = next;
	}
	return (-1);
}

static int	hash_name(const char *inner)
{
	int	c;

	c = inner[1] == ':';
	if (inner[1] && ft_strchr("-=+?", inner[1 + c]) && (c || (inner[2]
					&& inner[2] != '}')))
		return (1);
	return (0);
}

static int	is_ref(const char *s)
{
	int	n;

	if (s[0] != '{')
		return (name_len(s, 0) > 0);
	if (s[1] == '#' && hash_name(s + 1))
		n = 1;
	else if (s[1] == '#' && s[2] && s[2] != '}')
		n = name_len(s + 2, 1);
	else
		n = name_len(s + 1, 1);
	return (n > 0 && brace_end(s) != -1);
}

static int	ref_length(char *s)
{
	if (s[0] == '{')
		return (brace_end(s) + 1);
	return (name_len(s, 0));
}

static const char	*ifs_value(t_shell *shell)
{
	char	*ifs;

	ifs = env_get(shell->env, "IFS");
	return (ifs ? ifs : " \t\n");
}

static char	*join_params(t_shell *shell, int star)
{
	char	sep[2];
	char	*out;
	char	*tmp;
	int		i;

	sep[0] = star ? ifs_value(shell)[0] : ' ';
	sep[1] = '\0';
	out = ft_strdup("");
	i = 0;
	while (i < shell->nparams)
	{
		tmp = ft_strjoin(out, i ? sep : "");
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
	if (name[0] == '-')
		return (option_string(shell));
	if (name[0] == '#')
		return (ft_itoa(shell->nparams));
	if (name[0] == '@' || name[0] == '*')
		return (join_params(shell, name[0] == '*'));
	if (name[0] == '$')
		return (ft_itoa((int)shell->pid));
	if (name[0] == '!')
		return (shell->last_bg ? ft_itoa((int)shell->last_bg) : ft_strdup(""));
	if (ft_strcmp(name, "LINENO") == 0)
		return (ft_itoa(shell->lineno));
	if (ft_strcmp(name, "PIPESTATUS") == 0)
		return (ft_itoa(shell->npipe ? shell->pipe_st[0] : 0));
	if (!ft_isdigit(name[0]))
		return (dynamic_value(name, shell));
	n = ft_atoi(name);
	if (n == 0)
		return (ft_strdup(shell->argv0));
	if (n <= shell->nparams)
		return (ft_strdup(shell->params[n - 1]));
	return (ft_strdup(""));
}

static char	*lookup(const char *name, t_shell *shell)
{
	char	*value;
	int		n;

	value = special_value(name, shell);
	if (value)
	{
		n = ft_isdigit(name[0]) ? ft_atoi(name) : 0;
		if ((n > shell->nparams) || (name[0] == '!' && !shell->last_bg))
			return (free(value), NULL);
		return (value);
	}
	if (arr_is(env_get(shell->env, name)))
		return (arr_first(env_get(shell->env, name)));
	if (env_get(shell->env, name))
		return (ft_strdup(env_get(shell->env, name)));
	return (NULL);
}

static char	*trim_prefix(char *value, const char *pat, int longest)
{
	size_t	len;
	size_t	i;
	char	*head;
	int		hit;

	len = ft_strlen(value);
	i = longest ? len + 1 : 0;
	while (longest ? i-- > 0 : i <= len)
	{
		head = ft_substr(value, 0, i);
		hit = head && glob_match(pat, head);
		free(head);
		if (hit)
			return (ft_strdup(value + i));
		i += !longest;
	}
	return (ft_strdup(value));
}

static char	*trim_suffix(char *value, const char *pat, int longest)
{
	size_t	len;
	size_t	i;
	int		hit;

	len = ft_strlen(value);
	i = longest ? 0 : len + 1;
	while (longest ? i <= len : i-- > 0)
	{
		hit = glob_match(pat, value + i);
		if (hit)
			return (ft_substr(value, 0, i));
		i += longest;
	}
	return (ft_strdup(value));
}

static char	*apply_op(char *name, char *value, const char *rest, t_shell *shell)
{
	int		colon;
	char	op;
	char	*word;
	char	*res;
	int		unset;
	int		longest;
	colon = (rest[0] == ':');
	op = rest[colon];
	unset = !value || (colon && !*value);
	longest = (op == '#' || op == '%') && rest[colon + 1] == op;
	word = expand_variables((char *)rest + colon + 1 + longest, shell);
	res = NULL;
	if (op == '#' || op == '%')
		res = op == '#' ? trim_prefix(value ? value : "", word, longest)
			: trim_suffix(value ? value : "", word, longest);
	else if (op == '-' || op == '=' || op == '+' || op == '?')
	{
		res = ft_strdup(value ? value : "");
		if (op == '+' ? !unset : unset)
		{
			free(res);
			res = op == '+' ? ft_strdup(word) : ft_strdup(word);
			if (op == '=' && (ft_isalpha(name[0]) || name[0] == '_'))
				env_assign(&shell->env, name, word);
			if (op == '?')
				ml_put_all(2, "minishell: ", name, ": ",
					*word ? word : "parameter null or not set", "\n", NULL);
			if (op == '?')
			{
				res[0] = '\0';
				shell->expand_error = 1;
			}
		}
		else if (op == '+')
			res[0] = '\0';
	}
	return (free(word), res ? res : ft_strdup(value ? value : ""));
}

static void	check_unset(const char *name, char *value, t_shell *shell)
{
	if (value || !(shell->opts & OPT_U) || !name[0]
		|| ft_strchr("@*?#$-", name[0]))
		return ;
	ml_put_all(2, "minishell: ", name, ": unbound variable\n", NULL);
	shell->expand_error = 1;
}

static char	*param_expand(const char *in, t_shell *shell)
{
	char	*name;
	char	*value;
	char	*res;
	int		n;

	if (in[0] == '#' && in[1])
	{
		name = ft_strdup(in + 1);
		value = lookup(name, shell);
		check_unset(name, value, shell);
		res = ft_itoa(value ? (int)ft_strlen(value) : 0);
		return (free(name), free(value), res);
	}
	n = name_len(in, 1);
	name = ft_substr(in, 0, n);
	value = lookup(name, shell);
	if (!in[n] || in[n] == '#' || in[n] == '%')
		check_unset(name, value, shell);
	if (!in[n])
		res = value ? ft_strdup(value) : ft_strdup("");
	else
		res = apply_op(name, value, in + n, shell);
	return (free(name), free(value), res);
}

static char	*get_var_value(char *var_name, t_shell *shell)
{
	int		len;
	char	*name;
	char	*value;

	if (var_name[0] == '{')
	{
		len = brace_end(var_name);
		name = ft_substr(var_name, 1, len - 1);
		value = param_expand(name, shell);
		return (free(name), value);
	}
	len = name_len(var_name, 0);
	if (len == 0)
		return (ft_strdup("$"));
	name = ft_substr(var_name, 0, len);
	value = lookup(name, shell);
	check_unset(name, value, shell);
	free(name);
	return (value ? value : ft_strdup(""));
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
	return (s[0] == '\\' && s[1] && ft_strchr("$\\`\n", s[1]));
}

static int	heredoc_subst(t_buf *buf, char *line, int *i, t_shell *shell)
{
	char	*value;
	int		used;

	value = expand_subst_at(&line[*i], &used, shell, 1);
	if (!value)
		return (-1);
	*i += used;
	if (!buf_put(buf, value, ft_strlen(value)))
		return (free(value), 0);
	return (free(value), 1);
}

static int	heredoc_chunk(t_buf *buf, char *line, int *i, t_shell *shell)
{
	char	*value;
	int		start;
	int		done;

	start = *i;
	while (line[*i] && !(line[*i] == '$' && is_ref(&line[*i + 1]))
		&& !heredoc_escape(&line[*i]) && line[*i] != '`'
		&& !(line[*i] == '$' && line[*i + 1] == '('))
		(*i)++;
	if (*i > start && !buf_put(buf, line + start, *i - start))
		return (0);
	if (heredoc_escape(&line[*i]))
	{
		*i += 2;
		if (line[*i - 1] == '\n')
			return (1);
		return (buf_put(buf, line + *i - 1, 1));
	}
	if (!line[*i])
		return (1);
	if (line[*i] == '`' || line[*i + 1] == '(')
	{
		done = heredoc_subst(buf, line, i, shell);
		if (done >= 0)
			return (done);
		return ((*i)++, buf_put(buf, line + *i - 1, 1));
	}
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
# define KEEP 8
# define EMPTYQ 16

typedef struct s_word
{
	char	*str;
	char	*mask;
	int		len;
	size_t	cap;
	int		at_empty;
	int		noglob;
	int		nquote;
	int		nat;
	int		lit;
	int		emitted;
	int		marks;
	const char	*ifs;
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
	return (s[0] == '@' || (s[0] == '{' && s[1] == '@' && s[2] == '}'));
}

static void	put_at_words(t_word *w, t_shell *shell)
{
	int	i;

	if (shell->nparams == 0)
		w->nat++;
	i = 0;
	while (i < shell->nparams)
	{
		if (i)
			put_char(w, ' ', SEP | QUOTED | KEEP);
		put_value(w, ft_strdup(shell->params[i]), QUOTED);
		i++;
	}
}

static int	is_plain_params(const char *s)
{
	if (s[0] == '@' || s[0] == '*')
		return (1);
	return (s[0] == '{' && (s[1] == '@' || s[1] == '*') && s[2] == '}');
}

static void	put_split_params(t_word *w, t_shell *shell)
{
	int	i;

	i = 0;
	while (i < shell->nparams)
	{
		if (i)
			put_char(w, ' ', SEP);
		put_value(w, ft_strdup(shell->params[i]), SPLIT);
		i++;
	}
}

static void	fill_chars(t_word *w, char *raw, int quote, t_shell *shell);

static int	assign_default(char *name, char *text, t_shell *shell)
{
	if ((!ft_isalpha(name[0]) && name[0] != '_') || env_is_readonly(shell->env,
			name))
	{
		ml_put_all(2, "minishell: ", name, ": cannot assign in this way\n",
			NULL);
		shell->expand_error = 1;
		return (0);
	}
	env_assign(&shell->env, name, text);
	return (1);
}

static char	*tilde_word(char *word, t_shell *shell)
{
	char	*home;

	home = env_get(shell->env, "HOME");
	if (word[0] != '~' || (word[1] && word[1] != '/') || !home)
		return (ft_strdup(word));
	return (ft_strjoin(home, word + 1));
}

static void	use_word(t_word *w, char *name, char op, char *word_ctx[3])
{
	t_shell	*shell;
	char	*word;
	char	*flat;
	int		quote;

	shell = (t_shell *)word_ctx[2];
	quote = word_ctx[1][0];
	word = quote ? ft_strdup(word_ctx[0]) : tilde_word(word_ctx[0], shell);
	if (op == '-' || op == '+')
	{
		w->lit = SPLIT;
		fill_chars(w, word, quote, shell);
		w->lit = 0;
		return (free(word));
	}
	flat = expand_variables(word, shell);
	free(word);
	if (op == '=' && flat && assign_default(name, flat, shell))
		put_value(w, ft_strdup(flat), quote ? QUOTED : SPLIT);
	else if (op == '?')
	{
		ml_put_all(2, "minishell: ", name, ": ",
			flat && *flat ? flat : "parameter null or not set", "\n", NULL);
		shell->expand_error = 1;
	}
	free(flat);
}


char	*expand_pattern(char *word, t_shell *shell)
{
	t_word	t;
	char	*pat;
	int		i;
	int		j;

	ft_bzero(&t, sizeof(t));
	t.ifs = "";
	t.cap = ft_strlen(word);
	t.str = ft_calloc(t.cap + 1, 1);
	t.mask = ft_calloc(t.cap + 1, 1);
	word = tilde_word(word, shell);
	t.cap = ft_strlen(word);
	free(t.str);
	free(t.mask);
	t.str = ft_calloc(t.cap + 1, 1);
	t.mask = ft_calloc(t.cap + 1, 1);
	fill_chars(&t, word, 0, shell);
	free(word);
	pat = ft_calloc(t.len * 2 + 1, 1);
	i = 0;
	j = 0;
	while (pat && t.str && i < t.len)
	{
		if ((t.mask[i] & QUOTED) && ft_strchr("*?[]\\", t.str[i]))
			pat[j++] = '\\';
		pat[j++] = t.str[i++];
	}
	free(t.str);
	free(t.mask);
	return (pat);
}

static int	put_trim(t_word *w, char *inner, int quote, t_shell *shell)
{
	char	*name;
	char	*value;
	char	*pat;
	char	*res;
	int		n;
	int		longest;

	n = name_len(inner, 1);
	if (!n || (inner[n] != '#' && inner[n] != '%'))
		return (0);
	longest = inner[n + 1] == inner[n];
	name = ft_substr(inner, 0, n);
	value = lookup(name, shell);
	check_unset(name, value, shell);
	pat = expand_pattern(inner + n + 1 + longest, shell);
	if (inner[n] == '#')
		res = trim_prefix(value ? value : "", pat ? pat : "", longest);
	else
		res = trim_suffix(value ? value : "", pat ? pat : "", longest);
	put_value(w, res, quote ? QUOTED : SPLIT);
	return (free(name), free(value), free(pat), 1);
}

static char	*pattern_of(const char *s, int start, int len, t_shell *shell)
{
	char	*raw;
	char	*pat;

	raw = ft_substr(s, start, len);
	pat = expand_pattern(raw ? raw : "", shell);
	return (free(raw), pat);
}

static long	arith_text(const char *text, t_shell *shell)
{
	char	*flat;
	char	*res;
	long	v;

	flat = expand_variables((char *)text, shell);
	res = arith_string(flat ? flat : "", shell);
	v = res ? ft_atoi(res) : 0;
	return (free(flat), free(res), v);
}

static long	arith_part(const char *s, int n, t_shell *shell)
{
	char	*part;
	long	v;

	part = ft_substr(s, 0, n);
	v = arith_text(part ? part : "", shell);
	return (free(part), v);
}

static void	put_slice(t_word *w, int quote, long range[2], char name_has[2],
				t_shell *shell)
{
	long	i;
	long	total;
	long	end;
	long	off;
	char	*item;

	total = shell->nparams + 1;
	off = range[0] < 0 ? range[0] + total : range[0];
	if (off < 0 || off >= total)
		return ;
	end = total;
	if (name_has[1])
		end = range[1] < 0 ? total + range[1] : off + range[1];
	i = off;
	while (i < end && i < total)
	{
		if (i > off)
			put_char(w, ' ', name_has[0] == '@' ? (quote ? SEP | QUOTED | KEEP
					: SEP) : QUOTED);
		item = i == 0 ? (char *)shell->argv0 : shell->params[i - 1];
		put_value(w, ft_strdup(item), quote ? QUOTED : SPLIT);
		i++;
	}
}

static int	put_substring(t_word *w, char *inner, int n, int quote,
				t_shell *shell)
{
	char	*name;
	char	*value;
	char	*rest;
	long	range[2];
	char	flags[2];
	int		c;

	rest = inner + n + 1;
	c = 0;
	while (rest[c] && rest[c] != ':')
		c++;
	range[0] = arith_part(rest, c, shell);
	range[1] = rest[c] ? arith_text(rest + c + 1, shell) : 0;
	name = ft_substr(inner, 0, n);
	flags[0] = name[0];
	flags[1] = rest[c] == ':';
	if (n == 1 && (name[0] == '@' || name[0] == '*'))
		put_slice(w, quote, range, flags, shell);
	else
	{
		value = lookup(name, shell);
		check_unset(name, value, shell);
		put_value(w, param_substring(value ? value : "", range[0], range[1],
				flags[1]), quote ? QUOTED : SPLIT);
		free(value);
	}
	return (free(name), 1);
}

static int	put_replace(t_word *w, char *inner, int n, int quote,
				t_shell *shell)
{
	char	*name;
	char	*value;
	char	*pat;
	char	*rep;
	int		mode;
	int		c;

	name = ft_substr(inner, 0, n);
	value = lookup(name, shell);
	check_unset(name, value, shell);
	mode = 0;
	c = n + 1;
	if (inner[c] == '/' || inner[c] == '#' || inner[c] == '%')
		mode = inner[c] == '/' ? 1 : (inner[c] == '#' ? 2 : 3);
	c += mode > 0;
	n = c;
	while (inner[n] && inner[n] != '/')
		n += 1 + (inner[n] == '\\' && inner[n + 1]);
	pat = pattern_of(inner, c, n - c, shell);
	rep = expand_variables(inner[n] ? inner + n + 1 : "", shell);
	put_value(w, param_replace(value ? value : "", pat ? pat : "",
			rep ? rep : "", mode), quote ? QUOTED : SPLIT);
	return (free(name), free(value), free(pat), free(rep), 1);
}

static int	put_case(t_word *w, char *inner, int n, int quote, t_shell *shell)
{
	char	*name;
	char	*value;
	char	*pat;
	int		all;

	name = ft_substr(inner, 0, n);
	value = lookup(name, shell);
	check_unset(name, value, shell);
	all = inner[n + 1] == inner[n];
	pat = pattern_of(inner, n + 1 + all, ft_strlen(inner), shell);
	put_value(w, param_case(value ? value : "", pat ? pat : "",
			inner[n] == '^', all), quote ? QUOTED : SPLIT);
	return (free(name), free(value), free(pat), 1);
}

static int	put_transform(t_word *w, char *inner, int n, int quote,
				t_shell *shell)
{
	char	*name;
	char	*value;
	char	*res;

	name = ft_substr(inner, 0, n);
	value = lookup(name, shell);
	if (inner[n + 1] == 'Q')
		res = param_quote(value ? value : "");
	else if (inner[n + 1] == 'U' || inner[n + 1] == 'L')
		res = param_case(value ? value : "", "", inner[n + 1] == 'U', 1);
	else
		res = param_case(value ? value : "", "", 1, 0);
	put_value(w, res, quote ? QUOTED : SPLIT);
	return (free(name), free(value), 1);
}

static int	put_indirect(t_word *w, char *inner, int quote, t_shell *shell)
{
	char	*name;
	char	*value;
	char	*target;
	int		n;

	n = name_len(inner + 1, 1);
	if (!n || inner[1 + n])
		return (0);
	name = ft_substr(inner, 1, n);
	value = lookup(name, shell);
	target = value ? lookup(value, shell) : NULL;
	put_value(w, target, quote ? QUOTED : SPLIT);
	return (free(name), free(value), 1);
}

static int	put_extra(t_word *w, char *inner, int quote, t_shell *shell)
{
	int	n;

	if (inner[0] == '#' && (inner[1] == '@' || inner[1] == '*') && !inner[2])
		return (put_value(w, ft_itoa(shell->nparams), quote ? QUOTED : SPLIT), 1);
	if (inner[0] == '!')
		return (put_indirect(w, inner, quote, shell));
	n = name_len(inner, 1);
	if (!n || inner[0] == '#')
		return (0);
	if (inner[n] == ':' && inner[n + 1] && !ft_strchr("-=+?", inner[n + 1]))
		return (put_substring(w, inner, n, quote, shell));
	if (inner[n] == '/')
		return (put_replace(w, inner, n, quote, shell));
	if (inner[n] == '^' || inner[n] == ',')
		return (put_case(w, inner, n, quote, shell));
	if (inner[n] == '@' && ft_strchr("QULu", inner[n + 1]) && inner[n + 1]
		&& !inner[n + 2])
		return (put_transform(w, inner, n, quote, shell));
	return (0);
}

typedef struct s_aref
{
	char	*value;
	char	*owned;
	char	*sub;
	char	*rest;
	int		count;
	int		quote;
	int		neg;
	int		bang;
}	t_aref;

static void	put_elems(t_word *w, char **el, int range[2], t_aref *r)
{
	int	i;

	i = range[0];
	while (i < range[1])
	{
		if (i > range[0])
			put_char(w, ' ', r->sub[0] == '@' ? (r->quote ? SEP | QUOTED | KEEP
					: SEP) : (r->quote ? QUOTED : SEP));
		put_value(w, ft_strdup(el[i]), r->quote ? QUOTED : SPLIT);
		i++;
	}
	if (range[0] >= range[1] && r->quote && r->sub[0] == '@')
		w->nat++;
}

static void	slice_range(t_aref *r, int range[2], t_shell *shell)
{
	char	*colon;

	range[0] = 0;
	range[1] = r->count;
	if (r->rest[0] != ':' || !r->rest[1])
		return ;
	colon = ft_strchr(r->rest + 1, ':');
	range[0] = (int)arith_part(r->rest + 1, colon ? colon - r->rest - 1
			: (int)ft_strlen(r->rest + 1), shell);
	if (range[0] < 0)
		range[0] += r->count;
	range[0] = range[0] < 0 ? r->count : (range[0] > r->count ? r->count
			: range[0]);
	if (colon)
		range[1] = range[0] + (int)arith_text(colon + 1, shell);
	range[1] = range[1] > r->count ? r->count : range[1];
}

static int	array_all(t_word *w, t_aref *r, t_shell *shell)
{
	char	**el;
	int		range[2];
	int		n;
	int		i;

	el = r->bang ? NULL : arr_split(r->value, &n);
	r->count = r->bang ? arr_count(r->value) : n;
	if (r->neg)
		put_value(w, ft_itoa(r->count), r->quote ? QUOTED : SPLIT);
	else
	{
		if (r->bang)
		{
			el = ft_calloc(r->count + 1, sizeof(char *));
			i = -1;
			while (el && ++i < r->count)
				el[i] = ft_itoa(i);
		}
		slice_range(r, range, shell);
		put_elems(w, el, range, r);
	}
	return (free_str_array(el), 1);
}

static int	array_one(t_word *w, t_aref *r, t_shell *shell)
{
	char	*elem;
	char	*num;
	long	idx;

	if (r->rest[0])
		return (0);
	num = arith_string(r->sub, shell);
	idx = num ? ft_atoi(num) : 0;
	free(num);
	elem = arr_elem(r->value, idx);
	if (r->neg)
	{
		put_value(w, ft_itoa(elem ? (int)ft_strlen(elem) : 0),
			r->quote ? QUOTED : SPLIT);
		return (free(elem), 1);
	}
	put_value(w, elem ? elem : ft_strdup(""), r->quote ? QUOTED : SPLIT);
	return (1);
}

static int	put_array(t_word *w, char *inner, int quote, t_shell *shell)
{
	t_aref	r;
	char	*name;
	int		p;
	int		n;
	int		close;
	int		done;

	p = inner[0] == '#' || inner[0] == '!';
	n = 0;
	while (ft_isalnum(inner[p + n]) || inner[p + n] == '_')
		n++;
	close = p + n + 1;
	while (inner[p + n] == '[' && inner[close] && inner[close] != ']')
		close++;
	if (!n || inner[p + n] != '[' || inner[close] != ']'
		|| ft_isdigit(inner[p]))
		return (0);
	name = ft_substr(inner, p, n);
	r.value = env_get(shell->env, name);
	r.owned = NULL;
	if (ft_strcmp(name, "PIPESTATUS") == 0)
		r.value = r.owned = pipestatus_value(shell);
	r.sub = ft_substr(inner, p + n + 1, close - p - n - 1);
	r.rest = inner + close + 1;
	r.quote = quote;
	r.neg = inner[0] == '#';
	r.bang = inner[0] == '!';
	done = (r.sub[0] == '@' || r.sub[0] == '*') && !r.sub[1]
		? array_all(w, &r, shell) : array_one(w, &r, shell);
	free(r.owned);
	return (free(name), free(r.sub), done);
}

static int	put_brace(t_word *w, char *raw, int quote, t_shell *shell)
{
	char	*inner;
	char	*name;
	char	*value;
	char	*ctx[3];
	char	qc[2];
	int		n;
	int		colon;

	inner = ft_substr(raw, 1, brace_end(raw) - 1);
	if (put_array(w, inner, quote, shell))
		return (free(inner), 1);
	if (inner[0] != '#' && put_trim(w, inner, quote, shell))
		return (free(inner), 1);
	if (put_extra(w, inner, quote, shell))
		return (free(inner), 1);
	n = inner[0] == '#' ? hash_name(inner) : name_len(inner, 1);
	colon = inner[n] == ':';
	if (!inner[0] || !n || !inner[n] || !ft_strchr("-=+?", inner[n + colon]))
		return (free(inner), 0);
	name = ft_substr(inner, 0, n);
	value = lookup(name, shell);
	qc[0] = (char)quote;
	qc[1] = '\0';
	ctx[0] = inner + n + colon + 1;
	ctx[1] = qc;
	ctx[2] = (char *)shell;
	if (inner[n + colon] == '+' ? (value && (!colon || *value))
		: (!value || (colon && !*value)))
		use_word(w, name, inner[n + colon], ctx);
	else if (inner[n + colon] != '+')
		put_value(w, ft_strdup(value), quote ? QUOTED : SPLIT);
	return (free(inner), free(name), free(value), 1);
}

static int	put_dollar(t_word *w, char *raw, int quote, t_shell *shell)
{
	if (quote == '"' && is_at_ref(raw + 1))
		put_at_words(w, shell);
	else if (raw[1] == '{' && put_brace(w, raw + 1, quote, shell))
		;
	else if (!quote && is_plain_params(raw + 1))
		put_split_params(w, shell);
	else
		put_value(w, get_var_value(raw + 1, shell), quote ? QUOTED : SPLIT);
	return (ref_length(raw + 1) + 1);
}

static char	*unescape_backtick(const char *src, int len, int dq)
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
		if (src[i] == '\\' && i + 1 < len && (ft_strchr("`\\$", src[i + 1]) || (dq && src[i + 1] == '"')))
			i++;
		out[j++] = src[i++];
	}
	out[j] = '\0';
	return (out);
}

char	*expand_subst_at(const char *s, int *consumed, t_shell *shell, int dq)
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
		return (run_substitution(shell, unescape_backtick(s + 1, end - 2, dq)));
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

	out = expand_subst_at(s, &used, shell, quote == '"');
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

static void	fill_chars(t_word *w, char *raw, int quote, t_shell *shell)
{
	int	i;
	int	n;
	int	opened;

	i = 0;
	opened = quote ? -1 : 0;
	while (raw[i])
	{
		n = put_special(w, raw + i, quote, shell);
		if (n)
			i += n;
		else if ((raw[i] == '\'' || raw[i] == '"')
			&& (!quote || quote == raw[i]))
		{
			w->nquote += (quote == 0);
			if (quote == 0)
				opened = i;
			else if (opened >= 0 && opened + 1 == i && w->marks)
				put_char(w, 1, QUOTED | EMPTYQ);
			quote = (quote == 0) * raw[i++];
		}
		else if (raw[i] == '$' && quote != '\'' && is_ref(&raw[i + 1]))
			i += put_dollar(w, &raw[i], quote, shell);
		else
			put_char(w, raw[i++], quote ? QUOTED : w->lit);
	}
}

static void	fill_word(t_word *w, char *raw, t_shell *shell)
{
	int		i;
	char	*home;

	i = 0;
	home = env_get(shell->env, "HOME");
	if (raw[0] == '~' && (!raw[1] || raw[1] == '/') && home)
	{
		put_value(w, ft_strdup(home), QUOTED);
		i = 1;
	}
	fill_chars(w, raw + i, 0, shell);
}

static void	emit_one(t_word *w, int start, int end, t_token **tok[2])
{
	char	*pattern;
	char	*plain;
	int		k;
	int		j;
	int		p;
	int		has_glob;

	pattern = malloc((end - start) * 2 + 1);
	plain = malloc(end - start + 1);
	j = 0;
	p = 0;
	has_glob = 0;
	k = start - 1;
	while (pattern && plain && ++k < end)
	{
		if (w->mask[k] & EMPTYQ)
			continue ;
		if (w->mask[k] & QUOTED && ft_strchr("*?[\\", w->str[k]))
			pattern[j++] = '\\';
		else if (ft_strchr("*?[", w->str[k]))
			has_glob = 1;
		pattern[j++] = w->str[k];
		plain[p++] = w->str[k];
	}
	if (pattern && plain)
	{
		pattern[j] = '\0';
		plain[p] = '\0';
		has_glob |= pattern_has_group(pattern);
	}
	if (!has_glob || w->noglob || !pattern || !plain
		|| !expand_glob(pattern, tok[0], tok[1]))
	{
		if (has_glob && !w->noglob && (g_shopts & SHOPT_NULLGLOB))
			free(plain);
		else
			token_add_back(tok[0], tok[1], TOKEN_WORD, plain);
	}
	else
		free(plain);
	free(pattern);
}

static void	emit_empty(t_token **tok[2])
{
	token_add_back(tok[0], tok[1], TOKEN_WORD, ft_strdup(""));
}

static int	is_ifs(t_word *w, int k)
{
	return ((w->mask[k] & SPLIT) && w->ifs[0] && ft_strchr(w->ifs, w->str[k]));
}

static int	is_ifs_space(t_word *w, int k)
{
	return (is_ifs(w, k) && ft_strchr(" \t\n", w->str[k]));
}

static void	close_field(t_word *w, t_token **tok[2], int *start, int k)
{
	if (*start >= 0)
		emit_one(w, *start, k, tok);
	*start = -1;
}

static int	emit_fields(t_word *w, t_token **tok[2])
{
	int	start;
	int	k;
	int	after_delim;
	int	before;

	start = -1;
	after_delim = 1;
	k = -1;
	before = tok[1] && *tok[1] ? 1 : 0;
	(void)before;
	w->emitted = 0;
	while (++k <= w->len)
	{
		if (k < w->len && (w->mask[k] & SEP))
		{
			if (start < 0 && (w->mask[k] & KEEP))
				w->emitted += (emit_empty(tok), 1);
			w->emitted += start >= 0;
			close_field(w, tok, &start, k);
			after_delim = 1;
		}
		else if (k == w->len || is_ifs(w, k))
		{
			w->emitted += start >= 0;
			close_field(w, tok, &start, k);
			if (k < w->len && !is_ifs_space(w, k))
				w->emitted += after_delim && (emit_empty(tok), 1);
			after_delim = k < w->len && !is_ifs_space(w, k) ? 1 : after_delim;
		}
		else
		{
			start += (start < 0) * (k + 1);
			after_delim = 0;
		}
	}
	if (w->len > 0 && (w->mask[w->len - 1] & SEP) && (w->mask[w->len - 1] & KEEP))
		w->emitted += (emit_empty(tok), 1);
	return (w->emitted);
}

static void	expand_word(char *raw, t_shell *shell, t_token **head,
				t_token **cur)
{
	if ((raw[0] == '<' || raw[0] == '>') && raw[1] == '(' && raw[ft_strlen(raw) - 1] == ')')
	{
		token_add_back(head, cur, TOKEN_WORD, run_procsub(shell,
				ft_substr(raw, 2, ft_strlen(raw) - 3), raw[0] == '>'));
		return ;
	}
	t_word	w;
	t_token	**tok[2];
	char	*home;
	size_t	cap;

	home = env_get(shell->env, "HOME");
	cap = ft_strlen(raw) + (home ? ft_strlen(home) : 0);
	w.len = 0;
	w.cap = cap;
	w.at_empty = 0;
	w.noglob = (shell->opts & OPT_F) != 0;
	w.nquote = 0;
	w.nat = 0;
	w.lit = 0;
	w.marks = 1;
	w.ifs = ifs_value(shell);
	w.str = ft_calloc(cap + 1, 1);
	w.mask = ft_calloc(cap + 1, 1);
	tok[0] = head;
	tok[1] = cur;
	if (w.str && w.mask)
	{
		fill_word(&w, raw, shell);
		if (!emit_fields(&w, tok)
			&& w.nquote > w.nat)
			token_add_back(head, cur, TOKEN_WORD, ft_strdup(""));
	}
	free(w.str);
	free(w.mask);
}

static void	expand_braced(char *raw, t_shell *shell, t_token **head, t_token **cur)
{
	char	**words;
	int		i;

	words = (shell->opts & OPT_BRACE) ? brace_expand(raw) : NULL;
	if (!words)
		return (expand_word(raw, shell, head, cur));
	i = 0;
	while (words[i])
		expand_word(words[i++], shell, head, cur);
	free_str_array(words);
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
			expand_braced(old->value, shell, &head, &cur);
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
	cap = ft_strlen(token) + (home ? ft_strlen(home) : 0);
	w.len = 0;
	w.cap = cap;
	w.at_empty = 0;
	w.noglob = (shell->opts & OPT_F) != 0;
	w.nquote = 0;
	w.nat = 0;
	w.lit = 0;
	w.marks = 0;
	w.ifs = ifs_value(shell);
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
