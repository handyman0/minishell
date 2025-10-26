/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 18:32:16 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 19:41:41 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex.h"

static char	*search_in_paths(char **paths, const char *cmd)
{
	int		i;
	char	*part_path;
	char	*path;

	i = 0;
	while (paths[i])
	{
		part_path = ft_strjoin(paths[i], "/");
		path = ft_strjoin(part_path, cmd);
		free(part_path);
		if (access(path, F_OK) == 0)
			return (path);
		free(path);
		i++;
	}
	return (NULL);
}

char	*find_path(const char *cmd, char **envp)
{
	char	**paths;
	char	*path;
	int		i;

	i = 0;
	if (!envp)
		return (NULL);
	while (envp[i] && ft_strnstr(envp[i], "PATH", 4) == 0)
		i++;
	if (!envp[i])
		return (NULL);
	paths = ft_split(envp[i] + 5, ':');
	path = search_in_paths(paths, cmd);
	free_array(paths);
	return (path);
}

void	error(void)
{
	perror("Error");
	exit(EXIT_FAILURE);
}

void	execute(char *argv, char **envp)
{
	char	**cmd;
	char	*path;

	cmd = ft_split(argv, ' ');
	if (!cmd)
		error();
	path = find_path(cmd[0], envp);
	if (!path)
	{
		free_array(cmd);
		ft_putstr_fd("Command not found: ", 2);
		ft_putstr_fd(argv, 2);
		ft_putstr_fd("\n", 2);
		exit(127);
	}
	if (execve(path, cmd, envp) == -1)
	{
		free(path);
		free_array(cmd);
		error();
	}
}

void	free_array(char **array)
{
	int	i;

	if (!array)
		return ;
	i = 0;
	while (array[i])
	{
		free(array[i]);
		i++;
	}
	free(array);
}
