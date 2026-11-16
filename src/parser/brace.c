#include "../../includes/parser.h"

#define BRACE_MAX 20000

typedef struct s_brace
{
	char	**list;
	int		n;
	int		cap;
}	t_brace;

typedef struct s_seq
{
	long	from;
	long	to;
	long	step;
	int		width;
	int		chars;
}	t_seq;

static void	expand_into(t_brace *b, const char *word);

static int	skip_quoted(const char *s, int i)
{
	char	q;

	q = s[i++];
	while (s[i] && s[i] != q)
		i += (s[i] == '\\' && q == '"' && s[i + 1]) + 1;
	return (s[i] ? i + 1 : i);
}

static int	skip_atom(const char *s, int i)
{
	if (s[i] == '\\' && s[i + 1])
		return (i + 2);
	if (s[i] == '\'' || s[i] == '"' || s[i] == '`')
		return (skip_quoted(s, i));
	if (s[i] == '$' && s[i + 1] == '{')
		return (skip_brace(s, i));
	if (s[i] == '$' && s[i + 1] == '(')
		return (skip_dollar_paren(s, i));
	return (i + 1);
}

static int	find_close(const char *s, int i)
{
	int	depth;

	depth = 0;
	while (s[i])
	{
		if (s[i] == '{')
			depth++;
		else if (s[i] == '}' && --depth == 0)
			return (i);
		i = skip_atom(s, i);
		if (i < 0)
			return (-1);
	}
	return (-1);
}

static int	next_comma(const char *s, int i, int end)
{
	int	depth;

	depth = 0;
	while (i < end)
	{
		if (s[i] == '{')
			depth++;
		else if (s[i] == '}')
			depth--;
		else if (s[i] == ',' && depth == 0)
			return (i);
		i = skip_atom(s, i);
		if (i < 0)
			return (-1);
	}
	return (-1);
}

static void	add(t_brace *b, char *str)
{
	char	**grown;

	if (!str)
		return ;
	if (b->n + 1 >= b->cap)
	{
		b->cap = b->cap ? b->cap * 2 : 8;
		grown = ft_calloc(b->cap + 1, sizeof(char *));
		if (!grown)
			return (free(str));
		ft_memcpy(grown, b->list, sizeof(char *) * b->n);
		free(b->list);
		b->list = grown;
	}
	b->list[b->n++] = str;
}

static void	emit(t_brace *b, const char *pre, const char *mid, const char *post)
{
	char	*tmp;
	char	*res;

	tmp = ft_strjoin(pre, mid);
	res = ft_strjoin(tmp, post);
	free(tmp);
	if (res)
		expand_into(b, res);
	free(res);
}

static int	parse_num(const char *s, int len, long *out)
{
	int		i;
	long	v;

	i = (s[0] == '-' || s[0] == '+');
	if (i >= len || len - i > 12)
		return (0);
	v = 0;
	while (i < len)
	{
		if (!ft_isdigit(s[i]))
			return (0);
		v = v * 10 + (s[i++] - '0');
	}
	*out = s[0] == '-' ? -v : v;
	return (1);
}

static int	zero_padded(const char *s, int len)
{
	int	i;

	i = s[0] == '-' || s[0] == '+';
	return (len - i > 1 && s[i] == '0');
}

static int	parse_seq(const char *body, int len, t_seq *q)
{
	int	d1;
	int	d2;
	int	end;

	d1 = 0;
	while (d1 + 1 < len && !(body[d1] == '.' && body[d1 + 1] == '.'))
		d1++;
	if (d1 + 1 >= len)
		return (0);
	d2 = d1 + 2;
	while (d2 + 1 < len && !(body[d2] == '.' && body[d2 + 1] == '.'))
		d2++;
	end = d2 + 1 < len ? d2 : len;
	q->step = 1;
	if (end < len && !parse_num(body + end + 2, len - end - 2, &q->step))
		return (0);
	q->chars = 0;
	q->width = 0;
	if (parse_num(body, d1, &q->from) && parse_num(body + d1 + 2,
			end - d1 - 2, &q->to))
	{
		if (zero_padded(body, d1) || zero_padded(body + d1 + 2, end - d1 - 2))
			q->width = d1 > end - d1 - 2 ? d1 : end - d1 - 2;
		return (1);
	}
	q->chars = d1 == 1 && end - d1 - 2 == 1 && ft_isalpha(body[0])
		&& ft_isalpha(body[d1 + 2]);
	q->from = body[0];
	q->to = body[d1 + 2];
	return (q->chars);
}

static char	*seq_text(long v, const t_seq *q)
{
	char	*num;
	char	*pad;
	char	*res;
	int		len;

	if (q->chars)
		return (ft_substr((char []){(char)v, 0}, 0, 1));
	num = ft_itoa((int)(v < 0 ? -v : v));
	len = (int)ft_strlen(num) + (v < 0);
	pad = ft_calloc((q->width > len ? q->width : len) + 1, 1);
	if (!num || !pad)
		return (free(num), free(pad), NULL);
	ft_memset(pad, '0', q->width > len ? q->width - len : 0);
	ft_strlcat(pad, num, ft_strlen(pad) + ft_strlen(num) + 1);
	res = v < 0 ? ft_strjoin("-", pad) : ft_strdup(pad);
	return (free(num), free(pad), res);
}

static void	expand_seq(t_brace *b, const char *pre, t_seq *q, const char *post)
{
	long	cur;
	long	step;
	char	*text;

	step = q->step < 0 ? -q->step : q->step;
	if (step == 0)
		step = 1;
	cur = q->from;
	while (b->n < BRACE_MAX)
	{
		text = seq_text(cur, q);
		emit(b, pre, text, post);
		free(text);
		if (q->from <= q->to ? cur + step > q->to : cur - step < q->to)
			break ;
		cur += q->from <= q->to ? step : -step;
	}
}

static int	try_brace(t_brace *b, const char *word, int open, int close)
{
	char	*pre;
	char	*item;
	t_seq	q;
	int		start;
	int		end;

	pre = ft_substr(word, 0, open);
	start = open + 1;
	if (next_comma(word, start, close) < 0)
	{
		if (!parse_seq(word + start, close - start, &q))
			return (free(pre), 0);
		return (expand_seq(b, pre, &q, word + close + 1), free(pre), 1);
	}
	end = start - 1;
	while (end != close)
	{
		start = end + 1;
		end = next_comma(word, start, close);
		if (end < 0)
			end = close;
		item = ft_substr(word, start, end - start);
		emit(b, pre, item, word + close + 1);
		free(item);
	}
	return (free(pre), 1);
}

static void	expand_into(t_brace *b, const char *word)
{
	int	i;
	int	close;

	i = 0;
	while (word[i] && b->n < BRACE_MAX)
	{
		if (word[i] == '{')
		{
			close = find_close(word, i);
			if (close > 0 && try_brace(b, word, i, close))
				return ;
			i++;
			continue ;
		}
		i = skip_atom(word, i);
		if (i < 0)
			break ;
	}
	add(b, ft_strdup(word));
}

char	**brace_expand(const char *word)
{
	t_brace	b;

	ft_bzero(&b, sizeof(b));
	expand_into(&b, word);
	if (!b.list)
		add(&b, ft_strdup(word));
	return (b.list);
}
