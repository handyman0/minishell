#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <sys/resource.h>

static void	put_time(char *buf, struct timeval tv)
{
	char	*min;
	char	*sec;
	int		ms;
	size_t	n;

	min = ft_itoa((int)(tv.tv_sec / 60));
	sec = ft_itoa((int)(tv.tv_sec % 60));
	ms = (int)(tv.tv_usec / 1000);
	n = 0;
	ft_memcpy(buf + n, min, ft_strlen(min));
	n += ft_strlen(min);
	buf[n++] = 'm';
	ft_memcpy(buf + n, sec, ft_strlen(sec));
	n += ft_strlen(sec);
	buf[n++] = '.';
	buf[n++] = '0' + ms / 100;
	buf[n++] = '0' + (ms / 10) % 10;
	buf[n++] = '0' + ms % 10;
	buf[n++] = 's';
	buf[n] = '\0';
	free(min);
	free(sec);
}

int	builtin_times(char **argv, t_shell *shell)
{
	struct rusage	self;
	struct rusage	kids;
	char			a[32];
	char			b[32];
	char			c[32];
	char			d[32];

	(void)argv;
	(void)shell;
	getrusage(RUSAGE_SELF, &self);
	getrusage(RUSAGE_CHILDREN, &kids);
	put_time(a, self.ru_utime);
	put_time(b, self.ru_stime);
	put_time(c, kids.ru_utime);
	put_time(d, kids.ru_stime);
	ml_put_all(STDOUT_FILENO, a, " ", b, "\n", c, " ", d, "\n", NULL);
	return (0);
}
