/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:34:24 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 14:48:25 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include <unistd.h>

int	builtin_cd(char **argv, t_shell *shell)
{
	char	*path;
	char	*oldpwd;
	char	cwd[1024];

	if (argv[1] && argv[2])
	{
		ft_putstr_fd("minishell: cd: número excessivo de argumentos\n", STDERR_FILENO);
		return (1);
	}
	oldpwd = getcwd(NULL, 0);
	if (!oldpwd)
	{
		perror("minishell: cd");
		return (1);
	}
	if (!argv[1] || ft_strcmp(argv[1], "~") == 0)
	{
		path = env_get(shell->env, "HOME");
		if (!path)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			free(oldpwd);
			return (1);
		}
	}
	else if (ft_strcmp(argv[1], "-") == 0)
	{
		path = env_get(shell->env, "OLDPWD");
		if (!path)
		{
			ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
			free(oldpwd);
			return (1);
		}
		ft_putstr_fd(path, STDOUT_FILENO);
		ft_putchar_fd('\n', STDOUT_FILENO);
	}
	else
		path = argv[1];
	if (chdir(path) != 0)
	{
		ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
		ft_putstr_fd(path, STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
		perror("");
		free(oldpwd);
		return (1);
	}
	// Atualiza OLDPWD com o diretorio anterior
	env_set(&shell->env, "OLDPWD", oldpwd);
	free(oldpwd);

	// Atualizar PWD com o novo diretorio atual
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		env_set(&shell->env, "PWD", cwd);
	else
	{
		perror("minishell: cd");
		return (1);
	}
	return (0);
}
