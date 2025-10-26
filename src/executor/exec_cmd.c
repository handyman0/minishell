/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:29:11 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:35:52 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"

void	exec_command(char *cmd, char **envp)
{
	char	**args;
	char	*path;

	args = ft_split(cmd, ' ');
	if (!args || !args[0])
		error_exit("invalid command");
	path = find_path(args[0], envp);
	if (!path)
	{
		ft_putstr_fd("minishell: command not found: ", 2);
		ft_putendl_fd(args[0], 2);
		free_str_array(args);
		exit(127);
	}
	if (execve(path, args, envp) == -1)
	{
		free(path);
		free_str_array(args);
		error_exit("execve");
	}
}

