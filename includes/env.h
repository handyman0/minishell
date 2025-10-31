/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:01:35 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/31 19:40:31 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENV_H
# define ENV_H

# include "../libft/include/libft.h"
# include "../libft/include/ft_printf.h"
# include "utils.h"
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

/* ======================== */
/* Inicialização de limpeza */
/* ======================== */
t_env	*env_init(char **envp);
void	env_free(t_env *env);

/* =================== */
/* Busca e modificação */
/* =================== */

char	*env_get(t_env *env, const char *key);
char	*env_set(t_env **env, const char *key, const char *value);
char	*env_unset(t_env *env, const char *key);

/* =================== */
/* Utilitarios e debug */
/* =================== */

void	env_print(t_env *env);
int		env_size(t_env *env);
char	**env_to_array(t_env *env);

#endif
