/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 15:00:17 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 19:41:18 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/pipex_bonus.h"

static void	here_doc_child(int *fd, const char *limiter)
{
	char	*line;

	close(fd[0]);
	write(1, "heredoc> ", 9);
	while (get_next_line(&line) > 0)
	{
		if (ft_strncmp(line, limiter, ft_strlen(limiter)) == 0
			&& line[ft_strlen(limiter)] == '\n')
		{
			free(line);
			exit(EXIT_SUCCESS);
		}
		write(fd[1], line, ft_strlen(line));
		free(line);
		write(1, "heredoc> ", 9);
	}
}

void	here_doc(const char *limiter, int argc)
{
	pid_t	reader;
	int		fd[2];

	if (argc < 6)
		usage();
	if (pipe(fd) == -1)
		error();
	reader = fork();
	if (reader == -1)
		error();
	if (reader == 0)
		here_doc_child(fd, limiter);
	else
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		wait(NULL);
	}
}

void	usage(void)
{
	ft_putstr_fd("\033[31mError: Bad argument\n\e[0m", 2);
	ft_putstr_fd("Ex: ./pipex <file1> <cmd1> <cmd2> <...> <file2>\n", 1);
	ft_putstr_fd("\t./pipex \"here_doc\" <LIMITER> ", 1);
	ft_putstr_fd("<cmd> <cmd1> <...> <file>\n", 1);
	exit(EXIT_SUCCESS);
}

int	open_file(char *argv, int i)
{
	int	file;

	file = 0;
	if (i == 0)
		file = open(argv, O_WRONLY | O_CREAT | O_APPEND, 0777);
	else if (i == 1)
		file = open(argv, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	else if (i == 2)
		file = open(argv, O_RDONLY, 0777);
	if (file == -1)
		error();
	return (file);
}
