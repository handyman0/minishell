/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_heredoc.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:28:16 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:36:42 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include <readline/readline.h>

void	exec_heredoc(const char *limiter)
{
	int		fd[2];
	char	*line;

	if (pipe(fd) == -1)
		error_exit("pipe");
	while (1)
	{
		line = readline("heredoc> ");
		if (!line)
			break ;
		if (ft_strncmp(line, limiter, ft_strlen(limiter)) == 0
			&& line[ft_strlen(limiter)] == '\0')
		{
			free(line);
			break ;
		}
		ft_putendl_fd(line, fd[1]);
		free(line);
	}
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);
	close(fd[0]);
}
