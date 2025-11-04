/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:38:40 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:39:29 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include "../includes/minishell.h"

int	builtin_exit(char **argv, t_shell *shell)
{
	(void)argv;
	shell->running = 0;
	printf("exit\n");
	return (0);
}
