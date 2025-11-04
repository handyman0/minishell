/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 15:55:16 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:15:37 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/env.h"

void	env_print(t_env *env)
{
	while (env)
	{
		ft_printf("%s=%s\n", env->key, env->value);
		env = env->next;
	}
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
	int		i;
	int		size;
	char	**array;
	char	*tmp;

	size = env_size(env);
	array = malloc(sizeof(char *) * (size + 1));
	if (!array)
		return (NULL);
	i = 0;
	while (env && i < size)
	{
		tmp = ft_strjoin(env->key, "=");
		if (!tmp)
			break ;
		array[i] = ft_strjoin(tmp, env->value);
		free(tmp);
		if (!array[i])
			break ;
		env = env->next;
		i++;
	}
	array[i] = NULL;
	return (array);
}
