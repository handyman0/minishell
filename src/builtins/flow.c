#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

int	builtin_true(char **argv, t_shell *shell)
{
	(void)argv;
	(void)shell;
	return (0);
}

int	builtin_false(char **argv, t_shell *shell)
{
	(void)argv;
	(void)shell;
	return (1);
}

static int	loop_count(char **argv, t_shell *shell, const char *name)
{
	int	n;

	n = argv[1] ? ft_atoi(argv[1]) : 1;
	if (n < 1)
		return (ml_put_all(2, "minishell: ", name,
				": loop count out of range\n", NULL), -1);
	if (shell->loop_depth == 0)
		return (0);
	return (n > shell->loop_depth ? shell->loop_depth : n);
}

int	builtin_break(char **argv, t_shell *shell)
{
	int	n;

	n = loop_count(argv, shell, "break");
	if (n < 0)
		return (1);
	shell->brk = n;
	return (0);
}

int	builtin_continue(char **argv, t_shell *shell)
{
	int	n;

	n = loop_count(argv, shell, "continue");
	if (n < 0)
		return (1);
	shell->cont = n;
	return (0);
}

int	builtin_return(char **argv, t_shell *shell)
{
	if (shell->func_depth == 0)
		return (ml_put_all(2, "minishell: return: can only return from a "
				"function\n", NULL), 1);
	shell->ret = 1;
	if (argv[1])
		return (ft_atoi(argv[1]) & 255);
	return (shell->exit_status);
}

int	builtin_local(char **argv, t_shell *shell)
{
	char	*equal;
	char	*key;
	int		i;

	if (shell->func_depth == 0)
		return (ml_put_all(2, "minishell: local: can only be used in a "
				"function\n", NULL), 1);
	i = 1;
	while (argv[i])
	{
		equal = ft_strchr(argv[i], '=');
		key = equal ? ft_substr(argv[i], 0, equal - argv[i])
			: ft_strdup(argv[i]);
		push_local(shell, key);
		if (equal)
			env_assign(&shell->env, key, equal + 1);
		else if (!env_get(shell->env, key))
			env_assign(&shell->env, key, "");
		free(key);
		i++;
	}
	return (0);
}

int	builtin_shift(char **argv, t_shell *shell)
{
	int	n;
	int	i;

	n = argv[1] ? ft_atoi(argv[1]) : 1;
	if (n < 0 || n > shell->nparams)
		return (1);
	i = 0;
	while (i < n)
		free(shell->params[i++]);
	i = 0;
	while (i + n <= shell->nparams)
	{
		shell->params[i] = shell->params[i + n];
		i++;
	}
	shell->nparams -= n;
	return (0);
}

int	builtin_set(char **argv, t_shell *shell)
{
	int	i;

	if (!argv[1] || ft_strcmp(argv[1], "--") != 0)
		return (0);
	i = 0;
	while (i < shell->nparams)
		free(shell->params[i++]);
	free(shell->params);
	shell->nparams = 0;
	while (argv[2 + shell->nparams])
		shell->nparams++;
	shell->params = ft_calloc(shell->nparams + 1, sizeof(char *));
	i = 0;
	while (shell->params && i < shell->nparams)
	{
		shell->params[i] = ft_strdup(argv[2 + i]);
		i++;
	}
	return (0);
}

int	builtin_eval(char **argv, t_shell *shell)
{
	char	*text;
	char	*tmp;
	int		i;

	text = ft_strdup("");
	i = 1;
	while (argv[i])
	{
		tmp = ft_strjoin(text, i > 1 ? " " : "");
		free(text);
		text = ft_strjoin(tmp, argv[i++]);
		free(tmp);
	}
	shell->exit_status = 0;
	run_line(text, shell);
	free(text);
	return (shell->exit_status);
}
