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

static void	expand_token(t_token **tokens, t_token **current, char *pattern)
{
	DIR				*dir;
	struct dirent	*entry;
	int				count;
	char			*dup_str;

	dir = opendir(".");
	if (!dir)
		return ;
	count = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_name[0] != '.' || pattern[0] == '.')
		{
			if (match_pattern(pattern, entry->d_name))
			{
				dup_str = ft_strdup(entry->d_name);
				token_add_back(tokens, current, TOKEN_WORD, dup_str);
				count++;
			}
		}
	}
	closedir(dir);
	if (count == 0)
	{
		dup_str = ft_strdup(pattern);
		token_add_back(tokens, current, TOKEN_WORD, dup_str);
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
		if (current->type == TOKEN_WORD && ft_strchr(current->value, '*'))
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
