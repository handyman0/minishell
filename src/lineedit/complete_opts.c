#include "../../includes/lineedit.h"
#include "../../includes/executor.h"
#include "../../includes/builtins.h"
#include "../../includes/ctl.h"
#include "../../includes/minishell.h"
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char	**environ;

#define HELP_MAX 262144
#define HELP_MS 700
#define CACHE_MAX 64

typedef struct s_strs
{
	char	**v;
	int		n;
	int		cap;
}	t_strs;

typedef struct s_entry
{
	char	*key;
	t_strs	opts;
	t_strs	subs;
}	t_entry;

static t_entry	g_cache[CACHE_MAX];
static int		g_ncache;
static int		g_registered;

static const char	*g_tools[] = {"git", "docker", "podman", "kubectl", "cargo",
	"npm", "yarn", "pnpm", "systemctl", "journalctl", "apt", "apt-get",
	"apt-cache", "dnf", "pacman", "snap", "flatpak", "pip", "pip3", "go",
	"rustup", "gh", "nmcli", "helm", "terraform", "brew", "nix", "loginctl",
	"machinectl", "timedatectl", "hostnamectl", "resolvectl", "bluetoothctl",
	"gcloud", "aws", "az", "composer", "gem", "bundle", "conda", "uv", NULL};

static const char	*g_builtin_opts[][2] = {
{"cd", "-L -P"}, {"pwd", "-L -P"}, {"echo", "-n -e -E"},
{"read", "-r -p -s -t -n -d -a -u"},
{"set", "-e -u -x -f -C -n -v -o +o -a -m -b -h --"},
{"export", "-p -n -f"}, {"unset", "-f -v"}, {"type", "-t -a -p"},
{"declare", "-a -A -i -r -x -p -f -g"}, {"local", "-a -A -i -r -x"},
{"command", "-v -V -p"}, {"shopt", "-s -u -q -p"}, {"umask", "-S -p"},
{"trap", "-l -p"}, {"kill", "-s -l -n"}, {"wait", "-n"},
{"history", "-c -d -a -n -r -w"},
{"ulimit", "-a -S -H -c -d -e -f -i -l -m -n -p -q -r -s -t -u -v -x"},
{"test", "-e -f -d -r -w -x -s -z -n -L -h -b -c -p -S -t -u -g -k -O -G"},
{"[", "-e -f -d -r -w -x -s -z -n -L -h -b -c -p -S -t -u -g -k -O -G"},
{"alias", "-p"}, {"unalias", "-a"}, {"hash", "-r -d -l -p -t"},
{"printf", "-v"}, {"mapfile", "-t -n -O -s -C"}, {"jobs", "-l -p"},
{"dirs", "-c -l -p -v"}, {"pushd", "-n"}, {"popd", "-n"},
{"exec", "-a -c -l"}, {"readonly", "-p -a -A -f"}, {"getopts", ""},
{NULL, NULL}};

static const char	*g_git_opts[] = {"--version", "--help", "-C", "-c",
	"--exec-path", "--html-path", "--paginate", "--no-pager", "--git-dir",
	"--work-tree", "--namespace", "--bare", "--no-replace-objects", NULL};

static void	list_add(t_strs *l, const char *item, size_t n)
{
	char	**bigger;
	int		i;

	i = 0;
	while (i < l->n)
	{
		if (ft_strlen(l->v[i]) == n && ft_strncmp(l->v[i], item, n) == 0)
			return ;
		i++;
	}
	if (l->n + 2 > l->cap)
	{
		l->cap = l->cap ? l->cap * 2 : 32;
		bigger = ft_calloc(l->cap, sizeof(char *));
		if (!bigger)
			return ;
		i = -1;
		while (++i < l->n)
			bigger[i] = l->v[i];
		free(l->v);
		l->v = bigger;
	}
	l->v[l->n] = ft_substr(item, 0, n);
	if (l->v[l->n])
		l->n++;
}

static void	list_free(t_strs *l)
{
	while (l->n > 0)
		free(l->v[--l->n]);
	free(l->v);
	l->v = NULL;
	l->cap = 0;
}

static void	cache_free(void)
{
	while (g_ncache > 0)
	{
		g_ncache--;
		free(g_cache[g_ncache].key);
		list_free(&g_cache[g_ncache].opts);
		list_free(&g_cache[g_ncache].subs);
	}
}

static long	now_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void	child_exec(char *path, char **argv, char **envp, int out)
{
	int	nul;

	setpgid(0, 0);
	nul = open("/dev/null", O_RDONLY);
	dup2(nul, STDIN_FILENO);
	dup2(out, STDOUT_FILENO);
	dup2(out, STDERR_FILENO);
	signal(SIGINT, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	environ = envp;
	setenv("LC_ALL", "C", 1);
	setenv("TERM", "dumb", 1);
	setenv("PAGER", "cat", 1);
	setenv("GIT_PAGER", "cat", 1);
	execv(path, argv);
	_exit(127);
}

static char	*read_pipe(int fd, long deadline)
{
	char	*buf;
	size_t	len;
	ssize_t	got;
	long	left;

	buf = ft_calloc(HELP_MAX + 1, 1);
	len = 0;
	left = deadline - now_ms();
	while (buf && len < HELP_MAX && left > 0)
	{
		if (poll(&(struct pollfd){fd, POLLIN, 0}, 1, (int)left) <= 0)
			break ;
		got = read(fd, buf + len, HELP_MAX - len);
		if (got <= 0)
			break ;
		len += got;
		left = deadline - now_ms();
	}
	return (buf);
}

static char	*capture(char *path, char **argv, t_shell *shell)
{
	int		fds[2];
	pid_t	pid;
	char	**envp;
	char	*text;

	if (pipe(fds) == -1)
		return (NULL);
	envp = env_to_array(shell->env);
	pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		child_exec(path, argv, envp, fds[1]);
	}
	close(fds[1]);
	free_str_array(envp);
	text = pid > 0 ? read_pipe(fds[0], now_ms() + HELP_MS) : NULL;
	close(fds[0]);
	if (pid > 0)
	{
		kill(-pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
	return (text);
}

static int	opt_char(char c)
{
	return (ft_isalnum(c) || c == '-' || c == '_' || c == '.' || c == '+');
}

static void	add_option(t_strs *l, const char *tok, size_t n)
{
	if (n >= 2 && tok[0] == '-' && (n > 2 || tok[1] != '-'))
		list_add(l, tok, n);
}

static const char	*scan_option(t_strs *l, const char *s)
{
	size_t	n;
	size_t	m;
	char	buf[160];

	n = 0;
	while (opt_char(s[n]))
		n++;
	if (s[n] == '[' && ft_strncmp(s + n, "[no-]", 5) == 0 && n == 2)
	{
		m = 0;
		while (opt_char(s[n + 5 + m]) && m < 100)
			m++;
		ft_memcpy(buf, "--", 2);
		ft_memcpy(buf + 2, s + n + 5, m);
		list_add(l, buf, m + 2);
		ft_memcpy(buf, "--no-", 5);
		ft_memcpy(buf + 5, s + n + 5, m);
		list_add(l, buf, m + 5);
		return (s + n + 5 + m);
	}
	add_option(l, s, n);
	return (s + n);
}

static void	parse_option_line(t_strs *l, const char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	while (*s == '-')
	{
		s = scan_option(l, s);
		while (*s && *s != ',' && *s != ' ' && *s != '\t' && *s != '\n')
			s++;
		if (*s == ',')
			s++;
		while (*s == ' ')
			s++;
	}
}

static void	parse_options(t_strs *l, const char *text)
{
	const char	*line;

	line = text;
	while (*line)
	{
		parse_option_line(l, line);
		line = ft_strchr(line, '\n');
		if (!line)
			break ;
		line++;
	}
}

static int	is_heading(const char *line, size_t len)
{
	size_t	i;
	size_t	j;

	if (len == 0 || line[0] == ' ' || line[0] == '\t' || line[len - 1] != ':')
		return (0);
	i = 0;
	while (i + 7 <= len)
	{
		j = 0;
		while (j < 6 && ft_tolower(line[i + 1 + j]) == "ommand"[j])
			j++;
		if (j == 6 && (line[i] == 'C' || line[i] == 'c'))
			return (1);
		i++;
	}
	return (0);
}

static void	parse_subline(t_strs *l, const char *s)
{
	size_t	n;

	while (*s == ' ' || *s == '\t')
		s++;
	while (1)
	{
		n = 0;
		while (ft_isalnum(s[n]) || s[n] == '-' || s[n] == '_' || s[n] == '.')
			n++;
		if (n == 0 || !ft_isalnum(s[0]))
			return ;
		if (s[n] == '*')
			;
		else if (s[n] && s[n] != ' ' && s[n] != ',' && s[n] != '\n')
			return ;
		list_add(l, s, n);
		s += n + (s[n] == '*');
		if (*s != ',')
			return ;
		s++;
		while (*s == ' ')
			s++;
	}
}

static void	parse_subcommands(t_strs *l, const char *text)
{
	const char	*line;
	const char	*end;
	int			in_section;

	in_section = 0;
	line = text;
	while (*line)
	{
		end = ft_strchr(line, '\n');
		if (!end)
			end = line + ft_strlen(line);
		if (is_heading(line, end - line))
			in_section = 1;
		else if (in_section && *line && *line != ' ' && *line != '\t')
			in_section = 0;
		else if (in_section && (*line == ' ' || *line == '\t'))
			parse_subline(l, line);
		if (!*end)
			break ;
		line = end + 1;
	}
}

static int	in_list(const t_strs *l, const char *word)
{
	int	i;

	i = 0;
	while (i < l->n)
		if (ft_strcmp(l->v[i++], word) == 0)
			return (1);
	return (0);
}

static char	*base_name(char *cmd)
{
	char	*slash;

	slash = ft_strrchr(cmd, '/');
	return (slash ? slash + 1 : cmd);
}

static void	probe(t_entry *e, char **words, int nw, t_shell *shell)
{
	char	*path;
	char	*argv[nw + 2];
	char	*text;
	int		i;
	int		git;

	path = find_path(words[0], shell);
	git = ft_strcmp(base_name(words[0]), "git") == 0;
	i = -1;
	while (path && ++i < nw)
		argv[i] = words[i];
	argv[nw + 1] = NULL;
	argv[nw] = git ? "-h" : "--help";
	text = path ? capture(path, argv, shell) : NULL;
	if (text && nw == 1 && !git)
		parse_subcommands(&e->subs, text);
	if (text)
		parse_options(&e->opts, text);
	free(text);
	if (path && !e->opts.n && !git)
	{
		argv[nw] = "-h";
		text = capture(path, argv, shell);
		parse_options(&e->opts, text ? text : "");
		free(text);
	}
	free(path);
}

static void	git_subcommands(t_entry *e, t_shell *shell)
{
	char	*path;
	char	*argv[3];
	char	*text;
	char	*line;
	char	*end;

	path = find_path("git", shell);
	argv[0] = "git";
	argv[1] = "--list-cmds=main,others,alias,nohelpers";
	argv[2] = NULL;
	text = path ? capture(path, argv, shell) : NULL;
	line = text;
	while (line && *line)
	{
		end = ft_strchr(line, '\n');
		if (!end)
			end = line + ft_strlen(line);
		if (end > line)
			list_add(&e->subs, line, end - line);
		line = *end ? end + 1 : end;
	}
	free(text);
	free(path);
}

static t_entry	*get_entry(char **words, int nw, t_shell *shell)
{
	t_entry	*e;
	char	*key;
	char	*tmp;
	int		i;

	key = ft_strdup(words[0]);
	i = 0;
	while (++i < nw)
	{
		tmp = ft_strjoin(key, " ");
		free(key);
		key = ft_strjoin(tmp, words[i]);
		free(tmp);
	}
	i = -1;
	while (++i < g_ncache)
		if (ft_strcmp(g_cache[i].key, key) == 0)
			return (free(key), &g_cache[i]);
	if (g_ncache >= CACHE_MAX)
		cache_free();
	e = &g_cache[g_ncache++];
	ft_bzero(e, sizeof(*e));
	e->key = key;
	if (ft_strcmp(base_name(words[0]), "git") == 0 && nw == 1)
		git_subcommands(e, shell);
	else
		probe(e, words, nw, shell);
	return (e);
}

static char	**filter(const t_strs *l, const char *text)
{
	char	**out;
	int		i;
	int		n;

	out = ft_calloc(l->n + 1, sizeof(char *));
	i = 0;
	n = 0;
	while (out && i < l->n)
	{
		if (ft_strncmp(l->v[i], text, ft_strlen(text)) == 0)
			out[n++] = ft_strdup(l->v[i]);
		i++;
	}
	if (out && n == 0)
		return (free(out), NULL);
	return (out);
}

static int	make_name_ok(const char *s, size_t n)
{
	size_t	i;

	if (n == 0 || s[0] == '.' || s[0] == '-')
		return (0);
	i = 0;
	while (i < n)
		if (ft_strchr("%$=()#\t", s[i++]))
			return (0);
	return (1);
}

static void	make_line(t_strs *l, const char *line)
{
	const char	*colon;
	const char	*eq;
	size_t		n;

	if (*line == '\t' || *line == ' ' || *line == '#' || *line == '.'
		|| *line == '\n')
		return ;
	colon = ft_strchr(line, ':');
	eq = ft_strchr(line, '=');
	if (!colon || (eq && eq < colon) || colon[1] == '=')
		return ;
	while (line < colon)
	{
		while (line < colon && *line == ' ')
			line++;
		n = 0;
		while (line + n < colon && line[n] != ' ')
			n++;
		if (make_name_ok(line, n))
			list_add(l, line, n);
		line += n;
	}
}

static char	*read_text(const char *path)
{
	char	*buf;
	ssize_t	got;
	int		fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (NULL);
	buf = ft_calloc(HELP_MAX + 1, 1);
	got = buf ? read(fd, buf, HELP_MAX) : -1;
	close(fd);
	if (got < 0)
		return (free(buf), NULL);
	return (buf);
}

static char	**make_targets(const char *text)
{
	static const char	*names[] = {"Makefile", "makefile", "GNUmakefile", NULL};
	t_strs				l;
	char				*body;
	char				*line;
	char				**out;
	int					i;

	ft_bzero(&l, sizeof(l));
	i = 0;
	body = NULL;
	while (names[i] && !body)
		body = read_text(names[i++]);
	line = body;
	while (line && *line)
	{
		make_line(&l, line);
		line = ft_strchr(line, '\n');
		line = line ? line + 1 : NULL;
	}
	free(body);
	out = filter(&l, text);
	list_free(&l);
	return (out);
}

static char	**builtin_options(const char *cmd, const char *text)
{
	t_strs	l;
	char	**out;
	int		i;
	size_t	n;
	const char	*s;

	i = 0;
	while (g_builtin_opts[i][0] && ft_strcmp(g_builtin_opts[i][0], cmd) != 0)
		i++;
	if (!g_builtin_opts[i][0])
		return (NULL);
	ft_bzero(&l, sizeof(l));
	s = g_builtin_opts[i][1];
	while (*s)
	{
		n = 0;
		while (s[n] && s[n] != ' ')
			n++;
		list_add(&l, s, n);
		s += n + (s[n] == ' ');
	}
	out = filter(&l, text);
	list_free(&l);
	return (out);
}

static int	is_tool(const char *cmd)
{
	int	i;

	i = 0;
	while (g_tools[i])
		if (ft_strcmp(g_tools[i++], base_name((char *)cmd)) == 0)
			return (1);
	return (0);
}

static char	**git_root_options(const char *text)
{
	t_strs	l;
	char	**out;
	int		i;

	ft_bzero(&l, sizeof(l));
	i = 0;
	while (g_git_opts[i])
		list_add(&l, g_git_opts[i], ft_strlen(g_git_opts[i])), i++;
	out = filter(&l, text);
	list_free(&l);
	return (out);
}

static t_entry	*subcommand_entry(char **words, int nw, t_shell *shell)
{
	t_entry	*root;

	root = get_entry(words, 1, shell);
	if (nw >= 2 && words[1][0] != '-' && in_list(&root->subs, words[1]))
		return (get_entry(words, 2, shell));
	return (root);
}

char	**opts_candidates(char **words, int nw, const char *text,
			t_shell *shell)
{
	t_entry	*e;
	int		i;

	i = 0;
	while (i < nw)
		if (ft_strcmp(words[i++], "--") == 0)
			return (NULL);
	if (nw < 1)
		return (NULL);
	if (!g_registered && ++g_registered)
		atexit(cache_free);
	if (ft_strcmp(words[0], "make") == 0 && text[0] != '-')
		return (make_targets(text));
	if (is_builtin(words[0]) || find_func(shell, words[0]))
		return (text[0] == '-' ? builtin_options(words[0], text) : NULL);
	if (text[0] != '-' && !(nw == 1 && is_tool(words[0])))
		return (NULL);
	if (text[0] == '-' && nw == 1 && ft_strcmp(base_name(words[0]), "git") == 0)
		return (git_root_options(text));
	if (text[0] != '-')
		return (filter(&get_entry(words, 1, shell)->subs, text));
	e = subcommand_entry(words, nw, shell);
	return (filter(&e->opts, text));
}
