#include "../../includes/lineedit.h"
#include "../../includes/builtins.h"
#include <dirent.h>
#include <sys/stat.h>

typedef struct s_matches
{
	char	**items;
	int		count;
	int		cap;
}	t_matches;

typedef enum e_kind
{
	KIND_COMMAND,
	KIND_VARIABLE,
	KIND_FILE
}	t_kind;

static void	add_match(t_matches *m, char *item)
{
	char	**bigger;
	int		i;

	if (!item)
		return ;
	if (m->count + 1 > m->cap)
	{
		m->cap = m->cap ? m->cap * 2 : 64;
		bigger = malloc(sizeof(char *) * m->cap);
		if (!bigger)
			return (free(item));
		i = 0;
		while (i < m->count)
		{
			bigger[i] = m->items[i];
			i++;
		}
		free(m->items);
		m->items = bigger;
	}
	m->items[m->count++] = item;
}

static void	free_matches(t_matches *m)
{
	while (m->count > 0)
		free(m->items[--m->count]);
	free(m->items);
	m->items = NULL;
}

static void	unique_matches(t_matches *m)
{
	int	i;
	int	j;

	ml_sort_strings(m->items, m->count);
	i = 0;
	j = 0;
	while (i < m->count)
	{
		if (j > 0 && ft_strcmp(m->items[j - 1], m->items[i]) == 0)
			free(m->items[i]);
		else
			m->items[j++] = m->items[i];
		i++;
	}
	m->count = j;
}

static int	has_prefix(const char *name, const char *text)
{
	return (ft_strncmp(name, text, ft_strlen(text)) == 0);
}

static void	scan_path_dir(t_matches *m, char *dir, const char *text)
{
	DIR				*d;
	struct dirent	*entry;
	char			*prefix;
	char			*full;

	d = opendir(dir);
	prefix = ft_strjoin(dir, "/");
	entry = d ? readdir(d) : NULL;
	while (entry)
	{
		full = NULL;
		if (has_prefix(entry->d_name, text) && entry->d_type != DT_DIR
			&& (entry->d_name[0] != '.' || text[0] == '.'))
			full = ft_strjoin(prefix, entry->d_name);
		if (full && access(full, X_OK) == 0)
			add_match(m, ft_strdup(entry->d_name));
		free(full);
		entry = readdir(d);
	}
	free(prefix);
	if (d)
		closedir(d);
}

static void	command_matches(t_matches *m, const char *text, t_shell *shell)
{
	const char	**names;
	t_env		*alias;
	char		**dirs;
	int			i;

	names = builtin_names();
	i = 0;
	while (names[i])
	{
		if (has_prefix(names[i], text))
			add_match(m, ft_strdup(names[i]));
		i++;
	}
	alias = shell->aliases;
	while (alias)
	{
		if (has_prefix(alias->key, text))
			add_match(m, ft_strdup(alias->key));
		alias = alias->next;
	}
	dirs = ft_split(env_get(shell->env, "PATH") ? env_get(shell->env, "PATH")
			: "", ':');
	i = 0;
	while (dirs && dirs[i])
		scan_path_dir(m, dirs[i++], text);
	free_str_array(dirs);
}

static void	variable_matches(t_matches *m, const char *text, t_shell *shell)
{
	t_env	*env;

	env = shell->env;
	while (env)
	{
		if (has_prefix(env->key, text))
			add_match(m, ft_strdup(env->key));
		env = env->next;
	}
}

static int	is_directory(const char *dir, struct dirent *entry)
{
	struct stat	st;
	char		*full;
	char		*prefix;
	int			result;

	if (entry->d_type == DT_DIR)
		return (1);
	if (entry->d_type != DT_LNK && entry->d_type != DT_UNKNOWN)
		return (0);
	prefix = ft_strjoin(dir, "/");
	full = ft_strjoin(prefix, entry->d_name);
	result = full && stat(full, &st) == 0 && S_ISDIR(st.st_mode);
	free(prefix);
	free(full);
	return (result);
}

static char	*dir_path(const char *dirpart, t_shell *shell)
{
	char	*home;

	if (!*dirpart)
		return (ft_strdup("."));
	home = env_get(shell->env, "HOME");
	if (home && ft_strncmp(dirpart, "~/", 2) == 0)
		return (ft_strjoin(home, dirpart + 1));
	return (ft_strdup(dirpart));
}

static char	*join_entry(const char *dirpart, const char *name, int is_dir)
{
	char	*first;
	char	*second;

	first = ft_strjoin(dirpart, name);
	second = is_dir ? ft_strjoin(first, "/") : first;
	if (is_dir)
		free(first);
	return (second);
}

static char	*wrap_text(char *text, const char *left, const char *right)
{
	char	*first;
	char	*second;

	first = ft_strjoin(left, text);
	second = ft_strjoin(first, right);
	free(first);
	free(text);
	return (second);
}

static void	file_matches(t_matches *m, const char *text, t_shell *shell,
				int dirs_only)
{
	char			*slash;
	char			*dirpart;
	char			*path;
	DIR				*d;
	struct dirent	*entry;

	slash = ft_strrchr(text, '/');
	dirpart = slash ? ft_substr(text, 0, slash - text + 1) : ft_strdup("");
	path = dir_path(dirpart, shell);
	d = path ? opendir(path) : NULL;
	entry = d ? readdir(d) : NULL;
	while (entry)
	{
		if (has_prefix(entry->d_name, slash ? slash + 1 : text)
			&& (entry->d_name[0] != '.' || (slash ? slash[1] : text[0]) == '.')
			&& ft_strcmp(entry->d_name, ".") && ft_strcmp(entry->d_name, "..")
			&& (!dirs_only || is_directory(path, entry)))
			add_match(m, join_entry(dirpart, entry->d_name,
					is_directory(path, entry)));
		entry = readdir(d);
	}
	if (d)
		closedir(d);
	free(path);
	free(dirpart);
}


static int	is_command_position(t_edit *e, size_t start)
{
	int	i;

	i = (int)start - 1;
	if (i >= 0 && (e->buf[i] == '"' || e->buf[i] == '\''))
		i--;
	while (i >= 0 && e->buf[i] == ' ')
		i--;
	return (i < 0 || ft_strchr("|&;(", e->buf[i]) != NULL);
}

static int	only_directories(t_edit *e)
{
	size_t	i;

	i = 0;
	while (i < e->len && e->buf[i] == ' ')
		i++;
	return (ft_strncmp(e->buf + i, "cd ", 3) == 0);
}

static t_kind	kind_of(t_edit *e, size_t start, const char *text, int *brace)
{
	*brace = (start >= 2 && e->buf[start - 1] == '{'
			&& e->buf[start - 2] == '$');
	if ((start > 0 && e->buf[start - 1] == '$') || *brace)
		return (KIND_VARIABLE);
	if (!ft_strchr(text, '/') && is_command_position(e, start))
		return (KIND_COMMAND);
	return (KIND_FILE);
}

static size_t	common_prefix(t_matches *m)
{
	size_t	len;
	int		i;

	len = ft_strlen(m->items[0]);
	i = 1;
	while (i < m->count)
	{
		while (len > 0 && ft_strncmp(m->items[0], m->items[i], len) != 0)
			len--;
		i++;
	}
	return (len);
}

static int	needs_quote(const char *s, size_t len)
{
	size_t	i;

	i = 0;
	while (i < len)
	{
		if (ft_strchr(" \t\"$&|;<>()*?[]{}#~\\`", s[i]))
			return (1);
		if (s[i] == '\'')
			return (0);
		i++;
	}
	return (0);
}

static void	print_list(t_edit *e, t_matches *m)
{
	t_out	o;
	int		width;
	int		per_row;
	int		rows;
	int		i;
	int		r;

	o.s = NULL;
	o.len = 0;
	o.cap = 0;
	width = 0;
	i = 0;
	while (i < m->count)
		width = (int)ft_strlen(m->items[i++]) > width
			? (int)ft_strlen(m->items[i - 1]) : width;
	width += 2;
	per_row = term_cols() / width;
	per_row = per_row < 1 ? 1 : per_row;
	rows = (m->count + per_row - 1) / per_row;
	out_put(&o, "\r\n", 2);
	r = 0;
	while (r < rows)
	{
		i = r;
		while (i < m->count)
		{
			out_put(&o, m->items[i], ft_strlen(m->items[i]));
			if (i + rows < m->count)
				out_put(&o, "                                                  ",
					width - (int)ft_strlen(m->items[i]));
			i += rows;
		}
		out_put(&o, "\r\n", 2);
		r++;
	}
	write(STDOUT_FILENO, o.s, o.len);
	free(o.s);
	e->cur_row = 0;
}

static void	apply_single(t_edit *e, size_t start, t_matches *m, t_kind kind,
				int brace)
{
	char	*core;
	char	*text;
	char	*tail;
	size_t	len;
	char	quote;

	core = m->items[0];
	len = ft_strlen(core);
	quote = (start > 0 && (e->buf[start - 1] == '"'
				|| e->buf[start - 1] == '\'')) ? e->buf[start - 1] : 0;
	tail = " ";
	if (kind == KIND_FILE && core[len - 1] == '/')
		tail = "/";
	else if (brace)
		tail = "}";
	if (tail[0] == '/')
		len--;
	text = ft_substr(core, 0, len);
	if (kind == KIND_FILE && !quote && needs_quote(text, len))
		text = wrap_text(text, "'", "'");
	else if (quote && tail[0] == ' ')
		tail = quote == '"' ? "\" " : "' ";
	le_replace(e, start, e->pos, text);
	le_insert(e, tail, ft_strlen(tail));
	free(text);
}

static void	complete_run(t_edit *e, size_t start, const char *text,
				t_matches *m)
{
	t_kind	kind;
	int		brace;
	size_t	prefix;
	char	*part;

	kind = kind_of(e, start, text, &brace);
	if (kind == KIND_COMMAND)
		command_matches(m, text, e->shell);
	else if (kind == KIND_VARIABLE)
		variable_matches(m, text, e->shell);
	else
		file_matches(m, text, e->shell, only_directories(e));
	unique_matches(m);
	if (m->count == 1)
		return (apply_single(e, start, m, kind, brace));
	if (m->count == 0)
		return ;
	prefix = common_prefix(m);
	if (prefix <= ft_strlen(text))
		return (print_list(e, m));
	part = ft_substr(m->items[0], 0, prefix);
	if (kind == KIND_FILE && needs_quote(part, prefix)
		&& !(start > 0 && (e->buf[start - 1] == '"'
				|| e->buf[start - 1] == '\'')))
		part = wrap_text(part, "'", "");
	le_replace(e, start, e->pos, part);
	free(part);
}

int	complete_tab(t_edit *e)
{
	t_matches	m;
	size_t		start;
	char		*text;

	m.items = NULL;
	m.count = 0;
	m.cap = 0;
	start = e->pos;
	while (start > 0 && !ft_strchr(" \t\"'`@$><=;|&{(", e->buf[start - 1]))
		start--;
	text = ft_substr(e->buf, start, e->pos - start);
	if (text)
		complete_run(e, start, text, &m);
	free_matches(&m);
	free(text);
	return (1);
}
