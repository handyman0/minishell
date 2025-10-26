/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:30:07 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:37:04 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"

int	open_file(char *path, int mode)
{
	int	fd;

	if (mode == 0) // append
		fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else if (mode == 1) // truncate
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else // read
		fd = open(path, O_RDONLY, 0644);
	if (fd == -1)
		error_exit("open");
	return (fd);
}

void	handle_redirection_in(const char *file)
{
	int	fd;

	fd = open_file((char *)file, 2);
	dup2(fd, STDIN_FILENO);
	close(fd);
}

void	handle_redirection_out(const char *file, int append)
{
	int	fd;

	if (append)
		fd = open_file((char *)file, 0);
	else
		fd = open_file((char *)file, 1);
	dup2(fd, STDOUT_FILENO);
	close(fd);
}
