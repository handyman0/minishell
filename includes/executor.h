/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:02:41 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 20:13:31 by lmelo-do         ###   ########.fr       */
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
# include "minishell.h"

void	error_exit(const char *msg);
void	free_str_array(char **arr);

void	exec_heredoc(const char *limiter);

int		open_file(char *path, int mode);
void	handle_redirection_in(const char *file);
void	handle_redirection_out(const char *file, int append);

char	*find_path(char *cmd, t_shell *shell);
char	**env_to_array(t_env *env);
int		execute_tree(t_node *node, t_shell *shell);

#endif
