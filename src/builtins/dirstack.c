#include "../../includes/builtins.h"
#include "../../includes/output.h"

#define DIRS_MAX 64

static char	*g_dirs[DIRS_MAX];
static int	g_ndirs;

void	dirs_free(void)
{
	while (g_ndirs > 0)
		free(g_dirs[--g_ndirs]);
}

static void	show(t_shell *shell, int per_line)
{
	char	*cwd;
	int		i;

	cwd = logical_pwd(shell);
	ml_put_all(STDOUT_FILENO, cwd ? cwd : "?", NULL);
	free(cwd);
	i = g_ndirs;
	while (i-- > 0)
		ml_put_all(STDOUT_FILENO, per_line ? "\n" : " ", g_dirs[i], NULL);
	ft_putchar_fd('\n', STDOUT_FILENO);
}

int	builtin_dirs(char **argv, t_shell *shell)
{
	if (argv[1] && ft_strcmp(argv[1], "-c") == 0)
		return (dirs_free(), 0);
	show(shell, argv[1] && (ft_strcmp(argv[1], "-p") == 0
			|| ft_strcmp(argv[1], "-v") == 0));
	return (0);
}

static int	go(const char *dir, t_shell *shell)
{
	char	*argv[3];

	argv[0] = "cd";
	argv[1] = (char *)dir;
	argv[2] = NULL;
	return (builtin_cd(argv, shell));
}

int	builtin_pushd(char **argv, t_shell *shell)
{
	char	*cwd;
	char	*target;

	cwd = logical_pwd(shell);
	target = argv[1] ? argv[1] : (g_ndirs ? g_dirs[g_ndirs - 1] : NULL);
	if (!target || !cwd || g_ndirs >= DIRS_MAX)
		return (free(cwd), ml_put_all(2, "minishell: pushd: no other "
				"directory\n", NULL), 1);
	target = ft_strdup(target);
	if (go(target, shell) != 0)
		return (free(cwd), free(target), 1);
	if (!argv[1])
		free(g_dirs[--g_ndirs]);
	g_dirs[g_ndirs++] = cwd;
	free(target);
	show(shell, 0);
	return (0);
}

int	builtin_popd(char **argv, t_shell *shell)
{
	char	*dir;

	(void)argv;
	if (g_ndirs == 0)
		return (ml_put_all(2, "minishell: popd: directory stack empty\n",
				NULL), 1);
	dir = g_dirs[--g_ndirs];
	if (go(dir, shell) != 0)
		return (g_dirs[g_ndirs++] = dir, 1);
	free(dir);
	show(shell, 0);
	return (0);
}
