#include "../../includes/builtins.h"
#include "../../includes/output.h"

unsigned int	g_shopts;

typedef struct s_shopt
{
	const char		*name;
	unsigned int	bit;
}	t_shopt;

static const t_shopt	g_table[] = {{"nullglob", SHOPT_NULLGLOB},
{"dotglob", SHOPT_DOTGLOB}, {"extglob", SHOPT_EXTGLOB},
{"globstar", SHOPT_GLOBSTAR}, {"nocaseglob", SHOPT_NOCASEGLOB},
{"nocasematch", SHOPT_NOCASEMATCH}, {"expand_aliases", SHOPT_ALIASES},
{"failglob", SHOPT_FAILGLOB}, {"checkwinsize", 0}, {"cmdhist", 0},
{"histappend", 0}, {"lastpipe", 0}, {"progcomp", 0}, {"promptvars", 0},
{"sourcepath", 0}, {"interactive_comments", 0}, {"huponexit", 0},
{"extquote", 0}, {"force_fignore", 0}, {"hostcomplete", 0}, {NULL, 0}};

static const t_shopt	*find(const char *name)
{
	int	i;

	i = 0;
	while (g_table[i].name && ft_strcmp(g_table[i].name, name) != 0)
		i++;
	return (g_table[i].name ? &g_table[i] : NULL);
}

static void	show(const t_shopt *s, int plain)
{
	int	on;

	on = s->bit && (g_shopts & s->bit);
	if (plain)
		ml_put_all(STDOUT_FILENO, "shopt -", on ? "s " : "u ", s->name, "\n",
			NULL);
	else
		ml_put_all(STDOUT_FILENO, s->name, on ? "\ton\n" : "\toff\n", NULL);
}

static int	list_all(int only_on, int plain)
{
	int	i;

	i = 0;
	while (g_table[i].name)
	{
		if (!only_on || (g_table[i].bit && (g_shopts & g_table[i].bit)))
			show(&g_table[i], plain);
		i++;
	}
	return (0);
}

int	builtin_shopt(char **argv, t_shell *shell)
{
	int				mode;
	int				quiet;
	int				i;
	int				status;
	const t_shopt	*s;

	(void)shell;
	mode = 0;
	quiet = 0;
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		mode = argv[i][1] == 's' ? 1 : (argv[i][1] == 'u' ? 2 : mode);
		quiet |= argv[i][1] == 'q';
		i++;
	}
	if (!argv[i])
		return (list_all(mode == 1, 0));
	status = 0;
	while (argv[i])
	{
		s = find(argv[i++]);
		if (!s)
			status = (ml_put_all(2, "minishell: shopt: ", argv[i - 1],
						": invalid shell option name\n", NULL), 1);
		else if (mode == 1 || mode == 2)
			g_shopts = mode == 1 ? g_shopts | s->bit : g_shopts & ~s->bit;
		else if (quiet)
			status |= !(s->bit && (g_shopts & s->bit));
		else
			show(s, 0);
	}
	return (status);
}
