#include "../../includes/parser.h"
#include "../../includes/output.h"
#include "../../includes/ctl.h"

typedef struct s_arith
{
	const char	*p;
	t_shell		*shell;
	int			err;
	int			skip;
	int			depth;
	int			reported;
}	t_arith;

static const char	*g_levels[10][5] = {{"||", NULL}, {"&&", NULL},
{"|", NULL}, {"^", NULL}, {"&", NULL}, {"==", "!=", NULL},
{"<=", ">=", "<", ">", NULL}, {"<<", ">>", NULL}, {"+", "-", NULL},
{"*", "/", "%", NULL}};

static const char	*g_assign[] = {"<<=", ">>=", "+=", "-=", "*=", "/=", "%=",
	"&=", "^=", "|=", "=", NULL};

static long	comma(t_arith *a);
static long	assign(t_arith *a);
static long	arith_value(t_arith *a, const char *text);

static void	skip_space(t_arith *a)
{
	while (*a->p == ' ' || *a->p == '\t' || *a->p == '\n')
		a->p++;
}

static long	fail(t_arith *a, const char *msg)
{
	if (!a->err && msg)
		a->reported = 1;
	if (!a->err && msg)
		ml_put_all(2, "minishell: ", msg, "\n", NULL);
	a->err = 1;
	return (0);
}

static long	number(t_arith *a)
{
	unsigned long	value;
	int				base;
	int				digit;

	base = 10;
	if (a->p[0] == '0' && (a->p[1] == 'x' || a->p[1] == 'X'))
	{
		base = 16;
		a->p += 2;
	}
	else if (a->p[0] == '0' && ft_isdigit(a->p[1]))
		base = 8;
	value = 0;
	while (ft_isalnum(*a->p))
	{
		digit = ft_isdigit(*a->p) ? *a->p - '0' : (*a->p | 32) - 'a' + 10;
		if (digit >= base)
			return (fail(a, "invalid number in arithmetic expression"));
		value = value * base + digit;
		a->p++;
	}
	return ((long)value);
}

static long	read_scalar(t_arith *a, char *value)
{
	char	*end;
	long	n;

	if (!value || !*value)
		return (0);
	n = strtol(value, &end, 0);
	if (end != value && !*end)
		return (n);
	if (a->depth > 16)
		return (fail(a, "expression recursion level exceeded"));
	return (arith_value(a, value));
}

static long	read_var(t_arith *a, const char *name)
{
	char	*value;
	char	*first;
	long	n;

	value = env_get(a->shell->env, name);
	if (!value && !a->skip && (a->shell->opts & OPT_U))
	{
		ml_put_all(2, "minishell: ", name, ": unbound variable\n", NULL);
		return (a->err = a->reported = 1, 0);
	}
	if (!arr_is(value))
		return (read_scalar(a, value));
	first = arr_first(value);
	n = read_scalar(a, first);
	return (free(first), n);
}

static long	read_elem(t_arith *a, const char *name)
{
	const char	*close;
	char		*text;
	char		*elem;
	long		idx;
	long		res;

	close = ft_strchr(a->p, ']');
	if (!close)
		return (fail(a, "syntax error in arithmetic expression"));
	text = ft_substr(a->p, 1, close - a->p - 1);
	idx = arith_value(a, text ? text : "0");
	free(text);
	a->p = close + 1;
	elem = arr_elem(env_get(a->shell->env, name), idx);
	res = read_scalar(a, elem);
	return (free(elem), res);
}

static long	power(long base, long exp)
{
	unsigned long	result;
	unsigned long	b;

	result = 1;
	b = (unsigned long)base;
	while (exp-- > 0)
		result *= b;
	return ((long)result);
}

static long	divide(t_arith *a, char op, long l, long r)
{
	if (r == 0)
	{
		if (a->skip)
			return (0);
		return (fail(a, "division by 0"));
	}
	if (r == -1)
		return (op == '/' ? (long)(0UL - (unsigned long)l) : 0);
	return (op == '/' ? l / r : l % r);
}

static long	shift(const char *op, long l, long r)
{
	if (r < 0 || r >= 64)
		return (op[0] == '>' && l < 0 ? -1 : 0);
	if (op[0] == '<')
		return ((long)((unsigned long)l << r));
	return (l >> r);
}

static long	compare(const char *op, long l, long r)
{
	if (op[0] == '=')
		return (l == r);
	if (op[0] == '!')
		return (l != r);
	if (op[0] == '<')
		return (op[1] ? l <= r : l < r);
	return (op[1] ? l >= r : l > r);
}

static long	apply(t_arith *a, const char *op, long l, long r)
{
	if (ft_strcmp(op, "==") == 0 || ft_strcmp(op, "!=") == 0
		|| ft_strcmp(op, "<=") == 0 || ft_strcmp(op, ">=") == 0
		|| ft_strcmp(op, "<") == 0 || ft_strcmp(op, ">") == 0)
		return (compare(op, l, r));
	if (op[0] == '<' && op[1] == '<')
		return (shift(op, l, r));
	if (op[0] == '>' && op[1] == '>')
		return (shift(op, l, r));
	if (op[0] == '/' || op[0] == '%')
		return (divide(a, op[0], l, r));
	if (op[0] == '+')
		return ((long)((unsigned long)l + (unsigned long)r));
	if (op[0] == '-')
		return ((long)((unsigned long)l - (unsigned long)r));
	if (op[0] == '*')
		return ((long)((unsigned long)l * (unsigned long)r));
	if (op[0] == '|')
		return (l | r);
	if (op[0] == '&')
		return (l & r);
	return (l ^ r);
}

static int	op_ok(const char *p, const char *op)
{
	size_t	n;
	char	next;

	n = ft_strlen(op);
	next = p[n];
	if (next == '=' && (ft_strchr("+-*/%&^|", op[0]) || (n == 2
				&& (op[0] == '<' || op[0] == '>') && op[0] == op[1])))
		return (0);
	if (n == 1 && (op[0] == '|' || op[0] == '&' || op[0] == '<'
			|| op[0] == '>' || op[0] == '*') && next == op[0])
		return (0);
	return (1);
}

static const char	*match_level(t_arith *a, int lvl)
{
	int	i;

	i = 0;
	while (g_levels[lvl][i])
	{
		if (ft_strncmp(a->p, g_levels[lvl][i],
				ft_strlen(g_levels[lvl][i])) == 0
			&& op_ok(a->p, g_levels[lvl][i]))
			return (g_levels[lvl][i]);
		i++;
	}
	return (NULL);
}

static long	primary(t_arith *a);

static long	unary(t_arith *a)
{
	char	op;
	long	value;

	skip_space(a);
	if (*a->p != '-' && *a->p != '+' && *a->p != '!' && *a->p != '~')
		return (primary(a));
	op = *a->p++;
	value = unary(a);
	if (op == '-')
		return ((long)(0UL - (unsigned long)value));
	if (op == '!')
		return (!value);
	if (op == '~')
		return (~value);
	return (value);
}

static long	power_expr(t_arith *a)
{
	long	base;
	long	exp;

	base = unary(a);
	skip_space(a);
	if (a->p[0] != '*' || a->p[1] != '*')
		return (base);
	a->p += 2;
	exp = power_expr(a);
	if (exp < 0 && !a->skip)
		return (fail(a, "exponent less than 0"));
	return (power(base, exp));
}

static long	binary(t_arith *a, int lvl)
{
	long		left;
	long		right;
	const char	*op;

	if (lvl > 9)
		return (power_expr(a));
	left = binary(a, lvl + 1);
	skip_space(a);
	op = match_level(a, lvl);
	while (op && !a->err)
	{
		a->p += ft_strlen(op);
		if (lvl < 2)
			a->skip += (lvl == 0) == (left != 0);
		right = binary(a, lvl + 1);
		if (lvl < 2)
			a->skip -= (lvl == 0) == (left != 0);
		if (lvl == 0)
			left = left || right;
		else if (lvl == 1)
			left = left && right;
		else
			left = apply(a, op, left, right);
		skip_space(a);
		op = match_level(a, lvl);
	}
	return (left);
}

static long	ternary(t_arith *a)
{
	long	cond;
	long	yes;
	long	no;

	cond = binary(a, 0);
	skip_space(a);
	if (*a->p != '?' || a->err)
		return (cond);
	a->p++;
	a->skip += (cond == 0);
	yes = comma(a);
	a->skip -= (cond == 0);
	skip_space(a);
	if (*a->p != ':')
		return (fail(a, "syntax error in arithmetic expression"));
	a->p++;
	a->skip += (cond != 0);
	no = assign(a);
	a->skip -= (cond != 0);
	return (cond ? yes : no);
}

static long	store(t_arith *a, const char *name, long value)
{
	char	*text;
	char	buf[32];
	int		i;
	unsigned long	n;

	if (a->skip)
		return (value);
	if (env_is_readonly(a->shell->env, name))
		return (ml_put_all(2, "minishell: ", name, ": readonly variable\n",
				NULL), a->err = a->reported = 1, 0);
	n = value < 0 ? 0UL - (unsigned long)value : (unsigned long)value;
	i = 31;
	buf[i] = '\0';
	if (n == 0)
		buf[--i] = '0';
	while (n > 0)
	{
		buf[--i] = '0' + n % 10;
		n /= 10;
	}
	if (value < 0)
		buf[--i] = '-';
	text = ft_strdup(buf + i);
	env_assign(&a->shell->env, name, text);
	free(text);
	return (value);
}

static const char	*assign_op(t_arith *a)
{
	int	i;

	i = 0;
	while (g_assign[i])
	{
		if (ft_strncmp(a->p, g_assign[i], ft_strlen(g_assign[i])) == 0
			&& (ft_strlen(g_assign[i]) > 1 || a->p[1] != '='))
			return (g_assign[i]);
		i++;
	}
	return (NULL);
}

static long	assign(t_arith *a)
{
	const char	*start;
	const char	*op;
	char		name[256];
	size_t		len;
	long		value;

	skip_space(a);
	start = a->p;
	len = 0;
	if (ft_isalpha(*a->p) || *a->p == '_')
		while (ft_isalnum(a->p[len]) || a->p[len] == '_')
			len++;
	if (len == 0 || len >= sizeof(name))
		return (ternary(a));
	a->p += len;
	skip_space(a);
	op = assign_op(a);
	if (!op)
		return (a->p = start, ternary(a));
	ft_memcpy(name, start, len);
	name[len] = '\0';
	a->p += ft_strlen(op);
	value = assign(a);
	if (op[1])
		value = apply(a, op, read_var(a, name), value);
	return (store(a, name, value));
}

static long	comma(t_arith *a)
{
	long	value;

	value = assign(a);
	skip_space(a);
	while (*a->p == ',' && !a->err)
	{
		a->p++;
		value = assign(a);
		skip_space(a);
	}
	return (value);
}

static long	primary(t_arith *a)
{
	long	value;
	char	name[256];
	size_t	len;

	skip_space(a);
	if (*a->p == '(')
	{
		a->p++;
		value = comma(a);
		skip_space(a);
		if (*a->p != ')')
			return (fail(a, "syntax error in arithmetic expression"));
		a->p++;
		return (value);
	}
	if (ft_isdigit(*a->p))
		return (number(a));
	if (!ft_isalpha(*a->p) && *a->p != '_')
		return (fail(a, NULL));
	len = 0;
	while (ft_isalnum(a->p[len]) || a->p[len] == '_')
		len++;
	if (len >= sizeof(name))
		return (fail(a, NULL));
	ft_memcpy(name, a->p, len);
	name[len] = '\0';
	a->p += len;
	if (*a->p == '[')
		return (read_elem(a, name));
	return (read_var(a, name));
}

static long	arith_value(t_arith *a, const char *text)
{
	t_arith	sub;
	long	value;

	sub = *a;
	sub.p = text;
	sub.depth = a->depth + 1;
	value = comma(&sub);
	skip_space(&sub);
	if (sub.err || *sub.p)
		a->err = 1;
	return (value);
}

static char	*long_text(long value)
{
	char			buf[32];
	int				i;
	unsigned long	n;

	n = value < 0 ? 0UL - (unsigned long)value : (unsigned long)value;
	i = 31;
	buf[i] = '\0';
	if (n == 0)
		buf[--i] = '0';
	while (n > 0)
	{
		buf[--i] = '0' + n % 10;
		n /= 10;
	}
	if (value < 0)
		buf[--i] = '-';
	return (ft_strdup(buf + i));
}

char	*arith_string(const char *text, t_shell *shell)
{
	t_arith	a;
	long	value;

	a.p = text;
	a.shell = shell;
	a.err = 0;
	a.skip = 0;
	a.depth = 0;
	a.reported = 0;
	value = comma(&a);
	skip_space(&a);
	if (!a.err && *a.p)
		a.err = 1;
	if (a.err && !a.reported)
		ml_put_all(2, "minishell: arithmetic error: ", text, "\n", NULL);
	if (a.err)
	{
		shell->expand_error = 1;
		return (ft_strdup("0"));
	}
	return (long_text(value));
}
