/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 16:30:09 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:41:36 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTINS_H
#define BUILTINS_H

# include "../libft/include/libft.h"
# include "minishell.h"

int builtin_pwd(void);
int	builtin_echo(char **argv);
int	builtin_cd(char **argv, t_shell *shell);
int	builtin_export(char **argv, t_shell *shell);
int	builtin_unset(char **argv, t_shell *shell);
int	builtin_env(t_env *env);
int	builtin_exit(char **argv, t_shell *shell);

#endif
