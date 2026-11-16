#include "../../includes/parser.h"
#include "../../includes/output.h"

typedef struct s_arith
{
	const char	*p;
	t_shell		*shell;
	int			err;
}	t_arith;

typedef struct s_op
{
	const char	*text;
	int			prec;
}	t_op;

static const t_op	g_ops[] = {{"**", 11}, {"||", 1}, {"&&", 2}, {"==", 6},
{"!=", 6}, {"<=", 7}, {">=", 7}, {"<<", 8}, {">>", 8}, {"|", 3}, {"^", 4},
{"&", 5}, {"<", 7}, {">", 7}, {"+", 9}, {"-", 9}, {"*", 10}, {"/", 10},
{"%", 10}, {NULL, 0}};

static long	expr(t_arith *a, int min_prec);

static void	skip_space(t_arith *a)
{
	while (*a->p == ' ' || *a->p == '\t' || *a->p == '\n')
		a->p++;
}

static long	number(t_arith *a)
{
	long	value;
	int		base;
	int		digit;

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
			return (a->err = 1, 0);
		value = value * base + digit;
		a->p++;
	}
	return (value);
}

static long	variable(t_arith *a)
{
	char	name[256];
	char	*value;
	size_t	len;

	len = 0;
	while (ft_isalnum(a->p[len]) || a->p[len] == '_')
		len++;
	if (len >= sizeof(name))
		return (a->err = 1, 0);
	ft_memcpy(name, a->p, len);
	name[len] = '\0';
	a->p += len;
	value = env_get(a->shell->env, name);
	if (!value || !*value)
		return (0);
	return (ft_atoi(value));
}
static long	primary(t_arith *a);

static long	unary(t_arith *a)
{
	char	op;
	long	value;

	op = *a->p++;
	value = primary(a);
	if (op == '-')
		return (-value);
	if (op == '!')
		return (!value);
	if (op == '~')
		return (~value);
	return (value);
}

static long	primary(t_arith *a)
{
	long	value;

	skip_space(a);
	if (*a->p == '(')
	{
		a->p++;
		value = expr(a, 1);
		skip_space(a);
		if (*a->p != ')')
			return (a->err = 1, 0);
		a->p++;
		return (value);
	}
	if (*a->p == '-' || *a->p == '+' || *a->p == '!' || *a->p == '~')
		return (unary(a));
	if (ft_isdigit(*a->p))
		return (number(a));
	if (ft_isalpha(*a->p) || *a->p == '_')
		return (variable(a));
	return (a->err = 1, 0);
}

static long	power(long base, long exp)
{
	long	result;

	result = 1;
	while (exp-- > 0)
		result *= base;
	return (exp < -1 ? 0 : result);
}

static long	apply(t_arith *a, const char *op, long l, long r)
{
	if (op[0] == '|' && op[1] == '|')
		return (l || r);
	if (op[0] == '&' && op[1] == '&')
		return (l && r);
	if (op[0] == '=' || (op[0] == '!' && op[1] == '='))
		return ((l == r) == (op[0] == '='));
	if (op[0] == '<' && op[1] == '<')
		return (l << r);
	if (op[0] == '>' && op[1] == '>')
		return (l >> r);
	if (op[0] == '<' || op[0] == '>')
		return (op[0] == '<' ? (op[1] == '=' ? l <= r : l < r)
			: (op[1] == '=' ? l >= r : l > r));
	if (op[0] == '*' && op[1] == '*')
		return (power(l, r));
	if ((op[0] == '/' || op[0] == '%') && r == 0)
		return (put_all(2, "minishell: division by 0\n", NULL), a->err = 1, 0);
	return ((op[0] == '+') * (l + r) + (op[0] == '-') * (l - r)
		+ (op[0] == '*') * (l * r) + (op[0] == '/') * (r ? l / r : 0)
		+ (op[0] == '%') * (r ? l % r : 0) + (op[0] == '|') * (l | r)
		+ (op[0] == '&') * (l & r) + (op[0] == '^') * (l ^ r));
}

static const t_op	*match_op(t_arith *a)
{
	int	i;

	i = 0;
	while (g_ops[i].text)
	{
		if (ft_strncmp(a->p, g_ops[i].text, ft_strlen(g_ops[i].text)) == 0)
			return (&g_ops[i]);
		i++;
	}
	return (NULL);
}

static long	expr(t_arith *a, int min_prec)
{
	long		left;
	long		right;
	const t_op	*op;

	left = primary(a);
	skip_space(a);
	op = match_op(a);
	while (op && op->prec >= min_prec && !a->err)
	{
		a->p += ft_strlen(op->text);
		right = expr(a, op->prec + 1);
		left = apply(a, op->text, left, right);
		skip_space(a);
		op = match_op(a);
	}
	return (left);
}

static char	*long_text(long value)
{
	char	buf[32];
	int		i;
	int		negative;
	unsigned long	n;

	negative = value < 0;
	n = negative ? -(unsigned long)value : (unsigned long)value;
	i = 31;
	buf[i] = '\0';
	if (n == 0)
		buf[--i] = '0';
	while (n > 0)
	{
		buf[--i] = '0' + n % 10;
		n /= 10;
	}
	if (negative)
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
	value = expr(&a, 1);
	skip_space(&a);
	if (a.err || *a.p)
	{
		put_all(2, "minishell: syntax error in arithmetic expression\n", NULL);
		return (ft_strdup("0"));
	}
	return (long_text(value));
}
