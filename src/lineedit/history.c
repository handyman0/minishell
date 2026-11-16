#include "../../includes/lineedit.h"
#include <fcntl.h>

static char	*g_hist[HISTORY_MAX];
static int	g_count = 0;

int	le_history_count(void)
{
	return (g_count);
}

const char	*le_history_get(int index)
{
	if (index < 0 || index >= g_count)
		return (NULL);
	return (g_hist[index]);
}

void	le_history_add(const char *line)
{
	int	i;

	if (!line || !*line)
		return ;
	if (g_count > 0 && ft_strcmp(g_hist[g_count - 1], line) == 0)
		return ;
	if (g_count == HISTORY_MAX)
	{
		free(g_hist[0]);
		i = 1;
		while (i < g_count)
		{
			g_hist[i - 1] = g_hist[i];
			i++;
		}
		g_count--;
	}
	g_hist[g_count] = ft_strdup(line);
	if (g_hist[g_count])
		g_count++;
}

void	le_history_load(const char *path)
{
	static t_reader	reader;
	char			*line;
	int				fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return ;
	ml_reader_init(&reader, fd);
	line = ml_reader_line(&reader);
	while (line)
	{
		le_history_add(line);
		free(line);
		line = ml_reader_line(&reader);
	}
	close(fd);
}

void	le_history_save(const char *path)
{
	int	fd;
	int	i;

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1)
		return ;
	i = 0;
	while (i < g_count)
	{
		write(fd, g_hist[i], ft_strlen(g_hist[i]));
		write(fd, "\n", 1);
		i++;
	}
	close(fd);
}

const char	*history_suggestion(const char *buf, size_t len)
{
	int	i;

	if (len == 0)
		return (NULL);
	i = g_count - 1;
	while (i >= 0)
	{
		if (ft_strlen(g_hist[i]) > len
			&& ft_strncmp(g_hist[i], buf, len) == 0)
			return (g_hist[i] + len);
		i--;
	}
	return (NULL);
}

void	le_history_clear(void)
{
	while (g_count > 0)
		free(g_hist[--g_count]);
}
