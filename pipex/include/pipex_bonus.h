/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 18:10:01 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 18:36:25 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_BONUS_H
# define PIPEX_BONUS_H

# include "pipex.h"

void	child_process_bonus(char *argv, char **envp);
void	here_doc(char *limiter, int argc);
int		open_file(char *argv, int i);
void	usage(void);

#endif
