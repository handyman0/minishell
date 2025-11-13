/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 19:43:49 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 15:59:45 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/env.h"
#include "../../includes/utils.h"

t_env	*env_new(char *key, char *value)
{
	t_env	*node;

	node = malloc(sizeof(t_env));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	if (value)
		node->value = ft_strdup(value);
	else
		node->value = ft_strdup("");
	node->next = NULL;
	return (node);
}

void	env_add_back(t_env **env, t_env *new)
{
	t_env	*tmp;

	if (!new)
		return;
	if (!*env)
	{
		*env = new;
		return;
	}
	tmp = *env;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new;
}

t_env	*env_init(char **envp)
{
	t_env	*env;
	t_env	*node;
	char	*equal_sign;
	int		i;
	char	cwd[1024];

	env = NULL;

	if (envp)
	{
		i = 0;
		while (envp[i])
		{
			equal_sign = ft_strchr(envp[i], '=');
			if (equal_sign)
			{
				*equal_sign = '\0';
				node = env_new(envp[i], equal_sign + 1);
				*equal_sign = '=';
				if (node)
					env_add_back(&env, node);
			}
			i++;
		}
	}

	// Garantir que PWD está definido e correto
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		// Se PWD já existe, atualizar; senão, criar
		char	*existing_pwd = env_get(env, "PWD");
		if (existing_pwd)
			env_set(&env, "PWD", cwd);
		else
		{
			node = env_new("PWD", cwd);
			if (node)
				env_add_back(&env, node);
		}
	}
	// Se não há ambiente, criar mínimo
	if (!envp)
		return (env_init_minimal());
	return (env);
}

void	env_free(t_env *env)
{
	t_env	*tmp;

	while (env)
	{
		tmp = env->next;
		free(env->key);
		free(env->value);
		free(env);
		env = tmp;
	}
}
