#include "../includes/builtins.h"
#include <fcntl.h>
#include "../includes/output.h"

int	stdout_failed(const char *name)
{
	if (fcntl(STDOUT_FILENO, F_GETFD) != -1)
		return (0);
	ml_put_all(2, "minishell: ", name, ": write error: Bad file descriptor\n",
		NULL);
	return (1);
}

static int	echo_flags(const char *arg, int *newline, int *escapes)
{
	int	i;

	if (!arg || arg[0] != '-' || !arg[1])
		return (0);
	i = 1;
	while (arg[i] == 'n' || arg[i] == 'e' || arg[i] == 'E')
		i++;
	if (arg[i])
		return (0);
	i = 1;
	while (arg[i])
	{
		if (arg[i] == 'n')
			*newline = 0;
		else
			*escapes = arg[i] == 'e';
		i++;
	}
	return (1);
}

static int	digit_value(char c, int base)
{
	int	v;

	if (ft_isdigit(c))
		v = c - '0';
	else if ((c | 32) >= 'a' && (c | 32) <= 'f')
		v = (c | 32) - 'a' + 10;
	else
		return (-1);
	return (v < base ? v : -1);
}

static int	octal_or_hex(const char *s, char *out)
{
	int	base;
	int	max;
	int	n;
	int	v;

	if (s[0] != 'x' && s[0] != '0')
		return (0);
	base = s[0] == 'x' ? 16 : 8;
	max = base == 16 ? 2 : 3;
	n = 1;
	v = 0;
	while (n - 1 < max && digit_value(s[n], base) >= 0)
		v = v * base + digit_value(s[n++], base);
	if (base == 16 && n == 1)
		return (0);
	*out = (char)v;
	return (n);
}

static int	put_escape(const char *s, int *stop)
{
	static const char	map[] = "a\ab\bf\fn\nr\rt\tv\v\\\\e\033";
	char				c;
	int					n;
	int					k;

	k = 0;
	while (map[k] && map[k] != s[0])
		k += 2;
	if (map[k])
		return (ft_putchar_fd(map[k + 1], STDOUT_FILENO), 1);
	if (s[0] == 'c')
		return (*stop = 1, 1);
	n = octal_or_hex(s, &c);
	if (n)
	{
		ft_putchar_fd(c, STDOUT_FILENO);
		return (n);
	}
	ft_putchar_fd('\\', STDOUT_FILENO);
	return (0);
}

static void	put_arg(const char *s, int escapes, int *stop)
{
	int	i;

	i = 0;
	while (s[i] && !*stop)
	{
		if (escapes && s[i] == '\\' && s[i + 1])
			i += 1 + put_escape(s + i + 1, stop);
		else
			ft_putchar_fd(s[i++], STDOUT_FILENO);
	}
}

int	builtin_echo(char **argv)
{
	int	i;
	int	newline;
	int	escapes;
	int	stop;

	if (stdout_failed("echo"))
		return (1);
	i = 1;
	newline = 1;
	escapes = 0;
	stop = 0;
	while (echo_flags(argv[i], &newline, &escapes))
		i++;
	while (argv[i] && !stop)
	{
		put_arg(argv[i], escapes, &stop);
		if (argv[i + 1] && !stop)
			ft_putchar_fd(' ', STDOUT_FILENO);
		i++;
	}
	if (newline && !stop)
		ft_putchar_fd('\n', STDOUT_FILENO);
	return (0);
}
