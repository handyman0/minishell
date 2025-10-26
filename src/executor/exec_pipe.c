/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:27:39 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:36:11 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"

void	exec_pipe(char **cmds, char **envp)
{
	int		fd[2];
	pid_t	pid;
	int		i;

	i = 0;
	while (cmds[i + 1])
	{
		if (pipe(fd) == -1)
			error_exit("pipe");
		pid = fork();
		if (pid == -1)
			error_exit("fork");
		if (pid == 0)
		{
			close(fd[0]);
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
			exec_command(cmds[i], envp);
		}
		else
		{
			close(fd[1]);
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
			waitpid(pid, NULL, 0);
			i++;
		}
	}
	exec_command(cmds[i], envp);
}
