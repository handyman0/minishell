/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:24:56 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:29:06 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include "../../includes/minishell.h"
#include "../../includes/env.h"

static char	*search_in_path(char *cmd, char **paths)
{
	if (!cmd || cmd[0] == '\0')
		return (NULL);
	char	*full_path;
	char	*temp;
	int		i;

	i = 0;
	while (paths && paths[i])
	{
		temp = ft_strjoin(paths[i], "/");
		full_path = ft_strjoin(temp, cmd);
		free(temp);
		/* if the file exists in this path, return it (execve will handle permission errors) */
		if (access(full_path, F_OK) == 0)
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
	const char *default_path = "/bin:/usr/bin:/usr/local/bin";

	if (!cmd || !*cmd || !shell)
		return (NULL);

	path_env = NULL;
	if (shell->env)
		path_env = env_get(shell->env, "PATH");

	if (!path_env || *path_env == '\0')
		path_env = (char *)default_path;

	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);

	full_path = search_in_path(cmd, paths);
	free_str_array(paths);
	/* if cmd contains a slash, try it directly (relative or absolute path) */
	if (!full_path && ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == 0)
			full_path = ft_strdup(cmd);
	}
	return (full_path);
}
