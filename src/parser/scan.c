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

int	skip_dollar_paren(const char *s, int i)
{
	int	depth;

	depth = 1;
	i += 2;
	while (s[i])
	{
		if (s[i] == '\\' && s[i + 1])
			i += 2;
		else if (s[i] == '(')
			i++, depth++;
		else if (s[i] == ')' && --depth == 0)
			return (i + 1);
		else if (s[i] == ')')
			i++;
		else
			i = skip_nested(s, i);
		if (i == -1)
			return (-1);
	}
	return (-1);
}

int	skip_quote(const char *s, int i)
{
	if (s[i] == '\'')
		return (skip_single(s, i));
	return (skip_double(s, i));
}
