/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:36:39 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 14:44:20 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include "../includes/minishell.h"

static int	is_valid_identifier(char *str)
{
	int i = 0;

	// Caso especial: string vazia ou começando com '='
	if (!str || !*str || str[0] == '=')
		return (0);
	// Verifica se é um digito no inicio
	if (ft_isdigit(str[0]))
		return (0);
	// Verifica cada caractere até o '=' ou fim da string
	while (str[i] && str[i] != '=')
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	builtin_export(char **argv, t_shell *shell)
{
	int i = 1;
	int status = 0;
	char *equal_sign;

	if (!argv[1])
	{
		env_print(shell->env);
		return (0);
	}
	while (argv[i])
	{
		if (!is_valid_identifier(argv[i]))
		{
			ft_putstr_fd("minishell: export: `", STDERR_FILENO);
			ft_putstr_fd(argv[i], STDERR_FILENO);
			ft_putstr_fd("': não é um identificador válido\n", STDERR_FILENO);
			status = 1;
		}
		else
		{
			equal_sign = ft_strchr(argv[i], '=');
			if (equal_sign)
			{
				*equal_sign = '\0';
				env_set(&shell->env, argv[i], equal_sign + 1);
				*equal_sign = '=';
			}
			else
				env_set(&shell->env, argv[i], "");
		}
		i++;
	}
	return (status);
}
