#include "../../includes/builtins.h"
#include "../../includes/output.h"
#include <sys/resource.h>

typedef struct s_lim
{
	char	opt;
	int		res;
	long	unit;
	char	*desc;
}	t_lim;

static const t_lim	g_lims[] = {
{'c', RLIMIT_CORE, 1024, "core file size          (blocks, -c) "},
{'d', RLIMIT_DATA, 1024, "data seg size           (kbytes, -d) "},
{'f', RLIMIT_FSIZE, 1024, "file size               (blocks, -f) "},
{'n', RLIMIT_NOFILE, 1, "open files                      (-n) "},
{'s', RLIMIT_STACK, 1024, "stack size              (kbytes, -s) "},
{'t', RLIMIT_CPU, 1, "cpu time               (seconds, -t) "},
{'u', RLIMIT_NPROC, 1, "max user processes              (-u) "},
{'v', RLIMIT_AS, 1024, "virtual memory          (kbytes, -v) "},
{'l', RLIMIT_MEMLOCK, 1024, "max locked memory       (kbytes, -l) "},
{0, 0, 0, NULL}};

static const t_lim	*find(char opt)
{
	int	i;

	i = 0;
	while (g_lims[i].opt && g_lims[i].opt != opt)
		i++;
	return (g_lims[i].opt ? &g_lims[i] : NULL);
}

static void	show(const t_lim *l, int hard)
{
	struct rlimit	rl;
	char			*num;

	getrlimit(l->res, &rl);
	if ((hard ? rl.rlim_max : rl.rlim_cur) == RLIM_INFINITY)
		ml_put_all(STDOUT_FILENO, "unlimited\n", NULL);
	else
	{
		num = ft_itoa((int)((hard ? rl.rlim_max : rl.rlim_cur) / l->unit));
		ml_put_all(STDOUT_FILENO, num, "\n", NULL);
		free(num);
	}
}

static int	set(const t_lim *l, const char *val, int hard, int soft)
{
	struct rlimit	rl;
	rlim_t			v;

	getrlimit(l->res, &rl);
	if (ft_strcmp(val, "unlimited") == 0)
		v = RLIM_INFINITY;
	else if (ft_isdigit(val[0]))
		v = (rlim_t)ft_atoi(val) * l->unit;
	else
		return (ml_put_all(2, "minishell: ulimit: ", val,
				": invalid number\n", NULL), 1);
	if (hard || !soft)
		rl.rlim_max = v;
	if (soft || !hard)
		rl.rlim_cur = v;
	if (setrlimit(l->res, &rl) != 0)
		return (put_errno("ulimit", NULL), 1);
	return (0);
}

static void	show_all(int hard)
{
	int	i;

	i = 0;
	while (g_lims[i].opt)
	{
		ml_put_all(STDOUT_FILENO, g_lims[i].desc, NULL);
		show(&g_lims[i], hard);
		i++;
	}
}

int	builtin_ulimit(char **argv, t_shell *shell)
{
	const t_lim	*lim;
	int			hard;
	int			soft;
	int			i;
	int			k;

	(void)shell;
	hard = 0;
	soft = 0;
	lim = find('f');
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		k = 0;
		while (argv[i][++k])
		{
			if (argv[i][k] == 'H')
				hard = 1;
			else if (argv[i][k] == 'S')
				soft = 1;
			else if (argv[i][k] == 'a')
				return (show_all(hard), 0);
			else if (find(argv[i][k]))
				lim = find(argv[i][k]);
			else
				return (ml_put_all(2, "minishell: ulimit: bad option\n", NULL),
					2);
		}
		i++;
	}
	if (argv[i])
		return (set(lim, argv[i], hard, soft));
	return (show(lim, hard), 0);
}
