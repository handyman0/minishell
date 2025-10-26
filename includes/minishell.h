/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:12:21 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:42:58 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/include/libft.h"
# include "env.h"
# include "executor.h"
# include "parser.h"
# include "signals.h"
# include "utils.h"

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <signal.h>

typedef struct s_shell
{
	t_env		*env;
	int			exit_status;
	int			running;
}	t_shell;

#endif
