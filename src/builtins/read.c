#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <errno.h>

static int	grow(char **line, size_t *cap)
{
	char	*bigger;

	bigger = ft_calloc(*cap * 2, 1);
	if (!bigger)
		return (0);
	ft_memcpy(bigger, *line, *cap);
	free(*line);
	*line = bigger;
	*cap *= 2;
	return (1);
}

static int	read_line(char **line)
{
	char	c;
	size_t	len;
	size_t	cap;
	ssize_t	n;

	len = 0;
	cap = 64;
	*line = ft_calloc(cap, 1);
	n = read(STDIN_FILENO, &c, 1);
	while (*line && n > 0 && c != '\n')
	{
		if (len + 2 > cap && !grow(line, &cap))
			return (0);
		(*line)[len++] = c;
		n = read(STDIN_FILENO, &c, 1);
	}
	return (n > 0 || len > 0);
}

static void	assign_fields(char **names, const char *line, t_shell *shell)
{
	int		i;
	size_t	len;
	char	*value;

	i = 0;
	while (names[i])
	{
		while (*line == ' ' || *line == '\t')
			line++;
		if (!names[i + 1])
			len = ft_strlen(line);
		else
		{
			len = 0;
			while (line[len] && line[len] != ' ' && line[len] != '\t')
				len++;
		}
		while (!names[i + 1] && len > 0 && (line[len - 1] == ' '
				|| line[len - 1] == '\t'))
			len--;
		value = ft_substr(line, 0, len);
		env_assign(&shell->env, names[i], value);
		free(value);
		line += len;
		i++;
	}
}

int	builtin_read(char **argv, t_shell *shell)
{
	char	*line;
	char	*def[2];
	char	**names;
	int		got;
	int		i;

	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
		i++;
	def[0] = "REPLY";
	def[1] = NULL;
	names = argv[i] ? argv + i : def;
	got = read_line(&line);
	if (!line)
		return (1);
	assign_fields(names, line, shell);
	free(line);
	return (got > 0 ? 0 : 1);
}
