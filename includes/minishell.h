/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:12:21 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 20:09:25 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/include/libft.h"
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <signal.h>

// Declarações forward para quebrar dependências circulares
typedef struct s_env t_env;
typedef struct s_token t_token;
typedef struct s_node t_node;
typedef struct s_shell t_shell;

// Incluir outros headers depois das declarações forward
# include "env.h"
# include "parser.h"
# include "executor.h"
# include "signals.h"
# include "utils.h"

struct s_shell
{
	t_env		*env;
	int			exit_status;
	int			running;
};

#endif
