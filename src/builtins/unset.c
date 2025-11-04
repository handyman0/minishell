/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:37:12 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:37:32 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include "../includes/minishell.h"

int	builtin_unset(char **argv, t_shell *shell)
{
	(void)argv;
	(void)shell;
	printf("minishell: unset: ainda não implementado");
	return (1);
}
