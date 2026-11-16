#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

typedef struct s_option
{
	char			letter;
	const char		*name;
	unsigned int	bit;
}	t_option;

static const t_option	g_options[] = {{'a', "allexport", OPT_A},
{'C', "noclobber", OPT_C}, {'e', "errexit", OPT_E}, {'f', "noglob", OPT_F},
{'m', "monitor", OPT_M}, {'n', "noexec", OPT_N}, {'u', "nounset", OPT_U},
{'v', "verbose", OPT_V}, {'x', "xtrace", OPT_X}, {'h', "hashondef", OPT_H},
{'b', "notify", OPT_B}, {'B', "braceexpand", OPT_BRACE}, {0, "ignoreeof", OPT_I}, {0, NULL, 0}};

char	*option_string(t_shell *shell)
{
	char	buf[16];
	int		n;
	int		i;

	n = 0;
	i = 0;
	while (g_options[i].name)
	{
		if ((shell->opts & g_options[i].bit) && g_options[i].letter)
			buf[n++] = g_options[i].letter;
		i++;
	}
	if (shell->interactive)
		buf[n++] = 'i';
	buf[n] = '\0';
	return (ft_strdup(buf));
}

static const t_option	*by_letter(char letter)
{
	int	i;

	if (!letter)
		return (NULL);

	i = 0;
	while (g_options[i].name)
	{
		if (g_options[i].letter == letter)
			return (&g_options[i]);
		i++;
	}
	return (NULL);
}

static const t_option	*by_name(const char *name)
{
	int	i;

	i = 0;
	while (g_options[i].name)
	{
		if (ft_strcmp(g_options[i].name, name) == 0)
			return (&g_options[i]);
		i++;
	}
	return (NULL);
}

static void	list_options(t_shell *shell, int reusable)
{
	int			i;
	const char	*state;

	i = 0;
	while (g_options[i].name)
	{
		state = (shell->opts & g_options[i].bit) ? "on" : "off";
		if (reusable)
			ml_put_all(STDOUT_FILENO, "set ", (shell->opts & g_options[i].bit)
				? "-o " : "+o ", g_options[i].name, "\n", NULL);
		else
			ml_put_all(STDOUT_FILENO, g_options[i].name,
				"                " + ft_strlen(g_options[i].name), state,
				"\n", NULL);
		i++;
	}
}

static int	apply(t_shell *shell, const t_option *opt, int on)
{
	if (on)
		shell->opts |= opt->bit;
	else
		shell->opts &= ~opt->bit;
	g_noclobber = (shell->opts & OPT_C) != 0;
	return (0);
}

static int	set_letters(t_shell *shell, const char *arg)
{
	const t_option	*opt;
	int				k;

	k = 1;
	while (arg[k])
	{
		opt = by_letter(arg[k]);
		if (!opt)
			return (ml_put_all(2, "minishell: set: ", arg[0] == '-' ? "-" : "+",
					(char []){arg[k], '\0'}, ": invalid option\n", NULL), 2);
		apply(shell, opt, arg[0] == '-');
		k++;
	}
	return (0);
}

static int	needs_quoting(const char *value)
{
	if (!*value)
		return (1);
	while (*value)
	{
		if (!ft_isalnum(*value) && !ft_strchr("_./-:,+@%=", *value))
			return (1);
		value++;
	}
	return (0);
}

static void	print_variables(t_shell *shell)
{
	t_env	*var;
	char	**keys;
	char	*quoted;
	int		n;
	int		i;

	n = env_size(shell->env);
	keys = ft_calloc(n + 1, sizeof(char *));
	var = shell->env;
	i = 0;
	while (keys && var)
	{
		keys[i++] = var->key;
		var = var->next;
	}
	ml_sort_strings(keys, i);
	n = 0;
	while (keys && n < i)
	{
		if (arr_is(env_get(shell->env, keys[n])))
		{
			quoted = arr_display(env_get(shell->env, keys[n]));
			ml_put_all(STDOUT_FILENO, keys[n], "=", quoted, "\n", NULL);
			free(quoted);
			n++;
			continue ;
		}
		quoted = needs_quoting(env_get(shell->env, keys[n]))
			? quote_word(env_get(shell->env, keys[n])) : NULL;
		ml_put_all(STDOUT_FILENO, keys[n], "=", quoted ? quoted
			: env_get(shell->env, keys[n]), "\n", NULL);
		free(quoted);
		n++;
	}
	free(keys);
}

static void	set_params(t_shell *shell, char **args)
{
	int	i;

	i = 0;
	while (i < shell->nparams)
		free(shell->params[i++]);
	free(shell->params);
	shell->nparams = 0;
	while (args[shell->nparams])
		shell->nparams++;
	shell->params = ft_calloc(shell->nparams + 1, sizeof(char *));
	i = 0;
	while (shell->params && i < shell->nparams)
	{
		shell->params[i] = ft_strdup(args[i]);
		i++;
	}
}

static int	named_option(t_shell *shell, char **argv, int *i)
{
	const t_option	*opt;
	int				on;

	on = argv[*i][0] == '-';
	if (!argv[*i + 1])
		return (list_options(shell, !on), 0);
	opt = by_name(argv[*i + 1]);
	if (!opt)
		return (ml_put_all(2, "minishell: set: ", argv[*i + 1],
				": invalid option name\n", NULL), 2);
	apply(shell, opt, on);
	*i += 1;
	return (0);
}

int	builtin_set(char **argv, t_shell *shell)
{
	int	i;
	int	status;

	if (!argv[1])
		return (print_variables(shell), 0);
	i = 1;
	status = 0;
	while (!status && argv[i] && (argv[i][0] == '-' || argv[i][0] == '+')
		&& argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0 && ++i)
			break ;
		if (argv[i][1] == 'o' && !argv[i][2])
			status = named_option(shell, argv, &i);
		else
			status = set_letters(shell, argv[i]);
		i++;
	}
	if (status)
		return (shell->running = shell->spare, status);
	if (argv[i] || (i > 1 && ft_strcmp(argv[i - 1], "--") == 0))
		set_params(shell, argv + i);
	return (0);
}

unsigned int	option_bit_letter(char letter)
{
	const t_option	*opt;

	opt = by_letter(letter);
	return (opt ? opt->bit : 0);
}

unsigned int	option_bit_name(const char *name)
{
	const t_option	*opt;

	opt = by_name(name);
	return (opt ? opt->bit : 0);
}
