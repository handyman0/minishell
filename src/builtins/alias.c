#include "../includes/builtins.h"
#include "../includes/minishell.h"

static void	print_alias(t_env *alias)
{
	ft_putstr_fd("alias ", STDOUT_FILENO);
	ft_putstr_fd(alias->key, STDOUT_FILENO);
	ft_putstr_fd("='", STDOUT_FILENO);
	ft_putstr_fd(alias->value, STDOUT_FILENO);
	ft_putendl_fd("'", STDOUT_FILENO);
}

static t_env	*find_alias(t_env *list, const char *name)
{
	while (list && ft_strcmp(list->key, name) != 0)
		list = list->next;
	return (list);
}

static int	alias_error(char *cmd, char *name, char *msg)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(name, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putendl_fd(msg, STDERR_FILENO);
	return (1);
}

static int	define_alias(char *arg, t_shell *shell)
{
	char	*equal;
	char	*name;

	equal = ft_strchr(arg, '=');
	if (equal == arg || ft_strchr(arg, '/'))
		return (alias_error("alias", arg, "nome invalido"));
	name = ft_substr(arg, 0, equal - arg);
	if (!name)
		return (1);
	env_set(&shell->aliases, name, equal + 1);
	free(name);
	return (0);
}

int	builtin_alias(char **argv, t_shell *shell)
{
	t_env	*cur;
	int		status;
	int		i;

	status = 0;
	if (!argv[1])
	{
		cur = shell->aliases;
		while (cur)
		{
			print_alias(cur);
			cur = cur->next;
		}
	}
	i = 1;
	while (argv[i])
	{
		if (ft_strchr(argv[i], '='))
			status |= define_alias(argv[i], shell);
		else if (find_alias(shell->aliases, argv[i]))
			print_alias(find_alias(shell->aliases, argv[i]));
		else
			status |= alias_error("alias", argv[i], "nao encontrado");
		i++;
	}
	return (status);
}

int	builtin_unalias(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	if (argv[1] && ft_strcmp(argv[1], "-a") == 0)
	{
		env_free(shell->aliases);
		shell->aliases = NULL;
		return (0);
	}
	status = 0;
	i = 1;
	while (argv[i])
	{
		if (!env_get(shell->aliases, argv[i]))
			status |= alias_error("unalias", argv[i], "nao encontrado");
		else
			env_unset(&shell->aliases, argv[i]);
		i++;
	}
	return (status);
}
