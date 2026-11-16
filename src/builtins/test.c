#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <sys/stat.h>

typedef struct s_test
{
	char	**a;
	int		n;
	int		i;
	int		err;
}	t_test;

static int	test_or(t_test *t);

static int	file_test(const char *op, const char *path)
{
	struct stat	st;
	int			ok;

	if (op[1] == 'L' || op[1] == 'h')
		return (lstat(path, &st) == 0 && S_ISLNK(st.st_mode));
	ok = stat(path, &st) == 0;
	if (op[1] == 'e')
		return (ok);
	if (op[1] == 'f')
		return (ok && S_ISREG(st.st_mode));
	if (op[1] == 'd')
		return (ok && S_ISDIR(st.st_mode));
	if (op[1] == 's')
		return (ok && st.st_size > 0);
	if (op[1] == 'r')
		return (access(path, R_OK) == 0);
	if (op[1] == 'w')
		return (access(path, W_OK) == 0);
	if (op[1] == 'x')
		return (access(path, X_OK) == 0);
	if (op[1] == 'p')
		return (ok && S_ISFIFO(st.st_mode));
	if (op[1] == 'S')
		return (ok && S_ISSOCK(st.st_mode));
	if (op[1] == 'b')
		return (ok && S_ISBLK(st.st_mode));
	if (op[1] == 'g')
		return (ok && (st.st_mode & S_ISGID) != 0);
	if (op[1] == 'u')
		return (ok && (st.st_mode & S_ISUID) != 0);
	if (op[1] == 'k')
		return (ok && (st.st_mode & S_ISVTX) != 0);
	if (op[1] == 'O')
		return (ok && st.st_uid == geteuid());
	if (op[1] == 'G')
		return (ok && st.st_gid == getegid());
	return (op[1] == 'c' ? ok && S_ISCHR(st.st_mode) : -1);
}

static int	is_unary(const char *op)
{
	return (op[0] == '-' && op[1] && !op[2] && ft_strchr("zneEfdsrwxLhpSbctguOGk", op[1]));
}

static int	unary(t_test *t, const char *op, const char *arg)
{
	int	r;

	if (op[1] == 'z')
		return (*arg == '\0');
	if (op[1] == 'n')
		return (*arg != '\0');
	if (op[1] == 't')
		return (isatty(ft_atoi(arg)));
	r = file_test(op, arg);
	if (r < 0)
		t->err = 1;
	return (r > 0);
}

static int	is_int_op(const char *op)
{
	return (ft_strcmp(op, "-eq") == 0 || ft_strcmp(op, "-ne") == 0
		|| ft_strcmp(op, "-lt") == 0 || ft_strcmp(op, "-le") == 0
		|| ft_strcmp(op, "-gt") == 0 || ft_strcmp(op, "-ge") == 0);
}

static int	is_number(const char *s)
{
	if (*s == '-' || *s == '+')
		s++;
	if (!*s)
		return (0);
	while (*s)
		if (!ft_isdigit(*s++))
			return (0);
	return (1);
}

static int	compare_ints(t_test *t, const char *l, const char *op, const char *r)
{
	long	a;
	long	b;

	if (!is_number(l) || !is_number(r))
		return (t->err = 1, 0);
	a = ft_atoi(l);
	b = ft_atoi(r);
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

static int	is_file_op(const char *op)
{
	return (ft_strcmp(op, "-ef") == 0 || ft_strcmp(op, "-nt") == 0
		|| ft_strcmp(op, "-ot") == 0);
}

static int	is_binary(const char *op)
{
	return (ft_strcmp(op, "=") == 0 || ft_strcmp(op, "==") == 0
		|| ft_strcmp(op, "!=") == 0 || ft_strcmp(op, "<") == 0
		|| ft_strcmp(op, ">") == 0 || is_int_op(op) || is_file_op(op));
}

static int	compare_files(const char *l, const char *op, const char *r)
{
	struct stat	a;
	struct stat	b;

	if (stat(l, &a) != 0)
		return (0);
	if (ft_strcmp(op, "-ef") == 0)
		return (stat(r, &b) == 0 && a.st_dev == b.st_dev
			&& a.st_ino == b.st_ino);
	if (ft_strcmp(op, "-nt") == 0)
		return (stat(r, &b) != 0 || a.st_mtime > b.st_mtime);
	return (stat(r, &b) == 0 && a.st_mtime < b.st_mtime);
}

static int	binary(t_test *t, char *l, char *op, char *r)
{
	if (is_file_op(op))
		return (compare_files(l, op, r));
	if (is_int_op(op))
		return (compare_ints(t, l, op, r));
	if (op[0] == '=')
		return (ft_strcmp(l, r) == 0);
	if (op[0] == '!')
		return (ft_strcmp(l, r) != 0);
	if (op[0] == '<')
		return (ft_strcmp(l, r) < 0);
	return (ft_strcmp(l, r) > 0);
}

static int	primary(t_test *t)
{
	int	left;
	int	r;

	left = t->n - t->i;
	if (left <= 0)
		return (t->err = 1, 0);
	if (ft_strcmp(t->a[t->i], "(") == 0 && left >= 3)
	{
		t->i++;
		r = test_or(t);
		if (t->i < t->n && ft_strcmp(t->a[t->i], ")") == 0)
			t->i++;
		else
			t->err = 1;
		return (r);
	}
	if (left >= 3 && is_binary(t->a[t->i + 1]))
	{
		r = binary(t, t->a[t->i], t->a[t->i + 1], t->a[t->i + 2]);
		return (t->i += 3, r);
	}
	if (left >= 2 && is_unary(t->a[t->i]))
	{
		r = unary(t, t->a[t->i], t->a[t->i + 1]);
		return (t->i += 2, r);
	}
	return (t->a[t->i++][0] != '\0');
}

static int	test_not(t_test *t)
{
	if (t->i < t->n && ft_strcmp(t->a[t->i], "!") == 0 && t->n - t->i > 1)
	{
		t->i++;
		return (!test_not(t));
	}
	return (primary(t));
}

static int	test_and(t_test *t)
{
	int	v;
	int	r;

	v = test_not(t);
	while (t->i < t->n && ft_strcmp(t->a[t->i], "-a") == 0)
	{
		t->i++;
		r = test_not(t);
		v = v && r;
	}
	return (v);
}

static int	test_or(t_test *t)
{
	int	v;
	int	r;

	v = test_and(t);
	while (t->i < t->n && ft_strcmp(t->a[t->i], "-o") == 0)
	{
		t->i++;
		r = test_and(t);
		v = v || r;
	}
	return (v);
}

int	builtin_test(char **argv, t_shell *shell)
{
	t_test	t;
	int		result;

	(void)shell;
	t.a = argv + 1;
	t.n = 0;
	while (t.a[t.n])
		t.n++;
	if (ft_strcmp(argv[0], "[") == 0)
	{
		if (t.n == 0 || ft_strcmp(t.a[t.n - 1], "]") != 0)
			return (ml_put_all(2, "minishell: [: missing ]\n", NULL), 2);
		t.n--;
	}
	t.i = 0;
	t.err = 0;
	if (t.n == 0)
		return (1);
	result = test_or(&t);
	if (t.err || t.i != t.n)
		return (ml_put_all(2, "minishell: test: bad expression\n", NULL), 2);
	return (!result);
}
