/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 15:55:16 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 15:38:34 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/env.h"
#include "../../includes/output.h"

void	env_print(t_env *env)
{
	while (env)
	{
		if (env->exported)
			ml_put_all(STDOUT_FILENO, env->key, "=", env->value, "\n", NULL);
		env = env->next;
	}
}

int	env_exported_size(t_env *env)
{
	int	i;

	i = 0;
	while (env)
	{
		i += env->exported;
		env = env->next;
	}
	return (i);
}

int	env_size(t_env *env)
{
	int	i;

	i = 0;
	while (env)
	{
		env = env->next;
		i++;
	}
	return (i);
}

char	**env_to_array(t_env *env)
{
	char	**array;
	char	*tmp;
	int		i;

	array = malloc(sizeof(char *) * (env_exported_size(env) + 1));
	if (!array)
		return (NULL);
	i = 0;
	while (env)
	{
		if (env->exported && env->value[0] != ARR_MARK)
		{
			tmp = ft_strjoin(env->key, "=");
			array[i] = ft_strjoin(tmp, env->value);
			free(tmp);
			if (!array[i++])
				return (array[i - 1] = NULL, free_str_array(array), NULL);
		}
		env = env->next;
	}
	array[i] = NULL;
	return (array);
}
