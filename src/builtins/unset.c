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
#include "../includes/parser.h"

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

static int	unset_assoc(char *key, char *open, t_shell *shell)
{
	char	*sub;
	char	*text;
	char	*res;

	sub = ft_substr(open + 1, 0, ft_strlen(open) - 2);
	text = expand_variables(sub ? sub : "", shell);
	res = assoc_set(env_get(shell->env, key), text ? text : "", "", 1);
	env_assign(&shell->env, key, res);
	return (free(sub), free(text), free(res), free(key), 0);
}

static int	unset_element(char *name, t_shell *shell)
{
	char	*open;
	char	*key;
	char	*num;
	char	*sub;
	char	**el;
	char	*joined;
	int		n;
	long	idx;

	open = ft_strchr(name, '[');
	key = ft_substr(name, 0, open - name);
	if (assoc_is(env_get(shell->env, key)))
		return (unset_assoc(key, open, shell));
	sub = ft_substr(open + 1, 0, ft_strlen(open) - 2);
	num = arith_string(sub ? sub : "0", shell);
	free(sub);
	idx = num ? ft_atoi(num) : 0;
	free(num);
	el = arr_split(env_get(shell->env, key), &n);
	if (el && idx >= 0 && idx < n)
	{
		free(el[idx]);
		while (idx++ < n)
			el[idx - 1] = el[idx];
		joined = arr_join(el, n - 1);
		env_assign(&shell->env, key, joined);
		free(joined);
	}
	free_str_array(el);
	return (free(key), 0);
}

static int	unset_one(char *name, int functions, t_shell *shell)
{
	if (functions)
		return (func_remove(shell, name), 0);
	if (ft_strchr(name, '[') && name[ft_strlen(name) - 1] == ']')
		return (unset_element(name, shell));
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
