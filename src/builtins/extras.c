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

static char	*slurp(void)
{
	char	buf[4096];
	char	*text;
	char	*tmp;
	char	*chunk;
	ssize_t	n;

	text = ft_strdup("");
	n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
	while (text && n > 0)
	{
		buf[n] = '\0';
		chunk = ft_substr(buf, 0, n);
		tmp = ft_strjoin(text, chunk);
		free(chunk);
		free(text);
		text = tmp;
		n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
	}
	return (text);
}

static char	**split_lines(const char *text, int keep, int *n)
{
	char	**out;
	int		i;
	int		start;

	out = ft_calloc(ft_strlen(text) + 2, sizeof(char *));
	*n = 0;
	i = 0;
	start = 0;
	while (out && text[i])
	{
		if (text[i] == '\n')
		{
			out[(*n)++] = ft_substr(text, start, i - start + (keep != 0));
			start = i + 1;
		}
		i++;
	}
	if (out && text[start])
		out[(*n)++] = ft_substr(text, start, i - start);
	return (out);
}

int	builtin_mapfile(char **argv, t_shell *shell)
{
	char	*text;
	char	**lines;
	char	*joined;
	int		keep;
	int		n;
	int		i;

	keep = 1;
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
		keep &= !(ft_strchr(argv[i++], 't') != NULL);
	text = slurp();
	lines = split_lines(text ? text : "", keep, &n);
	joined = lines ? arr_join(lines, n) : NULL;
	if (joined)
		env_assign(&shell->env, argv[i] ? argv[i] : "MAPFILE", joined);
	free(joined);
	free(text);
	return (free_str_array(lines), 0);
}
