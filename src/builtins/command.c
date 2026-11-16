#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <unistd.h>

char	*quote_word(const char *word)
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

	if (is_reserved_word(name))
		return (ml_put_all(STDOUT_FILENO, name, "\n", NULL), 0);
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

static int	parse_flags(char **argv, int *mode, int *std_path)
{
	int	i;
	int	k;

	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0)
			return (i + 1);
		k = 1;
		while (argv[i][k])
		{
			if (argv[i][k] == 'v')
				*mode = 1;
			else if (argv[i][k] == 'V')
				*mode = 2;
			else if (argv[i][k] == 'p')
				*std_path = 1;
			else
				return (ml_put_all(2, "minishell: command: ", argv[i],
						": invalid option\n", NULL), -1);
			k++;
		}
		i++;
	}
	return (i);
}

static int	run_mode(char **words, int mode, t_shell *shell)
{
	int	status;

	if (mode == 2)
		return (builtin_type(words - 1, shell));
	if (mode == 0)
		return (run_words(words, shell));
	status = 0;
	while (*words)
		status |= describe(*words++, shell);
	return (status);
}

static char	*standard_path(void)
{
	char	*buf;
	size_t	n;

	n = confstr(_CS_PATH, NULL, 0);
	buf = malloc(n ? n : 1);
	if (!buf)
		return (NULL);
	if (n)
		confstr(_CS_PATH, buf, n);
	else
		buf[0] = '\0';
	return (buf);
}

int	builtin_command(char **argv, t_shell *shell)
{
	int		i;
	int		mode;
	int		std_path;
	int		status;
	char	*saved;
	char	*std;

	mode = 0;
	std_path = 0;
	i = parse_flags(argv, &mode, &std_path);
	if (i < 0)
		return (2);
	if (!argv[i])
		return (0);
	saved = NULL;
	if (std_path)
	{
		saved = ft_strdup(env_get(shell->env, "PATH") ? env_get(shell->env,
					"PATH") : "");
		std = standard_path();
		env_assign(&shell->env, "PATH", std ? std : "");
		free(std);
	}
	status = run_mode(argv + i, mode, shell);
	if (saved)
		env_assign(&shell->env, "PATH", saved);
	free(saved);
	return (status);
}
