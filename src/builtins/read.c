#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <errno.h>

typedef struct s_rline
{
	char	*buf;
	char	*quoted;
	size_t	len;
	size_t	cap;
}	t_rline;

static int	push(t_rline *l, char c, char q)
{
	char	*bigger;

	if (l->len + 1 > l->cap)
	{
		l->cap = l->cap ? l->cap * 2 : 64;
		bigger = realloc(l->buf, l->cap);
		if (!bigger)
			return (0);
		l->buf = bigger;
		bigger = realloc(l->quoted, l->cap);
		if (!bigger)
			return (0);
		l->quoted = bigger;
	}
	l->buf[l->len] = c;
	l->quoted[l->len] = q;
	l->len++;
	return (1);
}

static int	read_line(t_rline *l, int raw)
{
	char	c;
	char	next;

	while (read(STDIN_FILENO, &c, 1) > 0)
	{
		if (c == '\n')
			return (1);
		if (c == '\\' && !raw)
		{
			if (read(STDIN_FILENO, &next, 1) <= 0)
				return (0);
			if (next == '\n')
				continue ;
			if (!push(l, next, 1))
				return (0);
			continue ;
		}
		if (!push(l, c, 0))
			return (0);
	}
	return (0);
}

static int	is_delim(t_rline *l, size_t p, const char *ifs)
{
	return (p < l->len && !l->quoted[p] && l->buf[p]
		&& ft_strchr(ifs, l->buf[p]));
}

static int	is_blank(t_rline *l, size_t p, const char *ifs)
{
	return (is_delim(l, p, ifs)
		&& (l->buf[p] == ' ' || l->buf[p] == '\t' || l->buf[p] == '\n'));
}

static size_t	skip_blanks(t_rline *l, size_t p, const char *ifs)
{
	while (is_blank(l, p, ifs))
		p++;
	return (p);
}

static size_t	next_field(t_rline *l, size_t p, const char *ifs)
{
	p = skip_blanks(l, p, ifs);
	if (is_delim(l, p, ifs))
		p = skip_blanks(l, p + 1, ifs);
	return (p);
}

static size_t	last_end(t_rline *l, size_t pos, const char *ifs)
{
	size_t	end;
	size_t	core;
	size_t	k;

	end = l->len;
	while (end > pos && is_blank(l, end - 1, ifs))
		end--;
	if (end <= pos || !is_delim(l, end - 1, ifs))
		return (end);
	core = end - 1;
	while (core > pos && is_blank(l, core - 1, ifs))
		core--;
	k = core;
	while (k > pos && !is_delim(l, k - 1, ifs))
		k--;
	if (k > pos)
		return (end);
	return (core);
}

static int	assign_fields(char **names, t_rline *l, t_shell *shell)
{
	const char	*ifs;
	size_t		pos;
	size_t		end;
	char		*value;
	int			i;
	int			bad;

	ifs = env_get(shell->env, "IFS");
	if (!ifs)
		ifs = " \t\n";
	pos = 0;
	bad = 0;
	i = -1;
	while (names[++i])
	{
		pos = skip_blanks(l, pos, ifs);
		end = pos;
		if (names[i + 1])
			while (end < l->len && !is_delim(l, end, ifs))
				end++;
		else
			end = last_end(l, pos, ifs);
		value = ft_substr(l->buf, pos, end - pos);
		if (env_is_readonly(shell->env, names[i]))
			bad = 1;
		env_assign(&shell->env, names[i], value ? value : "");
		if (shell->opts & OPT_A)
			env_export(shell->env, names[i]);
		free(value);
		pos = next_field(l, end, ifs);
	}
	return (bad);
}

static void	assign_array(char *name, t_rline *l, t_shell *shell)
{
	const char	*ifs;
	char		**el;
	size_t		pos;
	size_t		end;
	char		*joined;
	int			n;

	ifs = env_get(shell->env, "IFS");
	if (!ifs)
		ifs = " \t\n";
	el = ft_calloc(l->len + 2, sizeof(char *));
	n = 0;
	pos = skip_blanks(l, 0, ifs);
	while (el && pos < l->len)
	{
		end = pos;
		while (end < l->len && !is_delim(l, end, ifs))
			end++;
		el[n++] = ft_substr(l->buf, pos, end - pos);
		pos = next_field(l, end, ifs);
	}
	joined = el ? arr_join(el, n) : NULL;
	if (joined)
		env_assign(&shell->env, name, joined);
	free(joined);
	free_str_array(el);
}

static int	read_options(char **argv, int *raw, char **array, char **prompt)
{
	int	i;
	int	k;

	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0 && ++i)
			break ;
		k = 0;
		while (argv[i][++k])
		{
			*raw |= argv[i][k] == 'r';
			if (argv[i][k] == 'a' && argv[i + 1])
				*array = argv[++i];
			else if (argv[i][k] == 'p' && argv[i + 1])
				*prompt = argv[++i];
			else if ((argv[i][k] == 'n' || argv[i][k] == 't'
				|| argv[i][k] == 'd' || argv[i][k] == 'u') && argv[i + 1])
				i++;
		}
		i++;
	}
	return (i);
}

int	builtin_read(char **argv, t_shell *shell)
{
	t_rline	l;
	char	*def[2];
	char	**names;
	int		raw;
	int		i;
	int		got;
	char	*array;
	char	*prompt;

	raw = 0;
	array = NULL;
	prompt = NULL;
	i = read_options(argv, &raw, &array, &prompt);
	if (prompt && isatty(STDIN_FILENO))
		ml_put_all(2, prompt, NULL);
	def[0] = "REPLY";
	def[1] = NULL;
	names = argv[i] ? argv + i : def;
	l.buf = NULL;
	l.quoted = NULL;
	l.len = 0;
	l.cap = 0;
	got = read_line(&l, raw);
	push(&l, 0, 0);
	l.len--;
	i = 0;
	if (array)
		assign_array(array, &l, shell);
	else
		i = assign_fields(names, &l, shell);
	free(l.buf);
	free(l.quoted);
	if (i)
		return (2);
	return (got ? 0 : 1);
}
