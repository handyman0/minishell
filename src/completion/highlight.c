#include "../../includes/completion.h"
#include "../../includes/builtins.h"
#include <sys/stat.h>

#define C_GREEN "\033[32m"
#define C_RED "\033[31m"
#define C_CYAN "\033[36m"
#define C_YELLOW "\033[33m"
#define C_MAGENTA "\033[35m"
#define C_RESET "\033[0m"

typedef struct s_out
{
	char	*buf;
	size_t	len;
}	t_out;

static void	put(t_out *out, const char *str, size_t n)
{
	ft_memcpy(out->buf + out->len, str, n);
	out->len += n;
}

static void	put_color(t_out *out, const char *color)
{
	put(out, color, ft_strlen(color));
}

static int	is_exec_file(const char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISREG(st.st_mode)
		&& access(path, X_OK) == 0);
}

static int	in_path(const char *name, t_shell *shell)
{
	char	**dirs;
	char	*full;
	char	*dir;
	int		i;
	int		found;

	dir = env_get(shell->env, "PATH");
	dirs = ft_split(dir ? dir : "", ':');
	found = 0;
	i = 0;
	while (dirs && dirs[i] && !found)
	{
		dir = ft_strjoin(dirs[i++], "/");
		full = ft_strjoin(dir, name);
		found = full && is_exec_file(full);
		free(dir);
		free(full);
	}
	free_str_array(dirs);
	return (found);
}

static int	command_exists(const char *word, size_t len, t_shell *shell)
{
	char	*name;
	int		ok;

	name = ft_substr(word, 0, len);
	ok = is_builtin(name);
	if (name && !ok && ft_strchr(name, '/'))
		ok = is_exec_file(name);
	else if (name && !ok && *name)
		ok = in_path(name, shell);
	free(name);
	return (ok);
}

static size_t	operator_len(const char *s)
{
	if ((s[0] == '|' && s[1] == '|') || (s[0] == '&' && s[1] == '&')
		|| (s[0] == '>' && s[1] == '>') || (s[0] == '<' && s[1] == '<'))
		return (2);
	if (ft_strchr("|;()<>", s[0]))
		return (1);
	return (0);
}

static size_t	word_len(const char *s, size_t max)
{
	size_t	i;
	char	quote;

	i = 0;
	quote = 0;
	while (i < max && s[i])
	{
		if (!quote && (ft_isspace(s[i]) || operator_len(s + i)))
			break ;
		if (!quote && (s[i] == '\'' || s[i] == '"'))
			quote = s[i];
		else if (quote && s[i] == quote)
			quote = 0;
		i++;
	}
	return (i);
}

static void	put_argument(t_out *out, const char *s, size_t len)
{
	size_t	i;
	char	quote;

	i = 0;
	quote = 0;
	while (i < len)
	{
		if ((s[i] == '\'' || s[i] == '"') && (!quote || quote == s[i]))
		{
			if (!quote)
				put_color(out, C_YELLOW);
			put(out, s + i, 1);
			if (quote)
				put_color(out, C_RESET);
			quote = (quote == 0) * s[i++];
		}
		else if (s[i] == '$' && quote != '\'' && i + 1 < len
			&& (ft_isalnum(s[i + 1]) || ft_strchr("?_", s[i + 1])))
		{
			put_color(out, C_MAGENTA);
			put(out, s + i++, 1);
			while (i < len && (ft_isalnum(s[i]) || s[i] == '_' || s[i] == '?'))
				put(out, s + i++, 1);
			put_color(out, quote ? C_YELLOW : C_RESET);
		}
		else
			put(out, s + i++, 1);
	}
	put_color(out, C_RESET);
}

static int	resets_command(const char *op, size_t len)
{
	return (len > 0 && (op[0] == '|' || op[0] == '&' || op[0] == ';'
			|| op[0] == '('));
}

char	*highlight_build(const char *line, size_t len, t_shell *shell)
{
	t_out	out;
	size_t	i;
	size_t	n;
	int		cmd;
	int		redir;

	out.buf = malloc(len * 24 + 64);
	out.len = 0;
	cmd = 1;
	redir = 0;
	i = 0;
	while (out.buf && i < len)
	{
		n = operator_len(line + i);
		if (ft_isspace(line[i]))
			put(&out, line + i++, 1);
		else if (n)
		{
			put_color(&out, C_CYAN);
			put(&out, line + i, n);
			put_color(&out, C_RESET);
			redir = (line[i] == '<' || line[i] == '>');
			cmd = resets_command(line + i, n) || (cmd && redir);
			i += n;
		}
		else
		{
			n = word_len(line + i, len - i);
			if (cmd && !redir)
			{
				put_color(&out, command_exists(line + i, n, shell)
					? C_GREEN : C_RED);
				put(&out, line + i, n);
				put_color(&out, C_RESET);
				cmd = 0;
			}
			else
				put_argument(&out, line + i, n);
			redir = 0;
			i += n;
		}
	}
	if (out.buf)
		out.buf[out.len] = '\0';
	return (out.buf);
}
