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
#include <sys/stat.h>

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

	if (!cmd || !*cmd || !shell)
		return (NULL);

	path_env = NULL;
	if (shell->env)
		path_env = env_get(shell->env, "PATH");

	if (!path_env || *path_env == '\0')
		path_env = "";

	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);

	full_path = search_in_path(cmd, paths);
	free_str_array(paths);
	if (!full_path && ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == 0)
			full_path = ft_strdup(cmd);
	}
	return (full_path);
}

static int	is_exec_file(const char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISREG(st.st_mode)
		&& access(path, X_OK) == 0);
}

char	*find_executable(const char *name, t_shell *shell)
{
	char	**dirs;
	char	*dir;
	char	*full;
	int		i;

	if (!name || !*name)
		return (NULL);
	if (ft_strchr(name, '/'))
		return (is_exec_file(name) ? ft_strdup(name) : NULL);
	dir = env_get(shell->env, "PATH");
	dirs = ft_split(dir ? dir : "", ':');
	i = 0;
	while (dirs && dirs[i])
	{
		dir = ft_strjoin(dirs[i++], "/");
		full = ft_strjoin(dir, name);
		free(dir);
		if (full && is_exec_file(full))
			return (free_str_array(dirs), full);
		free(full);
	}
	free_str_array(dirs);
	return (NULL);
}
