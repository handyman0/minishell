#include "../../includes/parser.h"

char	*param_substring(const char *value, long off, long len, int has_len)
{
	long	size;

	size = (long)ft_strlen(value);
	if (off < 0)
		off += size;
	if (off < 0 || off > size)
		return (ft_strdup(""));
	if (!has_len)
		return (ft_strdup(value + off));
	if (len < 0)
		len += size - off;
	if (len < 0)
		return (NULL);
	if (len > size - off)
		len = size - off;
	return (ft_substr(value, off, len));
}

static int	match_at(const char *pat, const char *value, int start, int *end)
{
	char	*part;
	int		j;
	int		len;

	len = (int)ft_strlen(value);
	j = len;
	while (j >= start)
	{
		part = ft_substr(value, start, j - start);
		if (part && glob_match(pat, part))
			return (free(part), *end = j, 1);
		free(part);
		j--;
	}
	return (0);
}

static char	*append(char *dst, const char *src, int n)
{
	char	*piece;
	char	*res;

	piece = ft_substr(src, 0, n);
	res = ft_strjoin(dst, piece);
	free(piece);
	free(dst);
	return (res);
}

static int	next_match(const char *pat, const char *value, int i, int mode)
{
	int	end;

	if ((mode == 2 && i > 0) || !match_at(pat, value, i, &end) || end <= i)
		return (-1);
	if (mode == 3 && value[end])
		return (-1);
	return (end);
}

char	*param_replace(const char *value, const char *pat, const char *rep,
			int mode)
{
	char	*out;
	int		i;
	int		end;
	int		len;

	out = ft_strdup("");
	len = (int)ft_strlen(value);
	i = 0;
	while (out && i < len && *pat)
	{
		end = next_match(pat, value, i, mode);
		if (end > 0)
		{
			out = append(out, rep, ft_strlen(rep));
			i = end;
			if (mode != 1)
				break ;
		}
		else
			out = append(out, value + i++, 1);
	}
	return (out ? append(out, value + i, len - i) : NULL);
}

char	*param_case(const char *value, const char *pat, int upper, int all)
{
	char	*out;
	char	one[2];
	int		i;

	out = ft_strdup(value);
	i = 0;
	one[1] = '\0';
	while (out && out[i])
	{
		one[0] = out[i];
		if (!*pat || glob_match(pat, one))
			out[i] = upper ? ft_toupper(out[i]) : ft_tolower(out[i]);
		if (!all)
			break ;
		i++;
	}
	return (out);
}

char	*param_quote(const char *value)
{
	char	*out;
	int		i;

	out = ft_strdup("'");
	i = 0;
	while (out && value[i])
	{
		if (value[i] == '\'')
			out = append(out, "'\\''", 4);
		else
			out = append(out, value + i, 1);
		i++;
	}
	return (out ? append(out, "'", 1) : NULL);
}
