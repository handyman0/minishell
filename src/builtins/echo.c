/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:18:40 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 18:07:38 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include <fcntl.h>
#include "../includes/output.h"

static int	is_n_flag(char *arg)
{
	int	i;

	if (!arg || arg[0] != '-' || arg[1] != 'n')
		return (0);
	i = 1;
	while (arg[i] == 'n')
		i++;
	return (arg[i] == '\0');
}

int	stdout_failed(const char *name)
{
	if (fcntl(STDOUT_FILENO, F_GETFD) != -1)
		return (0);
	ml_put_all(2, "minishell: ", name, ": write error: Bad file descriptor\n",
		NULL);
	return (1);
}

int	builtin_echo(char **argv)
{
	int	i;
	int	newline;

	if (stdout_failed("echo"))
		return (1);

	i = 1;
	newline = 1;
	while (is_n_flag(argv[i]))
	{
		newline = 0;
		i++;
	}
	while (argv[i])
	{
		ft_putstr_fd(argv[i], STDOUT_FILENO);
		if (argv[i + 1])
			ft_putchar_fd(' ', STDOUT_FILENO);
		i++;
	}
	if (newline)
		ft_putchar_fd('\n', STDOUT_FILENO);
	return (0);
}
