#ifndef READER_H
# define READER_H

# include <stddef.h>

# define READER_BUF 65536

typedef struct s_reader
{
	int		fd;
	size_t	pos;
	size_t	len;
	char	buf[READER_BUF];
}	t_reader;

void		reader_init(t_reader *reader, int fd);
char		*reader_line(t_reader *reader);
t_reader	*stdin_reader(void);
char		*reader_command(t_reader *reader, int (*incomplete)(char *));

#endif
