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

static int	match_pattern(const char *pattern, const char *filename)
{
	return (fnmatch(pattern, filename, 0) == 0);
}

static int	has_unquoted_star(const char *str)
{
	char	quote;

	quote = 0;
	while (*str)
	{
		if (!quote && (*str == '\'' || *str == '"'))
			quote = *str;
		else if (quote && *str == quote)
			quote = 0;
		else if (!quote && *str == '*')
			return (1);
		str++;
	}
	return (0);
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

static int	collect_matches(char *pattern, char **names, int max)
{
	DIR				*dir;
	struct dirent	*entry;
	int				count;

	dir = opendir(".");
	if (!dir)
		return (0);
	count = 0;
	entry = readdir(dir);
	while (entry && count < max)
	{
		if ((entry->d_name[0] != '.' || pattern[0] == '.')
			&& match_pattern(pattern, entry->d_name))
			names[count++] = ft_strdup(entry->d_name);
		entry = readdir(dir);
	}
	closedir(dir);
	return (count);
}

static void	expand_token(t_token **tokens, t_token **current, char *pattern)
{
	char	*names[4096];
	int		count;
	int		i;

	count = collect_matches(pattern, names, 4096);
	if (count == 0)
	{
		token_add_back(tokens, current, TOKEN_WORD, ft_strdup(pattern));
		return ;
	}
	sort_names(names, count);
	i = 0;
	while (i < count)
	{
		token_add_back(tokens, current, TOKEN_WORD, names[i]);
		i++;
	}
}

void	expand_wildcards(t_token **tokens)
{
	t_token	*new_list;
	t_token	*new_current;
	t_token	*current;
	t_token	*next;

	if(!tokens || !*tokens)
		return ;
	new_list = NULL;
	new_current = NULL;
	current = *tokens;
	while (current)
	{
		next = current->next;
		if (current->type == TOKEN_WORD && has_unquoted_star(current->value))
			expand_token(&new_list, &new_current, current->value);
		else
		{
			char	*dup_value = current->value ? ft_strdup(current->value) : NULL;
			token_add_back(&new_list, &new_current, current->type, dup_value);
		}
		if (current->value)
			free(current->value);
		free(current);
		current = next;
	}
	*tokens = new_list;
}
