/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 18:08:58 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 19:16:31 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex.h"

void	child_process(char **argv, char **envp, int *fd)
{
	int		filein;

	filein = open(argv[1], O_RDONLY);
	if (filein == -1)
		error();
	dup2(filein, STDIN_FILENO);
	dup2(fd[1], STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(filein);
	execute(argv[2], envp);
}

void	parent_process(char **argv, char **envp, int *fd)
{
	int		fileout;

	fileout = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fileout == -1)
		error();
	dup2(fd[0], STDIN_FILENO);
	dup2(fileout, STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(fileout);
	execute(argv[3], envp);
}

static void	init_complex(int argc, char **argv, int *fd, int *files)
{
	if (argc != 5)
	{
		ft_putstr_fd("\033[Error: Bad arguments\n\e[0m", 2);
		ft_putstr_fd("Ex: ./pipex <file1> <cmd1> <cmd2> <file2>\n", 1);
		exit(1);
	}
	if (pipe(fd) == -1)
		error();
	files[0] = open(argv[1], O_RDONLY, 0777);
	files[1] = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (files[0] || files[1] == -1)
		error();
}

int	main(int argc, char **argv, char **envp)
{
	int		fd[2];
	int		files[2];
	pid_t	pid1;

	init_complex(argc, argv, fd, files);
	pid1 = fork();
	if (pid1 == -1)
		error();
	if (pid1 == 0)
		child_process(argv, envp, fd);
	waitpid(pid1, NULL, 0);
	parent_process(argv, envp, fd);
	return (0);
}
