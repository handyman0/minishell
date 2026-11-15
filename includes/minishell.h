/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:12:21 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:08:41 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/include/libft.h"
# include <unistd.h>
# include <stdlib.h>
# include <signal.h>
# include <termios.h>

// Declarações forward para quebrar dependências circulares
typedef struct s_env t_env;
typedef struct s_token t_token;
typedef struct s_node t_node;
typedef struct s_shell t_shell;
typedef struct s_job t_job;

// Incluir outros headers depois das declarações forward
# include "env.h"
# include "parser.h"
# include "executor.h"
# include "signals.h"
# include "utils.h"
# include "completion.h"
# include "jobs.h"

/**
 * @struct s_shell
 * @brief Estado global da minishell.
 *
 * A estrutura mantém o ambiente, o status de saída do último comando
 * e se o loop principal da shell deve continuar executando.
 */
struct s_shell
{
	t_env		*env;
	int			exit_status;
	int			running;
	t_node		*ast;
	t_token		*tokens;
	t_env		*aliases;
	t_job		*jobs;
	int			next_job_id;
	int			interactive;
	int			in_child;
	pid_t		pgid;
	struct termios	tmodes;
};

void	run_line(char *line, t_shell *shell);

#endif
