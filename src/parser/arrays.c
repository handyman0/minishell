#include "../../includes/parser.h"

int	arr_is(const char *v)
{
	return (v && v[0] == ARR_MARK);
}

int	arr_count(const char *v)
{
	int	n;
	int	i;

	if (!arr_is(v))
		return (v ? 1 : 0);
	if (!v[1])
		return (0);
	n = 1;
	i = 1;
	while (v[i])
		n += v[i++] == ARR_SEP;
	return (n);
}

char	**arr_split(const char *v, int *n)
{
	char	**out;
	int		i;
	int		start;
	int		k;

	*n = arr_count(v);
	out = ft_calloc(*n + 1, sizeof(char *));
	if (!out)
		return (NULL);
	if (!arr_is(v))
		return (*n ? (out[0] = ft_strdup(v), out) : out);
	i = 1;
	k = 0;
	start = 1;
	while (k < *n)
	{
		while (v[i] && v[i] != ARR_SEP)
			i++;
		out[k++] = ft_substr(v, start, i - start);
		start = ++i;
	}
	return (out);
}

char	*arr_join(char **elems, int n)
{
	char	*out;
	char	*tmp;
	int		i;

	out = ft_strdup((char []){ARR_MARK, 0});
	i = 0;
	while (out && i < n)
	{
		if (i)
		{
			tmp = ft_strjoin(out, (char []){ARR_SEP, 0});
			free(out);
			out = tmp;
		}
		tmp = ft_strjoin(out, elems[i++]);
		free(out);
		out = tmp;
	}
	return (out);
}

char	*arr_elem(const char *v, long idx)
{
	char	**el;
	char	*res;
	int		n;

	el = arr_split(v, &n);
	if (idx < 0)
		idx += n;
	res = el && idx >= 0 && idx < n ? ft_strdup(el[idx]) : NULL;
	free_str_array(el);
	return (res);
}

char	*arr_first(const char *v)
{
	char	*res;

	res = arr_elem(v, 0);
	return (res ? res : ft_strdup(""));
}

char	*arr_set(const char *v, long idx, const char *val)
{
	char	**el;
	char	**grown;
	char	*res;
	int		n;

	el = arr_split(v, &n);
	if (idx < 0)
		idx += n;
	if (idx < 0)
		return (free_str_array(el), NULL);
	if (idx >= n)
	{
		grown = ft_calloc(idx + 2, sizeof(char *));
		ft_memcpy(grown, el, sizeof(char *) * n);
		free(el);
		el = grown;
		while (n < idx)
			el[n++] = ft_strdup("");
		el[n++] = ft_strdup(val);
	}
	else
	{
		free(el[idx]);
		el[idx] = ft_strdup(val);
	}
	res = arr_join(el, n);
	return (free_str_array(el), res);
}

int	assign_eq_index(const char *w)
{
	int	i;

	if (!w || !(ft_isalpha(w[0]) || w[0] == '_'))
		return (-1);
	i = 1;
	while (ft_isalnum(w[i]) || w[i] == '_')
		i++;
	if (w[i] == '[')
	{
		while (w[i] && w[i] != ']')
			i++;
		if (!w[i])
			return (-1);
		i++;
	}
	if (w[i] == '+')
		i++;
	return (w[i] == '=' ? i : -1);
}

char	*assign_with_index(const char *base, const char *sub, int plus,
			const char *val, t_shell *shell)
{
	char	*text;
	char	*res;
	char	*num;
	char	*cur;
	long	idx;

	num = NULL;
	if (assoc_is(base) && sub)
	{
		text = expand_variables((char *)sub, shell);
		cur = plus ? assoc_get(base, text ? text : "") : NULL;
		res = assoc_set(base, text ? text : "", plus ? (num = ft_strjoin(cur ? cur : "", val)) : val, 0);
		return (free(text), free(cur), free(num), res);
	}
	if (!sub)
	{
		cur = base && plus ? arr_first(base) : ft_strdup("");
		text = plus ? ft_strjoin(cur, val) : ft_strdup(val);
		free(cur);
		res = base && arr_is(base) ? arr_set(base, 0, text) : text;
		if (res != text)
			free(text);
		return (res);
	}
	text = expand_variables((char *)sub, shell);
	num = arith_string(text ? text : "0", shell);
	idx = num ? ft_atoi(num) : 0;
	free(text);
	free(num);
	cur = plus ? arr_elem(base, idx) : NULL;
	text = plus ? ft_strjoin(cur ? cur : "", val) : ft_strdup(val);
	res = arr_set(base ? base : (char []){ARR_MARK, 0}, idx, text);
	return (free(cur), free(text), res);
}

char	*arr_display(const char *v)
{
	char	**el;
	char	*out;
	char	*tmp;
	char	*idx;
	int		n;
	int		i;

	el = arr_split(v, &n);
	out = ft_strdup("(");
	i = 0;
	while (out && i < n)
	{
		idx = ft_itoa(i);
		tmp = ft_strjoin(out, i ? " [" : "[");
		free(out);
		out = ft_strjoin(tmp, idx);
		free(tmp);
		free(idx);
		tmp = ft_strjoin(out, "]=\"");
		free(out);
		out = ft_strjoin(tmp, el[i++]);
		free(tmp);
		tmp = ft_strjoin(out, "\"");
		free(out);
		out = tmp;
	}
	tmp = ft_strjoin(out, ")");
	return (free(out), free_str_array(el), tmp);
}

char	*pipestatus_value(t_shell *shell)
{
	char	**el;
	char	*res;
	int		i;

	el = ft_calloc(shell->npipe + 1, sizeof(char *));
	i = -1;
	while (el && ++i < shell->npipe)
		el[i] = ft_itoa(shell->pipe_st[i]);
	res = el ? arr_join(el, shell->npipe) : NULL;
	return (free_str_array(el), res);
}

int	assoc_is(const char *v)
{
	return (v && v[0] == ASSOC_MARK);
}

int	arr_like(const char *v)
{
	return (arr_is(v) || assoc_is(v));
}

static char	**assoc_entries(const char *v, int *n)
{
	char	*copy;
	char	**out;

	*n = 0;
	if (!assoc_is(v) || !v[1])
		return (ft_calloc(1, sizeof(char *)));
	copy = ft_strdup(v);
	copy[0] = ARR_MARK;
	out = arr_split(copy, n);
	free(copy);
	return (out);
}

static int	entry_is(const char *entry, const char *key)
{
	size_t	n;

	n = ft_strlen(key);
	return (ft_strncmp(entry, key, n) == 0 && entry[n] == ASSOC_KV);
}

char	*assoc_get(const char *v, const char *key)
{
	char	**el;
	char	*res;
	int		n;
	int		i;

	el = assoc_entries(v, &n);
	res = NULL;
	i = 0;
	while (el && i < n && !res)
	{
		if (entry_is(el[i], key))
			res = ft_strdup(ft_strchr(el[i], ASSOC_KV) + 1);
		i++;
	}
	free_str_array(el);
	return (res);
}

static char	*assoc_join(char **el, int n)
{
	char	*joined;

	joined = arr_join(el, n);
	if (joined)
		joined[0] = ASSOC_MARK;
	return (joined);
}

char	*assoc_set(const char *v, const char *key, const char *val, int drop)
{
	char	**el;
	char	**grown;
	char	*entry;
	char	*tmp;
	int		n;
	int		i;

	el = assoc_entries(v, &n);
	tmp = ft_strjoin(key, (char []){ASSOC_KV, 0});
	entry = ft_strjoin(tmp, val);
	free(tmp);
	i = 0;
	while (i < n && !entry_is(el[i], key))
		i++;
	if (i < n)
	{
		free(el[i]);
		el[i] = drop ? NULL : entry;
		while (drop && i++ < n)
			el[i - 1] = el[i];
		n -= drop;
		entry = drop ? (free(entry), NULL) : NULL;
	}
	else if (!drop)
	{
		grown = ft_calloc(n + 2, sizeof(char *));
		ft_memcpy(grown, el, sizeof(char *) * n);
		free(el);
		el = grown;
		el[n++] = entry;
	}
	else
		free(entry);
	tmp = assoc_join(el, n);
	return (free_str_array(el), tmp);
}

char	**assoc_list(const char *v, int keys, int *n)
{
	char	**el;
	char	*kv;
	int		i;
	char	*item;

	el = assoc_entries(v, n);
	i = 0;
	while (el && i < *n)
	{
		kv = ft_strchr(el[i], ASSOC_KV);
		item = keys ? ft_substr(el[i], 0, kv - el[i]) : ft_strdup(kv + 1);
		free(el[i]);
		el[i++] = item;
	}
	return (el);
}

char	*assoc_display(const char *v)
{
	char	**keys;
	char	**vals;
	char	*out;
	char	*tmp;
	int		n;
	int		i;

	keys = assoc_list(v, 1, &n);
	vals = assoc_list(v, 0, &n);
	out = ft_strdup("(");
	i = 0;
	while (out && i < n)
	{
		tmp = ft_strjoin(out, i ? " [" : "[");
		free(out);
		out = ft_strjoin(tmp, keys[i]);
		free(tmp);
		tmp = ft_strjoin(out, "]=\"");
		free(out);
		out = ft_strjoin(tmp, vals[i++]);
		free(tmp);
		tmp = ft_strjoin(out, "\"");
		free(out);
		out = tmp;
	}
	tmp = ft_strjoin(out, ")");
	return (free(out), free_str_array(keys), free_str_array(vals), tmp);
}

int	g_assoc_hint;

char	*assoc_literal(const char *base, int plus, char **words, int n)
{
	char	*cur;
	char	*next;
	char	*close;
	char	*key;
	int		i;

	cur = ft_strdup(plus && assoc_is(base) ? base : (char []){ASSOC_MARK, 0});
	i = 0;
	while (cur && i < n)
	{
		close = words[i][0] == '[' ? ft_strchr(words[i], ']') : NULL;
		if (close && close[1] == '=')
		{
			key = ft_substr(words[i], 1, close - words[i] - 1);
			next = assoc_set(cur, key, close + 2, 0);
			free(key);
			free(cur);
			cur = next;
		}
		i++;
	}
	return (cur);
}
