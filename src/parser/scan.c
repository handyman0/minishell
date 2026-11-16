#include "../../includes/parser.h"

static int	skip_single(const char *s, int i)
{
	i++;
	while (s[i] && s[i] != '\'')
		i++;
	return (s[i] ? i + 1 : -1);
}

int	skip_backtick(const char *s, int i)
{
	i++;
	while (s[i] && s[i] != '`')
	{
		if (s[i] == '\\' && s[i + 1])
			i++;
		i++;
	}
	return (s[i] ? i + 1 : -1);
}

static int	skip_double(const char *s, int i)
{
	i++;
	while (s[i] && s[i] != '"')
	{
		if (s[i] == '\\' && s[i + 1])
			i += 2;
		else if (s[i] == '$' && s[i + 1] == '(')
		{
			i = skip_dollar_paren(s, i);
			if (i == -1)
				return (-1);
		}
		else if (s[i] == '`')
		{
			i = skip_backtick(s, i);
			if (i == -1)
				return (-1);
		}
		else
			i++;
	}
	return (s[i] ? i + 1 : -1);
}

static int	skip_nested(const char *s, int i)
{
	if (s[i] == '\'')
		return (skip_single(s, i));
	if (s[i] == '"')
		return (skip_double(s, i));
	if (s[i] == '`')
		return (skip_backtick(s, i));
	if (s[i] == '$' && s[i + 1] == '(')
		return (skip_dollar_paren(s, i));
	return (i + 1);
}

typedef struct s_pscan
{
	int		depth;
	int		cases;
	int		bases[8];
	char	delim[64];
	int		strip;
	int		pending;
}	t_pscan;

static int	word_at(const char *s, int i, const char *w)
{
	int	n;

	n = ft_strlen(w);
	if (ft_strncmp(s + i, w, n) != 0)
		return (0);
	if (s[i + n] && !ft_isspace(s[i + n]) && !ft_strchr(";&|()<>", s[i + n]))
		return (0);
	return (i == 0 || ft_isspace(s[i - 1]) || ft_strchr(";&|()", s[i - 1]));
}

static int	read_delim(const char *s, int i, t_pscan *p)
{
	int	n;

	i += 2;
	p->strip = s[i] == '-';
	i += p->strip;
	while (s[i] == ' ' || s[i] == '\t')
		i++;
	n = 0;
	while (s[i] && !ft_isspace(s[i]) && !ft_strchr(";&|()<>", s[i]))
	{
		if (s[i] != '\'' && s[i] != '"' && s[i] != '\\' && n < 63)
			p->delim[n++] = s[i];
		i++;
	}
	p->delim[n] = '\0';
	p->pending = n > 0;
	return (i);
}

static int	skip_body(const char *s, int i, t_pscan *p)
{
	int	start;
	int	n;

	p->pending = 0;
	while (s[i])
	{
		start = i;
		while (p->strip && s[i] == '\t')
			i++;
		n = ft_strlen(p->delim);
		if (ft_strncmp(s + i, p->delim, n) == 0
			&& (s[i + n] == '\n' || !s[i + n]))
			return (i + n);
		i = start;
		while (s[i] && s[i] != '\n')
			i++;
		i += s[i] == '\n';
	}
	return (i);
}

static int	paren_step(const char *s, int i, t_pscan *p)
{
	if (s[i] == '#' && (i == 0 || ft_isspace(s[i - 1]) || s[i - 1] == '('))
		while (s[i] && s[i] != '\n')
			i++;
	else if (s[i] == '\n' && p->pending)
		return (skip_body(s, i + 1, p));
	else if (s[i] == '<' && s[i + 1] == '<' && s[i + 2] != '<')
		return (read_delim(s, i, p));
	else if (word_at(s, i, "case") && p->cases < 8)
		p->bases[p->cases++] = p->depth, i += 4;
	else if (word_at(s, i, "esac") && p->cases > 0)
		p->cases--, i += 4;
	else if (s[i] == '(')
		i++, p->depth++;
	else if (s[i] == ')' && p->cases > 0 && p->depth == p->bases[p->cases - 1])
		i++;
	else if (s[i] == ')')
		i++, p->depth--;
	else
		i = skip_nested(s, i);
	return (i);
}

int	skip_dollar_paren(const char *s, int i)
{
	t_pscan	p;

	ft_bzero(&p, sizeof(p));
	p.depth = 1;
	i += 2;
	while (s[i])
	{
		if (s[i] == '\\' && s[i + 1])
			i += 2;
		else
			i = paren_step(s, i, &p);
		if (i == -1)
			return (-1);
		if (p.depth == 0)
			return (i);
	}
	return (-1);
}

int	skip_quote(const char *s, int i)
{
	if (s[i] == '\'')
		return (skip_single(s, i));
	return (skip_double(s, i));
}

int	skip_brace(const char *s, int i)
{
	int	depth;

	depth = 1;
	i += 2;
	while (s[i])
	{
		if (s[i] == '\\' && s[i + 1])
			i += 2;
		else if (s[i] == '$' && s[i + 1] == '{')
			i += 2, depth++;
		else if (s[i] == '}' && --depth == 0)
			return (i + 1);
		else if (s[i] == '}')
			i++;
		else
			i = skip_nested(s, i);
		if (i == -1)
			return (-1);
	}
	return (-1);
}
