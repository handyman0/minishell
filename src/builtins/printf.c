#include "../../includes/builtins.h"
#include "../../includes/output.h"
#include "../../includes/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct s_pf
{
	char	*buf;
	size_t	len;
	size_t	cap;
	int		status;
	int		stop;
}	t_pf;

static void	pf_put(t_pf *p, const char *s, size_t n)
{
	char	*grown;

	if (p->len + n + 1 > p->cap)
	{
		p->cap = (p->len + n + 1) * 2;
		grown = malloc(p->cap);
		if (!grown)
			return ;
		if (p->buf)
			ft_memcpy(grown, p->buf, p->len);
		free(p->buf);
		p->buf = grown;
	}
	ft_memcpy(p->buf + p->len, s, n);
	p->len += n;
	p->buf[p->len] = '\0';
}

static int	hexval(char c)
{
	if (ft_isdigit(c))
		return (c - '0');
	if ((c | 32) >= 'a' && (c | 32) <= 'f')
		return ((c | 32) - 'a' + 10);
	return (-1);
}

static int	escape(t_pf *p, const char *s, int in_b)
{
	static const char	map[] = "a\ab\bf\fn\nr\rt\tv\v\\\\\"\"'\'";
	char				c;
	int					n;
	int					k;

	k = 0;
	while (map[k] && map[k] != s[0])
		k += 2;
	if (map[k])
		return (pf_put(p, &map[k + 1], 1), 1);
	if (s[0] == 'c' && in_b)
		return (p->stop = 1, 1);
	n = 0;
	c = 0;
	if (s[0] == 'x' && hexval(s[1]) >= 0)
	{
		n = 1;
		while (n < 3 && hexval(s[n]) >= 0)
			c = c * 16 + hexval(s[n++]);
	}
	else if (s[0] >= '0' && s[0] <= '7')
		while (n < 3 + in_b && s[n] >= '0' && s[n] <= '7' && (n || !in_b || 1))
			c = c * 8 + (s[n++] - '0');
	if (n == 0)
		return (pf_put(p, "\\", 1), 0);
	return (pf_put(p, &c, 1), n);
}

static void	put_escaped(t_pf *p, const char *s, int in_b)
{
	int	i;

	i = 0;
	while (s[i] && !p->stop)
	{
		if (s[i] == '\\' && s[i + 1])
			i += 1 + escape(p, s + i + 1, in_b);
		else
			pf_put(p, s + i++, 1);
	}
}

static long	to_number(t_pf *p, const char *arg, const char *name)
{
	char	*end;
	long	v;

	if (!arg || !*arg)
		return (0);
	if ((arg[0] == '\'' || arg[0] == '"') && arg[1])
		return ((unsigned char)arg[1]);
	errno = 0;
	v = strtol(arg, &end, 0);
	if (*end || end == arg)
	{
		ml_put_all(2, "minishell: printf: ", arg, ": invalid number\n", NULL);
		p->status = 1;
		return (end == arg ? 0 : v);
	}
	(void)name;
	return (v);
}

static void	pf_format(t_pf *p, const char *spec, char conv, const char *arg)
{
	char	tmp[512];
	char	*big;
	int		n;

	if (conv == 'f' || conv == 'e' || conv == 'g' || conv == 'E' || conv == 'G'
		|| conv == 'F')
		n = snprintf(tmp, sizeof(tmp), spec, strtod(arg ? arg : "0", NULL));
	else if (conv == 's')
		n = snprintf(tmp, sizeof(tmp), spec, arg ? arg : "");
	else if (conv == 'c')
		n = snprintf(tmp, sizeof(tmp), spec, arg && *arg ? *arg : 0);
	else
		n = snprintf(tmp, sizeof(tmp), spec, to_number(p, arg, spec));
	if (n >= (int)sizeof(tmp))
	{
		big = malloc(n + 1);
		if (big && conv == 's')
			snprintf(big, n + 1, spec, arg ? arg : "");
		if (big)
			pf_put(p, big, n);
		free(big);
	}
	else if (n > 0)
		pf_put(p, tmp, n);
}

static int	read_spec(const char *f, char *spec, size_t max)
{
	size_t	n;

	n = 0;
	spec[0] = '%';
	while (f[n] && ft_strchr("-+ #0'", f[n]) && n < max - 5)
	{
		spec[n + 1] = f[n];
		n++;
	}
	while ((ft_isdigit(f[n]) || f[n] == '.') && n < max - 5)
	{
		spec[n + 1] = f[n];
		n++;
	}
	return ((int)n + 1);
}

static void	put_quoted(t_pf *p, const char *s)
{
	char	*q;

	q = param_quote(s);
	if (q)
		pf_put(p, q, ft_strlen(q));
	free(q);
}

static void	numeric_spec(char *spec, int n, char conv)
{
	if (ft_strchr("diouxX", conv))
		spec[n++] = 'l';
	spec[n++] = conv;
	spec[n] = '\0';
}

static int	conversion(t_pf *p, const char *f, char **args, int *used)
{
	char	spec[64];
	char	*arg;
	int		n;
	char	conv;

	n = read_spec(f, spec, sizeof(spec));
	conv = f[n - 1];
	arg = args[*used];
	if (!conv)
		return (pf_put(p, "%", 1), pf_put(p, f, n - 1), n - 1);
	if (conv == 'b')
		put_escaped(p, arg ? arg : "", 1);
	else if (conv == 'q')
		put_quoted(p, arg ? arg : "");
	else if (ft_strchr("diouxXcsfeEgGF", conv))
	{
		numeric_spec(spec, n, conv);
		pf_format(p, spec, conv, arg);
	}
	else
		return (pf_put(p, "%", 1), pf_put(p, f, n), n);
	if (arg)
		(*used)++;
	return (n);
}

static int	run_format(t_pf *p, const char *f, char **args, int *used)
{
	int	i;
	int	any;

	i = 0;
	any = 0;
	while (f[i] && !p->stop)
	{
		if (f[i] == '%' && f[i + 1] == '%')
			pf_put(p, "%", 1), i += 2;
		else if (f[i] == '%')
		{
			i += conversion(p, f + i + 1, args, used) + 1;
			any = 1;
		}
		else if (f[i] == '\\' && f[i + 1])
			i += 1 + escape(p, f + i + 1, 0);
		else
			pf_put(p, f + i++, 1);
	}
	return (any);
}

int	builtin_printf(char **argv, t_shell *shell)
{
	t_pf	p;
	char	*var;
	int		i;
	int		used;
	int		any;

	ft_bzero(&p, sizeof(p));
	var = NULL;
	i = 1;
	if (argv[i] && ft_strcmp(argv[i], "-v") == 0 && argv[i + 1])
		var = argv[i + 1];
	i += var ? 2 : 0;
	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	if (!argv[i])
		return (ml_put_all(2, "minishell: printf: usage: printf format "
				"[arguments]\n", NULL), 2);
	used = i + 1;
	any = run_format(&p, argv[i], argv, &used);
	while (any && argv[used] && !p.stop)
	{
		any = used;
		run_format(&p, argv[i], argv, &used);
		any = used != any;
	}
	if (var)
		env_assign(&shell->env, var, p.buf ? p.buf : "");
	else if (p.buf && !stdout_failed("printf"))
		write(STDOUT_FILENO, p.buf, p.len);
	return (free(p.buf), p.status);
}
