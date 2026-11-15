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
#include <fnmatch.h>

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

static int	is_match(const char *base, struct dirent *entry)
{
	if (ft_strcmp(entry->d_name, ".") == 0
		|| ft_strcmp(entry->d_name, "..") == 0)
		return (0);
	if (entry->d_name[0] == '.' && base[0] != '.')
		return (0);
	return (fnmatch(base, entry->d_name, 0) == 0);
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
