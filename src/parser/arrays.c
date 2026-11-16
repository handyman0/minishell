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
