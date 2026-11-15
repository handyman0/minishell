#include "../../includes/lineedit.h"
#include "../../includes/builtins.h"
#include "../../includes/executor.h"

#define C_GREEN "\033[32m"
#define C_RED "\033[31m"
#define C_CYAN "\033[36m"
#define C_YELLOW "\033[33m"
#define C_MAGENTA "\033[35m"
#define C_RESET "\033[0m"

static void	put(t_out *out, const char *str, size_t n)
{
	out_put(out, str, n);
}

static void	put_color(t_out *out, const char *color)
{
	put(out, color, ft_strlen(color));
}

#define CACHE_SLOTS 16
#define CACHE_WORD 128

typedef struct s_cache
{
	char	word[CACHE_SLOTS][CACHE_WORD];
	char	ok[CACHE_SLOTS];
	int		used;
}	t_cache;

static t_cache	g_cache;

void	highlight_reset(void)
{
	g_cache.used = 0;
}

static int	lookup_command(const char *name, t_shell *shell)
{
	char	*path;
	int		ok;

	ok = is_builtin(name) || env_get(shell->aliases, name) != NULL;
	if (!ok)
	{
		path = find_executable(name, shell);
		ok = path != NULL;
		free(path);
	}
	return (ok);
}

static int	command_exists(const char *word, size_t len, t_shell *shell)
{
	char	name[CACHE_WORD];
	int		i;
	int		ok;

	if (len == 0 || len >= CACHE_WORD)
		return (0);
	ft_memcpy(name, word, len);
	name[len] = '\0';
	i = 0;
	while (i < g_cache.used)
	{
		if (ft_strcmp(g_cache.word[i], name) == 0)
			return (g_cache.ok[i]);
		i++;
	}
	ok = lookup_command(name, shell);
	if (g_cache.used < CACHE_SLOTS)
	{
		ft_strlcpy(g_cache.word[g_cache.used], name, CACHE_WORD);
		g_cache.ok[g_cache.used++] = ok;
	}
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

	out.s = NULL;
	out.cap = 0;
	out.len = 0;
	cmd = 1;
	redir = 0;
	i = 0;
	while (i < len)
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
	out_put(&out, "", 1);
	return (out.s);
}
