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
#include "../includes/output.h"

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

static void	put_quoted(const char *value)
{
	int	i;

	ft_putchar_fd('\'', STDOUT_FILENO);
	i = 0;
	while (value[i])
	{
		if (value[i] == '\'')
			ft_putstr_fd("'\\''", STDOUT_FILENO);
		else
			ft_putchar_fd(value[i], STDOUT_FILENO);
		i++;
	}
	ft_putchar_fd('\'', STDOUT_FILENO);
}

static t_env	*next_exported(t_env *env, const char *after)
{
	t_env	*best;

	best = NULL;
	while (env)
	{
		if (env->exported && (!after || ft_strcmp(env->key, after) > 0)
			&& (!best || ft_strcmp(env->key, best->key) < 0))
			best = env;
		env = env->next;
	}
	return (best);
}

static void	print_exports(t_shell *shell)
{
	t_env	*cur;

	cur = next_exported(shell->env, NULL);
	while (cur)
	{
		ml_put_all(STDOUT_FILENO, "export ", cur->key, NULL);
		if (cur->value)
		{
			ft_putchar_fd('=', STDOUT_FILENO);
			put_quoted(cur->value);
		}
		ft_putchar_fd('\n', STDOUT_FILENO);
		cur = next_exported(shell->env, cur->key);
	}
}

int	builtin_export(char **argv, t_shell *shell)
{
	int i = 1;
	int status = 0;
	char *equal_sign;

	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	else if (argv[i] && ft_strcmp(argv[i], "-p") == 0)
		i++;
	if (!argv[i])
		return (print_exports(shell), 0);
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
				*equal_sign = '\0';
			if (env_is_readonly(shell->env, argv[i]))
			{
				ml_put_all(2, "minishell: ", argv[i],
					": readonly variable\n", NULL);
				shell->running = shell->spare;
				status = 1;
			}
			else if (equal_sign)
			{
				env_set(&shell->env, argv[i], equal_sign + 1);
				env_export(shell->env, argv[i]);
			}
			else if (!env_export(shell->env, argv[i]))
				env_set(&shell->env, argv[i], "");
			if (equal_sign)
				*equal_sign = '=';
		}
		i++;
	}
	return (status);
}
