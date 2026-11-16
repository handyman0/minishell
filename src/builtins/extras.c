#include "../../includes/builtins.h"
#include "../../includes/parser.h"
#include "../../includes/lineedit.h"
#include "../../includes/output.h"
#include "../../includes/executor.h"

int	builtin_let(char **argv, t_shell *shell)
{
	char	*res;
	int		i;
	long	last;

	if (!argv[1])
		return (ml_put_all(2, "minishell: let: expression expected\n", NULL),
			1);
	last = 0;
	i = 1;
	while (argv[i])
	{
		res = arith_string(argv[i++], shell);
		if (!res || (res[0] == '\0'))
			return (free(res), 1);
		last = ft_atoi(res);
		free(res);
	}
	return (last == 0);
}

int	builtin_builtin(char **argv, t_shell *shell)
{
	t_node	node;

	if (!argv[1])
		return (0);
	if (!is_builtin(argv[1]))
		return (ml_put_all(2, "minishell: builtin: ", argv[1],
				": not a shell builtin\n", NULL), 1);
	ft_bzero(&node, sizeof(node));
	node.type = NODE_CMD;
	node.data.cmd.argv = argv + 1;
	shell->bypass_func = 1;
	return (execute_builtin(&node, shell));
}

static void	history_print(int from)
{
	char	*num;
	int		i;

	i = from;
	while (i < le_history_count())
	{
		num = ft_itoa(i + 1);
		ml_put_all(STDOUT_FILENO, "  ", num, "  ", le_history_get(i), "\n",
			NULL);
		free(num);
		i++;
	}
}

int	builtin_history(char **argv, t_shell *shell)
{
	int	count;

	(void)shell;
	count = le_history_count();
	if (argv[1] && ft_strcmp(argv[1], "-c") == 0)
		return (le_history_clear(), 0);
	if (argv[1] && ft_isdigit(argv[1][0]))
		history_print(count > ft_atoi(argv[1]) ? count - ft_atoi(argv[1]) : 0);
	else if (argv[1])
		return (ml_put_all(2, "minishell: history: ", argv[1],
				": invalid option\n", NULL), 2);
	else
		history_print(0);
	return (0);
}
