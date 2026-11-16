/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:37:12 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 19:03:32 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include "../includes/output.h"

static int	is_valid_identifier(char *str)
{
	int i = 0;

	if (!str || !*str || ft_isdigit(str[0]))
		return (0);
	while (str[i])
	{
		if(!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

static int	unset_one(char *name, int functions, t_shell *shell)
{
	if (functions)
		return (func_remove(shell, name), 0);
	if (!is_valid_identifier(name))
		return (ml_put_all(2, "minishell: unset: '", name,
				"': not a valid identifier\n", NULL), 1);
	if (env_is_readonly(shell->env, name))
	{
		ml_put_all(2, "minishell: unset: ", name,
			": cannot unset: readonly variable\n", NULL);
		shell->running = shell->spare;
		return (1);
	}
	env_unset(&shell->env, name);
	return (0);
}

int	builtin_unset(char **argv, t_shell *shell)
{
	int	i;
	int	status;
	int	functions;

	i = 1;
	functions = 0;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0 && ++i)
			break ;
		if (ft_strcmp(argv[i], "-f") == 0)
			functions = 1;
		else if (ft_strcmp(argv[i], "-v") == 0)
			functions = 0;
		else
			return (ml_put_all(2, "minishell: unset: ", argv[i],
					": invalid option\n", NULL), 2);
		i++;
	}
	status = 0;
	while (argv[i])
		status |= unset_one(argv[i++], functions, shell);
	return (status);
}
