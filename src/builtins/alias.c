#include "../includes/builtins.h"
#include "../includes/minishell.h"

static void	print_alias(t_env *alias)
{
	char	*quoted;

	quoted = quote_word(alias->value);
	ml_put_all(STDOUT_FILENO, alias->key, "=", quoted, "\n", NULL);
	free(quoted);
}

static int	alias_error(char *cmd, char *name, char *msg)
{
	ml_put_all(2, "minishell: ", cmd, ": ", name, ": ", msg, "\n", NULL);
	return (1);
}

static int	define_alias(char *arg, t_shell *shell)
{
	char	*equal;
	char	*name;

	equal = ft_strchr(arg, '=');
	name = ft_substr(arg, 0, equal - arg);
	if (!name || !*name || ft_strchr(name, '/'))
		return (free(name), alias_error("alias", arg, "invalid alias name"));
	env_set(&shell->aliases, name, equal + 1);
	free(name);
	return (0);
}

static void	print_all(t_shell *shell)
{
	t_env	*cur;
	char	**keys;
	int		n;

	n = env_size(shell->aliases);
	keys = ft_calloc(n + 1, sizeof(char *));
	cur = shell->aliases;
	n = 0;
	while (keys && cur)
	{
		keys[n++] = cur->key;
		cur = cur->next;
	}
	ml_sort_strings(keys, n);
	n = 0;
	while (keys && keys[n])
	{
		cur = shell->aliases;
		while (cur && ft_strcmp(cur->key, keys[n]) != 0)
			cur = cur->next;
		print_alias(cur);
		n++;
	}
	free(keys);
}

int	builtin_alias(char **argv, t_shell *shell)
{
	t_env	*cur;
	int		status;
	int		i;

	i = 1;
	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	if (!argv[i])
		print_all(shell);
	status = 0;
	while (argv[i])
	{
		cur = shell->aliases;
		while (cur && ft_strcmp(cur->key, argv[i]) != 0)
			cur = cur->next;
		if (ft_strchr(argv[i], '='))
			status |= define_alias(argv[i], shell);
		else if (cur)
			print_alias(cur);
		else
			status |= alias_error("alias", argv[i], "not found");
		i++;
	}
	return (status);
}

int	builtin_unalias(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	i = 1;
	if (argv[i] && ft_strcmp(argv[i], "-a") == 0)
	{
		env_free(shell->aliases);
		shell->aliases = NULL;
		return (0);
	}
	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	status = 0;
	while (argv[i])
	{
		if (!env_get(shell->aliases, argv[i]))
			status |= alias_error("unalias", argv[i], "not found");
		else
			env_unset(&shell->aliases, argv[i]);
		i++;
	}
	return (status);
}
