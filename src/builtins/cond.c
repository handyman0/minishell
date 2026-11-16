#include "../../includes/builtins.h"
#include "../../includes/parser.h"
#include "../../includes/output.h"
#include <regex.h>

typedef struct s_cond
{
	char	**w;
	int		n;
	int		i;
	int		err;
	t_shell	*shell;
}	t_cond;

static int	cond_or(t_cond *c);

static int	is_op(const char *s, const char *list)
{
	char	*hit;
	int		n;

	n = ft_strlen(s);
	hit = ft_strnstr(list, s, ft_strlen(list));
	return (n > 0 && hit && (hit == list || hit[-1] == ' ')
		&& (hit[n] == ' ' || hit[n] == '\0'));
}

static int	is_binary_op(const char *s)
{
	return (is_op(s, "== != = =~ < > -eq -ne -lt -le -gt -ge -nt -ot -ef"));
}

static int	is_unary_op(const char *s)
{
	return (is_op(s, "-n -z -e -f -d -r -w -x -s -L -h -b -c -p -S -t -g -u -k "
			"-O -G -N -v"));
}

static char	*flat(t_cond *c, const char *raw)
{
	char	*res;

	res = expand_variables((char *)raw, c->shell);
	return (res ? res : ft_strdup(""));
}

static long	number(t_cond *c, const char *raw)
{
	char	*text;
	char	*res;
	long	v;

	text = flat(c, raw);
	res = arith_string(text, c->shell);
	v = res ? ft_atoi(res) : 0;
	return (free(text), free(res), v);
}

static int	regex_match(const char *text, const char *pat)
{
	regex_t	re;
	int		hit;

	if (regcomp(&re, pat, REG_EXTENDED) != 0)
		return (-1);
	hit = regexec(&re, text, 0, NULL, 0) == 0;
	regfree(&re);
	return (hit);
}

static int	string_binary(t_cond *c, const char *l, const char *op,
				const char *rraw)
{
	char	*r;
	char	*pat;
	int		res;

	if (ft_strcmp(op, "<") == 0 || ft_strcmp(op, ">") == 0)
	{
		r = flat(c, rraw);
		res = ft_strcmp(l, r);
		return (free(r), op[0] == '<' ? res < 0 : res > 0);
	}
	if (ft_strcmp(op, "=~") == 0)
	{
		r = flat(c, rraw);
		res = regex_match(l, r);
		if (res < 0)
			c->err = 2;
		return (free(r), res > 0);
	}
	pat = expand_pattern((char *)rraw, c->shell);
	res = glob_match(pat ? pat : "", l);
	free(pat);
	return (op[0] == '!' ? !res : res);
}

static int	file_binary(t_cond *c, const char *l, const char *op,
				const char *rraw)
{
	char	*argv[5];
	char	*r;
	int		res;

	r = flat(c, rraw);
	argv[0] = "test";
	argv[1] = (char *)l;
	argv[2] = (char *)op;
	argv[3] = r;
	argv[4] = NULL;
	res = builtin_test(argv, c->shell) == 0;
	return (free(r), res);
}

static int	binary(t_cond *c, const char *op, const char *rraw, char *l)
{
	long	a;
	long	b;

	if (ft_strcmp(op, "-nt") == 0 || ft_strcmp(op, "-ot") == 0
		|| ft_strcmp(op, "-ef") == 0)
		return (file_binary(c, l, op, rraw));
	if (op[0] == '-')
	{
		a = number(c, l);
		b = number(c, rraw);
		if (ft_strcmp(op, "-eq") == 0)
			return (a == b);
		if (ft_strcmp(op, "-ne") == 0)
			return (a != b);
		if (ft_strcmp(op, "-lt") == 0)
			return (a < b);
		if (ft_strcmp(op, "-le") == 0)
			return (a <= b);
		return (ft_strcmp(op, "-gt") == 0 ? a > b : a >= b);
	}
	return (string_binary(c, l, op, rraw));
}

static int	unary(t_cond *c, const char *op, const char *raw)
{
	char	*arg;
	char	*argv[4];
	int		res;

	arg = flat(c, raw);
	if (ft_strcmp(op, "-v") == 0)
		res = env_get(c->shell->env, arg) != NULL;
	else
	{
		argv[0] = "test";
		argv[1] = (char *)op;
		argv[2] = arg;
		argv[3] = NULL;
		res = builtin_test(argv, c->shell) == 0;
	}
	return (free(arg), res);
}

static int	primary(t_cond *c)
{
	char	*l;
	int		res;

	if (c->i >= c->n)
		return (c->err = 2, 0);
	if (ft_strcmp(c->w[c->i], "(") == 0)
	{
		c->i++;
		res = cond_or(c);
		if (c->i < c->n && ft_strcmp(c->w[c->i], ")") == 0)
			c->i++;
		else
			c->err = 2;
		return (res);
	}
	if (c->i + 2 < c->n && is_binary_op(c->w[c->i + 1]))
	{
		l = flat(c, c->w[c->i]);
		res = binary(c, c->w[c->i + 1], c->w[c->i + 2], l);
		free(l);
		return (c->i += 3, res);
	}
	if (c->i + 1 < c->n && is_unary_op(c->w[c->i]))
		return (c->i += 2, unary(c, c->w[c->i - 2], c->w[c->i - 1]));
	l = flat(c, c->w[c->i++]);
	res = *l != '\0';
	return (free(l), res);
}

static int	cond_not(t_cond *c)
{
	if (c->i < c->n && ft_strcmp(c->w[c->i], "!") == 0)
	{
		c->i++;
		return (!cond_not(c));
	}
	return (primary(c));
}

static int	cond_and(t_cond *c)
{
	int	v;
	int	r;

	v = cond_not(c);
	while (c->i < c->n && ft_strcmp(c->w[c->i], "&&") == 0)
	{
		c->i++;
		r = cond_not(c);
		v = v && r;
	}
	return (v);
}

static int	cond_or(t_cond *c)
{
	int	v;
	int	r;

	v = cond_and(c);
	while (c->i < c->n && ft_strcmp(c->w[c->i], "||") == 0)
	{
		c->i++;
		r = cond_and(c);
		v = v || r;
	}
	return (v);
}

int	cond_command(char **argv, t_shell *shell)
{
	t_cond	c;
	int		res;

	c.w = argv + 1;
	c.n = 0;
	while (c.w[c.n])
		c.n++;
	c.n--;
	c.i = 0;
	c.err = 0;
	c.shell = shell;
	res = c.n > 0 ? cond_or(&c) : 0;
	if (c.err || c.i != c.n)
		return (ml_put_all(2, "minishell: [[: syntax error in conditional "
				"expression\n", NULL), 2);
	return (!res);
}

int	arith_command(char *word, t_shell *shell)
{
	char	*inner;
	char	*text;
	char	*res;
	int		status;

	inner = ft_substr(word, 2, ft_strlen(word) - 4);
	text = expand_variables(inner ? inner : "", shell);
	res = arith_string(text ? text : "", shell);
	status = !res || ft_strcmp(res, "0") == 0 || !*res;
	return (free(inner), free(text), free(res), status);
}
