/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:24:56 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 17:45:05 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include "../../includes/minishell.h"
#include "../../includes/env.h"

static char	*search_in_path(char *cmd, char **paths)
{
	char	*full_path;
	char	*temp;
	int		i;

	i = 0;
	while (paths && paths[i])
	{
		temp = ft_strjoin(paths[i], "/");
		full_path = ft_strjoin(temp, cmd);
		free(temp);
		if (access(full_path, X_OK) == 0)
			return (full_path);
		free(full_path);
		i++;
	}
	return (NULL);
}

char	*find_path(char *cmd, t_shell *shell)
{
	char	*path_env;
	char	**paths;
	char	*full_path;

	if (!shell)
	{
		printf("find_path: shell é NULL\n");
		return (NULL);
	}
	if (!shell->env)
	{
		printf("find_path: shell->env é NULL, criando PATH mínimo\n");
		path_env = "/bin:/usr/bin:/usr/local/bin";
	}
	else
	{
		path_env = env_get(shell->env, "PATH");
	}

	if (!path_env)
	{
		printf("find_path: PATH não encontrado\n");
		return (NULL);
	}
	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);

	full_path = search_in_path(cmd, paths);
	free_str_array(paths);
	return (full_path);
}
