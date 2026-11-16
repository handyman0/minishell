#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include "../includes/executor.h"
#include "../includes/output.h"
#include <sys/stat.h>
#include <unistd.h>

static void	not_found(char *cmd, char *name)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(name, STDERR_FILENO);
	ft_putendl_fd(": not found", STDERR_FILENO);
}

static int	path_is_dir(const char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

typedef struct s_ty
{
	int	all;
	int	word;
	int	path;
}	t_ty;

static void	put_path(t_ty *o, const char *name, const char *dir)
{
	if (o->word)
		ml_put_all(STDOUT_FILENO, "file\n", NULL);
	else if (o->path)
		ml_put_all(STDOUT_FILENO, dir, "\n", NULL);
	else
		ml_put_all(STDOUT_FILENO, name, " is ", dir, "\n", NULL);
}

static void	put_absolute_path(t_ty *o, const char *name, t_shell *shell)
{
	char	*cwd;
	char	*tmp;
	char	*full;

	if (name[0] == '/')
		return (put_path(o, name, name));
	cwd = logical_pwd(shell);
	tmp = ft_strjoin(cwd ? cwd : ".", "/");
	full = ft_strjoin(tmp, name);
	put_path(o, name, full);
	free(cwd);
	free(tmp);
	free(full);
}

static int	each_path(char *name, t_shell *shell, t_ty *o)
{
	char	*dirs;
	char	*dir;
	char	*full;
	int		n;
	int		found;

	dirs = env_get(shell->env, "PATH");
	found = 0;
	if (ft_strchr(name, '/'))
	{
		if (access(name, X_OK) != 0 || path_is_dir(name))
			return (0);
		return (put_absolute_path(o, name, shell), 1);
	}
	while (dirs && *dirs && !ft_strchr(name, '/'))
	{
		n = 0;
		while (dirs[n] && dirs[n] != ':')
			n++;
		dir = ft_substr(dirs, 0, n);
		full = ft_strjoin(n ? dir : ".", "/");
		dir = (free(dir), ft_strjoin(full, name));
		free(full);
		if (access(dir, X_OK) == 0 && !path_is_dir(dir) && ++found)
			put_path(o, name, dir);
		free(dir);
		dirs += n + (dirs[n] == ':');
		if (found && !o->all)
			break ;
	}
	return (found);
}

static int	say(t_ty *o, char *name, const char *word, const char *rest)
{
	if (o->path)
		return (1);
	if (o->word)
		ml_put_all(STDOUT_FILENO, word, "\n", NULL);
	else
		ml_put_all(STDOUT_FILENO, name, rest, NULL);
	return (1);
}

static int	describe_type(char *name, t_shell *shell, t_ty *o)
{
	char	*alias;
	int		hit;

	hit = 0;
	if (is_reserved_word(name) && ++hit)
		say(o, name, "keyword", " is a shell keyword\n");
	if ((!hit || o->all) && find_func(shell, name) && ++hit)
		say(o, name, "function", " is a function\n");
	alias = env_get(shell->aliases, name);
	if ((!hit || o->all) && alias && ++hit && !o->path)
	{
		if (o->word)
			ml_put_all(STDOUT_FILENO, "alias\n", NULL);
		else
			ml_put_all(STDOUT_FILENO, name, " is aliased to `", alias, "'\n",
				NULL);
	}
	if ((!hit || o->all) && is_builtin(name) && ++hit)
		say(o, name, "builtin", " is a shell builtin\n");
	if ((!hit || o->all || o->path) && each_path(name, shell, o))
		hit = 1;
	if (!hit && !o->word)
		return (not_found("type", name), 1);
	return (!hit);
}

static int	describe_which(char *name, t_shell *shell)
{
	char	*alias;
	char	*path;

	alias = env_get(shell->aliases, name);
	if (alias)
		return (ml_put_all(STDOUT_FILENO, name, ": aliased to ", alias, "\n", NULL), 0);
	if (is_builtin(name))
		return (ml_put_all(STDOUT_FILENO, name, ": shell built-in command\n", NULL), 0);
	path = find_executable(name, shell);
	if (!path)
		return (not_found("which", name), 1);
	ml_put_all(STDOUT_FILENO, path, "\n", NULL);
	free(path);
	return (0);
}

int	builtin_type(char **argv, t_shell *shell)
{
	t_ty	o;
	int		status;
	int		i;
	int		k;

	ft_bzero(&o, sizeof(o));
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		k = 0;
		while (argv[i][++k])
		{
			o.all |= argv[i][k] == 'a';
			o.word |= argv[i][k] == 't';
			o.path |= argv[i][k] == 'p' || argv[i][k] == 'P';
		}
		i++;
	}
	status = 0;
	while (argv[i])
		status |= describe_type(argv[i++], shell, &o);
	return (status);
}

int	builtin_which(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	if (!argv[1])
		return (1);
	status = 0;
	i = 1;
	while (argv[i])
		status |= describe_which(argv[i++], shell);
	return (status);
}
