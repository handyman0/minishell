/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:30:07 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 18:45:56 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"

int	open_file(char *path, int mode)
{
	int	fd;

	if (mode == 0)
		fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else if (mode == 1)
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else
		fd = open(path, O_RDONLY);
	if (fd == -1)
		return (-1);
	return (fd);
}

int	handle_redirection_in(const char *file)
{
	int	fd;

	fd = open_file((char *)file, 2);
	if (fd == -1)
	{
		perror("minishell");
		return (0);
	}
	dup2(fd, STDIN_FILENO);
	close(fd);
	return (1);
}

int	handle_redirection_out(const char *file, int append)
{
	int	fd;

	if (append)
		fd = open_file((char *)file, 0);
	else
		fd = open_file((char *)file, 1);
	if (fd == -1)
	{
		perror("minishell");
		return (0);
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
	return (1);
}
