#include "../../includes/builtins.h"
#include "../../includes/parser.h"
#include "../../includes/output.h"

typedef struct s_decl
{
	int	exp;
	int	ro;
	int	integer;
	int	print;
	int	global;
	int	funcs;
	int	array;
	int	assoc;
}	t_decl;

static int	parse_flags(char **argv, t_decl *d, int *i)
{
	int	k;

	while (argv[*i] && argv[*i][0] == '-' && argv[*i][1])
	{
		if (ft_strcmp(argv[*i], "--") == 0 && ++(*i))
			break ;
		k = 0;
		while (argv[*i][++k])
		{
			if (argv[*i][k] == 'x')
				d->exp = 1;
			else if (argv[*i][k] == 'r')
				d->ro = 1;
			else if (argv[*i][k] == 'i')
				d->integer = 1;
			else if (argv[*i][k] == 'p')
				d->print = 1;
			else if (argv[*i][k] == 'g')
				d->global = 1;
			else if (argv[*i][k] == 'a')
				d->array = 1;
			else if (argv[*i][k] == 'A')
				d->assoc = 1;
			else if (argv[*i][k] == 'f' || argv[*i][k] == 'F')
				d->funcs = 1;
			else if (!ft_strchr("lutn", argv[*i][k]))
				return (ml_put_all(2, "minishell: declare: -",
						(char []){argv[*i][k], 0}, ": invalid option\n", NULL),
					0);
		}
		(*i)++;
	}
	return (1);
}

static void	show_array(const char *v)
{
	char	*text;

	text = assoc_is(v) ? assoc_display(v) : arr_display(v);
	ml_put_all(STDOUT_FILENO, "=", text, NULL);
	free(text);
}

static void	print_one(t_env *var, t_decl *d)
{
	char	flags[8];
	int		n;

	n = 0;
	flags[n++] = '-';
	if (arr_is(var->value))
		flags[n++] = 'a';
	if (assoc_is(var->value))
		flags[n++] = 'A';
	if (var->readonly)
		flags[n++] = 'r';
	if (var->exported)
		flags[n++] = 'x';
	if (n == 1)
		flags[n++] = '-';
	flags[n] = '\0';
	(void)d;
	ml_put_all(STDOUT_FILENO, "declare ", flags, " ", var->key, NULL);
	if (arr_like(var->value))
		show_array(var->value);
	else if (var->value)
		ml_put_all(STDOUT_FILENO, "=\"", var->value, "\"", NULL);
	ft_putchar_fd('\n', STDOUT_FILENO);
}

static int	print_vars(char **names, t_shell *shell, t_decl *d)
{
	t_env	*var;
	int		status;

	status = 0;
	if (!*names)
	{
		var = shell->env;
		while (var)
		{
			print_one(var, d);
			var = var->next;
		}
		return (0);
	}
	while (*names)
	{
		var = shell->env;
		while (var && ft_strcmp(var->key, *names) != 0)
			var = var->next;
		if (var)
			print_one(var, d);
		else if (++status)
			ml_put_all(2, "minishell: declare: ", *names, ": not found\n", NULL);
		names++;
	}
	return (status);
}

static char	*new_value(t_decl *d)
{
	static char	arr[2] = {ARR_MARK, 0};
	static char	map[2] = {ASSOC_MARK, 0};

	if (d->assoc)
		return (map);
	return (d->array ? arr : "");
}

static int	declare_one(char *arg, t_decl *d, t_shell *shell)
{
	char	*eq;
	char	*key;
	char	*val;

	eq = ft_strchr(arg, '=');
	key = eq ? ft_substr(arg, 0, eq - arg) : ft_strdup(arg);
	val = NULL;
	if (eq)
		val = d->integer ? arith_string(eq + 1, shell) : ft_strdup(eq + 1);
	if (!key || !*key || ft_isdigit(*key))
		return (free(key), free(val), ml_put_all(2, "minishell: declare: `",
				arg, "': not a valid identifier\n", NULL), 1);
	if (env_is_readonly(shell->env, key) && val)
		return (free(key), free(val), ml_put_all(2, "minishell: ", arg,
				": readonly variable\n", NULL), 1);
	if (shell->func_depth > 0 && !d->global)
		builtin_local((char *[]){"local", key, NULL}, shell);
	if (val)
		env_assign(&shell->env, key, val);
	else if (!env_get(shell->env, key))
		env_assign(&shell->env, key, new_value(d));
	else if (d->assoc && !assoc_is(env_get(shell->env, key)) && !val)
		env_assign(&shell->env, key, new_value(d));
	else if (d->array && !arr_is(env_get(shell->env, key)))
		val = NULL;
	if (d->exp)
		env_export(shell->env, key);
	if (d->ro)
		env_set_readonly(shell->env, key);
	return (free(key), free(val), 0);
}

int	builtin_declare(char **argv, t_shell *shell)
{
	t_decl	d;
	int		i;
	int		status;

	ft_bzero(&d, sizeof(d));
	i = 1;
	if (!parse_flags(argv, &d, &i))
		return (2);
	if (d.funcs)
		return (0);
	if (d.print || !argv[i])
		return (print_vars(argv + i, shell, &d));
	status = 0;
	while (argv[i])
		status |= declare_one(argv[i++], &d, shell);
	return (status);
}
