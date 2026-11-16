#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

static char	*quote_word(const char *word)
{
	char	*out;
	size_t	i;
	size_t	j;

	out = malloc(ft_strlen(word) * 4 + 3);
	if (!out)
		return (NULL);
	j = 0;
	out[j++] = '\'';
	i = 0;
	while (word[i])
	{
		if (word[i] == '\'')
		{
			ft_memcpy(out + j, "'\\''", 4);
			j += 4;
		}
		else
			out[j++] = word[i];
		i++;
	}
	out[j++] = '\'';
	out[j] = '\0';
	return (out);
}

static int	describe(const char *name, t_shell *shell)
{
	char	*path;
	char	*alias;

	if (find_func(shell, name))
		return (ml_put_all(STDOUT_FILENO, name, "\n", NULL), 0);
	alias = env_get(shell->aliases, name);
	if (alias)
		return (ml_put_all(STDOUT_FILENO, "alias ", name, "='", alias, "'\n",
				NULL), 0);
	if (is_builtin(name))
		return (ml_put_all(STDOUT_FILENO, name, "\n", NULL), 0);
	path = find_executable(name, shell);
	if (!path)
		return (1);
	ml_put_all(STDOUT_FILENO, path, "\n", NULL);
	free(path);
	return (0);
}

static int	run_words(char **words, t_shell *shell)
{
	char	*text;
	char	*quoted;
	char	*tmp;
	int		i;

	text = ft_strdup("");
	i = 0;
	while (words[i])
	{
		quoted = quote_word(words[i]);
		tmp = ft_strjoin(text, i ? " " : "");
		free(text);
		text = ft_strjoin(tmp, quoted);
		free(tmp);
		free(quoted);
		i++;
	}
	shell->bypass_func = 1;
	shell->exit_status = 0;
	run_line(text, shell);
	shell->bypass_func = 0;
	free(text);
	return (shell->exit_status);
}

int	builtin_command(char **argv, t_shell *shell)
{
	int	i;
	int	mode;
	int	status;

	mode = 0;
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0 && ++i)
			break ;
		if (ft_strcmp(argv[i], "-v") == 0)
			mode = 1;
		else if (ft_strcmp(argv[i], "-V") == 0)
			mode = 2;
		else if (ft_strcmp(argv[i], "-p") != 0)
			return (ml_put_all(2, "minishell: command: ", argv[i],
					": invalid option\n", NULL), 2);
		i++;
	}
	if (!argv[i])
		return (0);
	if (mode == 2)
		return (builtin_type(argv + i - 1, shell));
	if (mode == 0)
		return (run_words(argv + i, shell));
	status = 0;
	while (argv[i])
		status |= describe(argv[i++], shell);
	return (status);
}
