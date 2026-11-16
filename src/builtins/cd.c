#include "../includes/builtins.h"
#include "../includes/output.h"
#include "../includes/minishell.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define CD_MAX 8192

static int	is_dir(const char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

char	*logical_pwd(t_shell *shell)
{
	struct stat	a;
	struct stat	b;
	char		*pwd;

	pwd = env_get(shell->env, "PWD");
	if (pwd && pwd[0] == '/' && stat(pwd, &a) == 0 && stat(".", &b) == 0
		&& a.st_dev == b.st_dev && a.st_ino == b.st_ino)
		return (ft_strdup(pwd));
	return (getcwd(NULL, 0));
}

static int	pop_component(char *out, int *len)
{
	if (*len > 0 && !is_dir(out))
		return (0);
	while (*len > 0 && out[*len - 1] != '/')
		(*len)--;
	if (*len > 0)
		(*len)--;
	out[*len] = '\0';
	return (1);
}

static char	*canonicalize(const char *path)
{
	char	*out;
	int		len;
	int		i;
	int		n;

	out = ft_calloc(CD_MAX, 1);
	len = 0;
	i = 0;
	while (out && path[i])
	{
		while (path[i] == '/')
			i++;
		n = 0;
		while (path[i + n] && path[i + n] != '/')
			n++;
		if (n == 0)
			break ;
		if (n == 2 && path[i] == '.' && path[i + 1] == '.')
		{
			if (!pop_component(out, &len))
				return (free(out), NULL);
		}
		else if (!(n == 1 && path[i] == '.') && len + n + 2 < CD_MAX)
		{
			out[len++] = '/';
			ft_memcpy(out + len, path + i, n);
			len += n;
			out[len] = '\0';
		}
		i += n;
	}
	if (out && !out[0])
		ft_memcpy(out, "/", 2);
	return (out);
}

static char	*search_cdpath(const char *dir, t_shell *shell, int *print)
{
	char	*cdpath;
	char	*entry;
	char	*cand;
	char	*tmp;
	int		n;

	*print = 0;
	cdpath = env_get(shell->env, "CDPATH");
	if (!cdpath || dir[0] == '/' || ft_strcmp(dir, ".") == 0
		|| ft_strcmp(dir, "..") == 0 || ft_strncmp(dir, "./", 2) == 0
		|| ft_strncmp(dir, "../", 3) == 0)
		return (NULL);
	while (1)
	{
		n = 0;
		while (cdpath[n] && cdpath[n] != ':')
			n++;
		entry = ft_substr(cdpath, 0, n);
		tmp = ft_strjoin(n ? entry : ".", "/");
		cand = ft_strjoin(tmp, dir);
		free(tmp);
		free(entry);
		if (is_dir(cand))
			return (*print = n > 0, cand);
		free(cand);
		if (!cdpath[n])
			return (NULL);
		cdpath += n + 1;
	}
}

static int	only_flags(const char *arg)
{
	int	k;

	k = 1;
	while (arg[k] == 'L' || arg[k] == 'P')
		k++;
	return (arg[k] == '\0');
}

static int	parse_options(char **argv, int *physical)
{
	int	i;
	int	k;

	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0)
			return (i + 1);
		if (!only_flags(argv[i]))
			break ;
		k = 1;
		while (argv[i][k])
			*physical = argv[i][k++] == 'P';
		i++;
	}
	return (i);
}

static char	*pick_target(char **argv, int i, t_shell *shell, int *print)
{
	char	*path;

	*print = 0;
	if (!argv[i] || ft_strcmp(argv[i], "~") == 0)
	{
		path = env_get(shell->env, "HOME");
		if (!path || !*path)
			return (ml_put_all(2, "minishell: cd: HOME not set\n", NULL), NULL);
		return (ft_strdup(path));
	}
	if (ft_strcmp(argv[i], "-") == 0)
	{
		path = env_get(shell->env, "OLDPWD");
		if (!path)
			return (ml_put_all(2, "minishell: cd: OLDPWD not set\n", NULL),
				NULL);
		*print = 1;
		return (ft_strdup(path));
	}
	path = search_cdpath(argv[i], shell, print);
	return (path ? path : ft_strdup(argv[i]));
}

static int	change_dir(char *target, int physical, char *old, char **now)
{
	char	*full;
	char	*canon;
	int		rc;

	if (physical)
	{
		rc = chdir(target);
		*now = rc == 0 ? getcwd(NULL, 0) : NULL;
		return (rc);
	}
	full = target;
	if (target[0] != '/')
	{
		full = ft_strjoin(old, "/");
		canon = ft_strjoin(full, target);
		free(full);
		full = canon;
	}
	canon = canonicalize(full);
	if (full != target)
		free(full);
	if (!canon)
		return (errno = ENOENT, -1);
	rc = chdir(canon);
	*now = rc == 0 ? canon : NULL;
	if (rc != 0)
		free(canon);
	return (rc);
}

int	builtin_cd(char **argv, t_shell *shell)
{
	char	*target;
	char	*old;
	char	*now;
	int		physical;
	int		print;
	int		i;

	physical = 0;
	i = parse_options(argv, &physical);
	if (argv[i] && argv[i + 1])
		return (ml_put_all(2, "minishell: cd: too many arguments\n", NULL), 1);
	target = pick_target(argv, i, shell, &print);
	if (!target)
		return (1);
	old = logical_pwd(shell);
	if (!old || change_dir(target, physical, old, &now) != 0)
	{
		put_errno("cd", argv[i] ? argv[i] : target);
		return (free(target), free(old), 1);
	}
	if (print)
		ml_put_all(STDOUT_FILENO, now, "\n", NULL);
	env_assign(&shell->env, "OLDPWD", old);
	env_assign(&shell->env, "PWD", now);
	return (free(target), free(old), free(now), 0);
}
