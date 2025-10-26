/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:02:41 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:34:04 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include <unistd.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <stdio.h>
# include "../libft/include/libft.h"
# include "utils.h"
# include "parser.h"

void	error_exit(const char *msg);
void	free_str_array(char **arr);

char	*find_path(const char *cmd, char **envp);
void	exec_command(char *cmd, char **envp);

void	exec_pipe(char **cmds, char **envp);
void	exec_heredoc(const char *limiter);

int		open_file(char *path, int mode);
void	handle_redirection_in(const char *file);
void	handle_redirection_out(const char *file, int append);

#endif

