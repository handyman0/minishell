
#include "../include/mylib.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static size_t	total_length(va_list ap)
{
	const char	*part;
	size_t		total;

	total = 0;
	part = va_arg(ap, const char *);
	while (part)
	{
		total += ft_strlen(part);
		part = va_arg(ap, const char *);
	}
	return (total);
}

static void	flush(int fd, const char *buf, size_t len)
{
	ssize_t	n;

	while (len > 0)
	{
		n = write(fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue ;
		if (n <= 0)
			return ;
		buf += n;
		len -= n;
	}
}

void	ml_put_all(int fd, ...)
{
	va_list		ap;
	va_list		copy;
	const char	*part;
	char		*buf;
	size_t		len;

	va_start(ap, fd);
	va_copy(copy, ap);
	len = total_length(copy);
	va_end(copy);
	buf = malloc(len + 1);
	if (buf)
	{
		len = 0;
		part = va_arg(ap, const char *);
		while (part)
		{
			ft_memcpy(buf + len, part, ft_strlen(part));
			len += ft_strlen(part);
			part = va_arg(ap, const char *);
		}
		flush(fd, buf, len);
		free(buf);
	}
	va_end(ap);
}

