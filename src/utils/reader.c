#include "../../includes/reader.h"
#include "../../libft/include/libft.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void	reader_init(t_reader *reader, int fd)
{
	reader->fd = fd;
	reader->pos = 0;
	reader->len = 0;
}

t_reader	*stdin_reader(void)
{
	static t_reader	reader = {STDIN_FILENO, 0, 0, {0}};

	return (&reader);
}

static int	next_char(t_reader *reader)
{
	ssize_t	n;

	while (reader->pos >= reader->len)
	{
		n = read(reader->fd, reader->buf, READER_BUF);
		if (n < 0 && errno == EINTR)
			continue ;
		if (n <= 0)
			return (-1);
		reader->pos = 0;
		reader->len = n;
	}
	return ((unsigned char)reader->buf[reader->pos++]);
}

static char	*grow(char *line, size_t len, size_t *cap)
{
	char	*bigger;
	size_t	i;

	*cap = *cap ? *cap * 2 : 128;
	bigger = malloc(*cap);
	i = 0;
	while (bigger && i < len)
	{
		bigger[i] = line[i];
		i++;
	}
	free(line);
	return (bigger);
}

char	*reader_line(t_reader *reader)
{
	char	*line;
	size_t	len;
	size_t	cap;
	int		c;

	line = NULL;
	len = 0;
	cap = 0;
	c = next_char(reader);
	if (c == -1)
		return (NULL);
	while (c != -1 && c != '\n')
	{
		if (len + 2 > cap)
			line = grow(line, len, &cap);
		if (!line)
			return (NULL);
		line[len++] = c;
		c = next_char(reader);
	}
	if (!line)
		line = grow(line, 0, &cap);
	if (line)
		line[len] = '\0';
	return (line);
}

char	*reader_command(t_reader *reader, int (*incomplete)(char *))
{
	char	*line;
	char	*next;
	char	*joined;
	size_t	len;

	line = reader_line(reader);
	while (line && incomplete(line))
	{
		next = reader_line(reader);
		if (!next)
			break ;
		len = ft_strlen(line);
		joined = malloc(len + ft_strlen(next) + 2);
		if (!joined)
			return (free(next), line);
		ft_memcpy(joined, line, len);
		joined[len] = '\n';
		ft_memcpy(joined + len + 1, next, ft_strlen(next) + 1);
		free(line);
		free(next);
		line = joined;
	}
	return (line);
}
