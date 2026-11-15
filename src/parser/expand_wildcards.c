/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_wildcards.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 15:02:20 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/05 17:39:38 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../libft/include/libft.h"
#include <dirent.h>

#define MAX_MATCHES 4096

static char	*unescape(const char *str, size_t len)
{
	char	*out;
	size_t	i;
	size_t	j;

	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	j = 0;
	while (i < len)
	{
		if (str[i] == '\\' && i + 1 < len)
			i++;
		out[j++] = str[i++];
	}
	out[j] = '\0';
	return (out);
}

static void	sort_names(char **names, int count)
{
	char	*tmp;
	int		i;
	int		j;

	i = 1;
	while (i < count)
	{
		tmp = names[i];
		j = i - 1;
		while (j >= 0 && ft_strcmp(names[j], tmp) > 0)
		{
			names[j + 1] = names[j];
			j--;
		}
		names[j + 1] = tmp;
		i++;
	}
}

static const char	*posix_end(const char *p)
{
	const char	*q;

	if (p[0] != '[' || p[1] != ':')
		return (NULL);
	q = p + 2;
	while (ft_isalpha(*q))
		q++;
	if (q > p + 2 && q[0] == ':' && q[1] == ']')
		return (q + 2);
	return (NULL);
}

static int	posix_match(const char *p, const char *end, char c)
{
	size_t	len;

	p += 2;
	len = end - 2 - p;
	if (len == 5 && ft_strncmp(p, "alpha", 5) == 0)
		return (ft_isalpha(c));
	if (len == 5 && ft_strncmp(p, "digit", 5) == 0)
		return (ft_isdigit(c));
	if (len == 5 && ft_strncmp(p, "alnum", 5) == 0)
		return (ft_isalnum(c));
	if (len == 5 && ft_strncmp(p, "upper", 5) == 0)
		return (c >= 'A' && c <= 'Z');
	if (len == 5 && ft_strncmp(p, "lower", 5) == 0)
		return (c >= 'a' && c <= 'z');
	if (len == 5 && ft_strncmp(p, "space", 5) == 0)
		return (ft_isspace(c));
	if (len == 6 && ft_strncmp(p, "xdigit", 6) == 0)
		return (ft_isdigit(c) || (c >= 'a' && c <= 'f')
			|| (c >= 'A' && c <= 'F'));
	if (len == 5 && ft_strncmp(p, "punct", 5) == 0)
		return (ft_isprint(c) && !ft_isalnum(c) && c != ' ');
	return (0);
}

static const char	*class_close(const char *p)
{
	const char	*q;

	q = p + 1;
	if (*q == '!' || *q == '^')
		q++;
	if (*q == ']')
		q++;
	while (*q && *q != ']')
		q = posix_end(q) ? posix_end(q) : q + 1;
	return (*q == ']' ? q : NULL);
}

static int	in_class(const char *p, char c)
{
	int			negate;
	int			hit;
	const char	*end;

	p++;
	negate = (*p == '!' || *p == '^');
	p += negate;
	hit = 0;
	if (*p == ']')
		hit = (c == *p++);
	while (*p && *p != ']')
	{
		end = posix_end(p);
		if (end)
			hit |= posix_match(p, end, c);
		if (end)
			p = end;
		else if (p[1] == '-' && p[2] && p[2] != ']')
		{
			hit |= (c >= p[0] && c <= p[2]);
			p += 3;
		}
		else
			hit |= (c == *p++);
	}
	return (hit != negate);
}

static const char	*next_pattern(const char *p)
{
	if (*p == '[' && class_close(p))
		return (class_close(p) + 1);
	if (*p == '\\' && p[1])
		return (p + 2);
	return (p + 1);
}

static int	match_one(const char *p, char c)
{
	if (*p == '?')
		return (1);
	if (*p == '[' && class_close(p))
		return (in_class(p, c));
	if (*p == '\\' && p[1])
		p++;
	return (*p == c);
}

static int	match_pattern(const char *p, const char *s)
{
	const char	*star;
	const char	*mark;

	star = NULL;
	mark = s;
	while (*s)
	{
		if (*p == '*')
		{
			star = p++;
			mark = s;
		}
		else if (*p && match_one(p, *s))
		{
			p = next_pattern(p);
			s++;
		}
		else if (star)
		{
			p = star + 1;
			s = ++mark;
		}
		else
			return (0);
	}
	while (*p == '*')
		p++;
	return (*p == '\0');
}

static int	is_match(const char *base, struct dirent *entry)
{
	if (ft_strcmp(entry->d_name, ".") == 0
		|| ft_strcmp(entry->d_name, "..") == 0)
		return (0);
	if (entry->d_name[0] == '.' && base[0] != '.')
		return (0);
	return (match_pattern(base, entry->d_name));
}

static int	collect(const char *dirpath, const char *base, char **names)
{
	DIR				*dir;
	struct dirent	*entry;
	int				count;

	dir = opendir(dirpath);
	if (!dir)
		return (0);
	count = 0;
	entry = readdir(dir);
	while (entry && count < MAX_MATCHES)
	{
		if (is_match(base, entry))
			names[count++] = ft_strdup(entry->d_name);
		entry = readdir(dir);
	}
	closedir(dir);
	return (count);
}

int	expand_glob(char *pattern, t_token **head, t_token **cur)
{
	char	*names[MAX_MATCHES];
	char	*slash;
	char	*prefix;
	int		count;
	int		i;

	slash = ft_strrchr(pattern, '/');
	prefix = ft_strdup("");
	if (slash)
	{
		free(prefix);
		prefix = unescape(pattern, slash - pattern + 1);
	}
	count = 0;
	if (prefix && (!slash || slash[1]))
		count = collect(slash ? prefix : ".", slash ? slash + 1 : pattern,
				names);
	sort_names(names, count);
	i = 0;
	while (i < count)
	{
		token_add_back(head, cur, TOKEN_WORD, ft_strjoin(prefix, names[i]));
		free(names[i++]);
	}
	free(prefix);
	return (count);
}
