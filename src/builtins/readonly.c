#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

static void	print_readonly(t_shell *shell)
{
	t_env	*var;

	var = shell->env;
	while (var)
	{
		if (var->readonly)
			ml_put_all(STDOUT_FILENO, "readonly ", var->key, "='", var->value,
				"'\n", NULL);
		var = var->next;
	}
}

static int	readonly_one(char *arg, t_shell *shell)
{
	char	*equal;
	char	*key;
	int		status;

	equal = ft_strchr(arg, '=');
	key = equal ? ft_substr(arg, 0, equal - arg) : ft_strdup(arg);
	status = 0;
	if (!key || !*key || ft_isdigit(*key))
		status = (ml_put_all(2, "minishell: readonly: ", arg,
					": not a valid identifier\n", NULL), 1);
	else if (equal && env_is_readonly(shell->env, key))
		status = (ml_put_all(2, "minishell: ", key,
					": readonly variable\n", NULL), shell->running = shell->spare, 1);
	else
	{
		if (equal)
			env_set(&shell->env, key, equal + 1);
		else if (!env_get(shell->env, key))
			env_set(&shell->env, key, "");
		env_set_readonly(shell->env, key);
	}
	free(key);
	return (status);
}

int	builtin_readonly(char **argv, t_shell *shell)
{
	int	i;
	int	status;

	i = 1;
	if (argv[i] && ft_strcmp(argv[i], "-p") == 0)
		i++;
	if (!argv[i])
		return (print_readonly(shell), 0);
	status = 0;
	while (argv[i])
		status |= readonly_one(argv[i++], shell);
	return (status);
}
