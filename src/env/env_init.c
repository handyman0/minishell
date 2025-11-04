/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 19:43:49 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 16:52:33 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/env.h"
#include "../../includes/utils.h"

static void	free_split(char **split)
{
	int	i;

	if (!split)
		return;
	i = 0;
	while (split[i])
	{
		free(split[i]);
		i++;
	}
	free(split);
}

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
	char	**split;
	int		i;

	env = NULL;

	if (!envp)
	{
		printf("⚠️  envp é NULL - criando ambiente mínimo\n");
		return (env_init_minimal());
	}

	i = 0;
	while (envp[i])
	{
		split = ft_split(envp[i], '=');
		if (split && split[0])
		{
			node = env_new(split[0], split[1] ? split[1] : "");
			if (node)
				env_add_back(&env, node);
		}
		if (split)
			free_split(split);
		i++;
	}

	// Se ainda estiver vazio, cria mínimo
	if (!env)
	{
		printf("⚠️  Ambiente vazio - criando mínimo\n");
		return (env_init_minimal());
	}

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
