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
#include "../../includes/ctl.h"
#include "../../mylib/include/mylib.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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
	if (len == 5 && ft_strncmp(p, "graph", 5) == 0)
		return (ft_isprint(c) && c != ' ');
	if (len == 5 && ft_strncmp(p, "print", 5) == 0)
		return (ft_isprint(c));
	if (len == 5 && ft_strncmp(p, "blank", 5) == 0)
		return (c == ' ' || c == '\t');
	if (len == 5 && ft_strncmp(p, "cntrl", 5) == 0)
		return (c < 32 || c == 127);
	return (0);
}

static int	elem(const char *p, int *len)
{
	if (p[0] == '[' && (p[1] == '.' || p[1] == '=') && p[2] && p[3] == p[1]
		&& p[4] == ']')
	{
		*len = 5;
		return ((unsigned char)p[2]);
	}
	*len = 1;
	return ((unsigned char)p[0]);
}

static const char	*class_close(const char *p)
{
	const char	*q;
	int			len;

	q = p + 1;
	if (*q == '!' || *q == '^')
		q++;
	if (*q == ']')
		q++;
	while (*q && *q != ']')
	{
		if (*q == '\\' && q[1])
			q++;
		elem(q, &len);
		q = posix_end(q) ? posix_end(q) : q + len;
	}
	return (*q == ']' ? q : NULL);
}

static int	in_class(const char *p, char c)
{
	int			negate;
	int			hit;
	const char	*end;
	int			lo;
	int			l1;
	int			l2 = 1;

	p++;
	negate = (*p == '!' || *p == '^');
	p += negate;
	hit = 0;
	if (*p == ']')
		hit = (c == *p++);
	while (*p && *p != ']')
	{
		if (*p == '\\' && p[1])
			p++;
		end = posix_end(p);
		lo = elem(p, &l1);
		if (end)
			hit |= posix_match(p, end, c);
		if (end)
			p = end;
		else if (p[l1] == '-' && p[l1 + 1] && p[l1 + 1] != ']')
		{
			l2 = 1;
			hit |= ((unsigned char)c >= lo
				&& (unsigned char)c <= elem(p + l1 + 1, &l2));
			p += l1 + 1 + l2;
		}
		else
		{
			hit |= ((unsigned char)c == lo);
			p += l1;
		}
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
	if (entry->d_name[0] == '.' && base[0] != '.'
		&& !(g_shopts & SHOPT_DOTGLOB))
		return (0);
	return (match_pattern(base, entry->d_name));
}

typedef struct s_glob
{
	char	**out;
	int		count;
	char	**segs;
	int		nsegs;
}	t_glob;

static int	has_magic(const char *seg)
{
	int	i;

	i = 0;
	while (seg[i])
	{
		if (seg[i] == '\\' && seg[i + 1])
			i++;
		else if (seg[i] == '*' || seg[i] == '?'
			|| (seg[i] == '[' && class_close(seg + i)))
			return (1);
		i++;
	}
	return (0);
}

static int	is_dir_path(const char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static char	*join_path(const char *base, const char *name)
{
	char	*tmp;
	char	*res;

	if (!*base)
		return (ft_strdup(name));
	if (base[ft_strlen(base) - 1] == '/')
		return (ft_strjoin(base, name));
	tmp = ft_strjoin(base, "/");
	res = ft_strjoin(tmp, name);
	return (free(tmp), res);
}

static void	walk(t_glob *g, const char *base, int idx);

static void	walk_entry(t_glob *g, const char *base, int idx, const char *name)
{
	char	*path;
	char	*slashed;

	path = join_path(base, name);
	if (idx == g->nsegs - 1)
	{
		if (g->count < MAX_MATCHES)
			g->out[g->count++] = path;
		else
			free(path);
		return ;
	}
	if (is_dir_path(path))
	{
		slashed = ft_strjoin(path, "/");
		walk(g, slashed, idx + 1);
		free(slashed);
	}
	free(path);
}

static void	walk_dir(t_glob *g, const char *base, int idx)
{
	DIR				*dir;
	struct dirent	*entry;

	dir = opendir(*base ? base : ".");
	if (!dir)
		return ;
	entry = readdir(dir);
	while (entry && g->count < MAX_MATCHES)
	{
		if (is_match(g->segs[idx], entry))
			walk_entry(g, base, idx, entry->d_name);
		entry = readdir(dir);
	}
	closedir(dir);
}

static void	walk(t_glob *g, const char *base, int idx)
{
	char	*name;
	char	*path;

	if (idx >= g->nsegs)
		return ;
	if (!g->segs[idx][0])
	{
		if (idx == g->nsegs - 1 && is_dir_path(*base ? base : "."))
			g->out[g->count++] = ft_strdup(base);
		return ;
	}
	if (has_magic(g->segs[idx]))
		return (walk_dir(g, base, idx));
	name = unescape(g->segs[idx], ft_strlen(g->segs[idx]));
	path = join_path(base, name);
	if (idx == g->nsegs - 1)
	{
		if (access(path, F_OK) == 0 && g->count < MAX_MATCHES)
			g->out[g->count++] = ft_strdup(path);
	}
	else
		walk_entry(g, base, idx, name);
	free(name);
	free(path);
}

static char	**split_segments(const char *pattern, int *n)
{
	char	**segs;
	int		i;
	int		start;

	*n = 1;
	i = -1;
	while (pattern[++i])
		*n += pattern[i] == '/';
	segs = ft_calloc(*n + 1, sizeof(char *));
	i = 0;
	start = 0;
	*n = 0;
	while (segs)
	{
		if (!pattern[i] || pattern[i] == '/')
		{
			segs[(*n)++] = ft_substr(pattern, start, i - start);
			start = i + 1;
		}
		if (!pattern[i++])
			break ;
	}
	return (segs);
}

int	expand_glob(char *pattern, t_token **head, t_token **cur)
{
	t_glob	g;
	int		i;

	g.out = ft_calloc(MAX_MATCHES + 1, sizeof(char *));
	g.count = 0;
	g.segs = split_segments(pattern, &g.nsegs);
	if (g.out && g.segs)
	{
		if (pattern[0] == '/')
			walk(&g, "/", 1);
		else
			walk(&g, "", 0);
		sort_names(g.out, g.count);
	}
	i = 0;
	while (g.out && i < g.count)
		token_add_back(head, cur, TOKEN_WORD, g.out[i++]);
	free(g.out);
	free_str_array(g.segs);
	return (g.count);
}

int	glob_match(const char *pattern, const char *name)
{
	return (match_pattern(pattern, name));
}
