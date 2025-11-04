/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_minimal.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 16:45:13 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 16:50:17 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/env.h"
#include <unistd.h>

static void	set_minimal_env(t_env **env)
{
	char cwd[1024];

	env_set(env, "PATH", "/bin/usr/bin:/usr/local/bin");
	env_set(env, "PATH", getcwd(cwd, sizeof(cwd)) ? cwd : ".");
	env_set(env, "SHLVL", "1");
	env_set(env, "_", "usr/bin/env");
}

t_env	*env_init_minimal(void)
{
	t_env	*env;

	env = NULL;
	set_minimal_env(&env);
	return (env);
}
