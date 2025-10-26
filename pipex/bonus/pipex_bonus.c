/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 17:30:19 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 19:39:12 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex_bonus.h"

void	child_process_bonus(char *argv, char **envp)
{
	pid_t	pid;
	int		fd[2];

	if (pipe(fd) == -1)
		error();
	pid = fork();
	if (pid == -1)
		error();
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execute(argv, envp);
	}
	else
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		waitpid(pid, NULL, 0);
	}
}

static void	process_here_doc(int argc, char **argv, int *fileout)
{
	*fileout = open_file(argv[argc - 1], 0);
	here_doc(argv[2], argc);
}

static void	proccess_normal(int argc, char **argv, int *filein, int *fileout)
{
	*fileout = open_file(argv[argc - 1], 1);
	*filein = open_file(argv[1], 2);
	dup2(*filein, STDIN_FILENO);
}

int	main(int argc, char **argv, char **envp)
{
	int	i;
	int	filein;
	int	fileout;

	if (argc >= 5)
	{
		if (ft_strncmp(argv[1], "here_doc", 8) == 0)
		{
			i = 3;
			process_here_doc(argc, argv, &fileout);
		}
		else
		{
			i = 2;
			proccess_normal(argc, argv, &filein, &fileout);
		}
		while (i < argc - 2)
			child_process_bonus(argv[i++], envp);
		dup2(fileout, STDOUT_FILENO);
		execute(argv[argc - 2], envp);
	}
	usage();
	return (0);
}
