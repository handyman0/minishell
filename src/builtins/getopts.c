#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

typedef struct s_opt_ctx
{
	char	*spec;
	char	*name;
	char	**args;
	int		argc;
	int		idx;
}	t_opt_ctx;

static void	set_var(t_shell *shell, const char *key, const char *value)
{
	env_assign(&shell->env, (char *)key, (char *)value);
}

static void	set_index(t_shell *shell, int idx)
{
	char	*text;

	text = ft_itoa(idx);
	set_var(shell, "OPTIND", text);
	free(text);
	shell->optind_last = idx;
}

static void	report(t_shell *shell, t_opt_ctx *ctx, char c, int missing)
{
	char	text[2];

	text[0] = c;
	text[1] = '\0';
	if (shell->optsilent)
	{
		set_var(shell, ctx->name, missing ? ":" : "?");
		set_var(shell, "OPTARG", text);
		return ;
	}
	ml_put_all(2, "minishell: ", missing ? "option requires an argument -- "
		: "illegal option -- ", text, "\n", NULL);
	set_var(shell, ctx->name, "?");
	env_unset(&shell->env, "OPTARG");
}

static void	with_argument(t_shell *shell, t_opt_ctx *ctx, char c)
{
	char	*arg;

	arg = ctx->args[ctx->idx - 1];
	if (arg[shell->optpos + 1])
		set_var(shell, "OPTARG", arg + shell->optpos + 1);
	else if (ctx->idx < ctx->argc)
	{
		set_var(shell, "OPTARG", ctx->args[ctx->idx]);
		ctx->idx++;
	}
	else
	{
		ctx->idx++;
		shell->optpos = 0;
		return (report(shell, ctx, c, 1));
	}
	ctx->idx++;
	shell->optpos = 0;
	set_var(shell, ctx->name, (char []){c, '\0'});
}

static void	parse_one(t_shell *shell, t_opt_ctx *ctx)
{
	char	*arg;
	char	*pos;
	char	c;

	arg = ctx->args[ctx->idx - 1];
	c = arg[shell->optpos];
	pos = c == ':' ? NULL : ft_strchr(ctx->spec, c);
	if (pos && pos[1] == ':')
		return (with_argument(shell, ctx, c));
	if (!arg[++shell->optpos])
	{
		shell->optpos = 0;
		ctx->idx++;
	}
	if (!pos)
		return (report(shell, ctx, c, 0));
	env_unset(&shell->env, "OPTARG");
	set_var(shell, ctx->name, (char []){c, '\0'});
}

static int	finish(t_shell *shell, t_opt_ctx *ctx, int idx)
{
	set_var(shell, ctx->name, "?");
	env_unset(&shell->env, "OPTARG");
	set_index(shell, idx);
	shell->optpos = 0;
	return (1);
}

int	builtin_getopts(char **argv, t_shell *shell)
{
	t_opt_ctx	ctx;
	char		*arg;
	char		*cur;

	if (!argv[1] || !argv[2])
		return (ml_put_all(2, "minishell: getopts: usage: getopts optstring"
				" name [arg ...]\n", NULL), 2);
	shell->optsilent = argv[1][0] == ':';
	ctx.spec = argv[1] + shell->optsilent;
	ctx.name = argv[2];
	ctx.args = argv[3] ? argv + 3 : shell->params;
	ctx.argc = 0;
	while (ctx.args && ctx.args[ctx.argc])
		ctx.argc++;
	cur = env_get(shell->env, "OPTIND");
	ctx.idx = cur ? ft_atoi(cur) : 1;
	if (ctx.idx < 1 || ctx.idx != shell->optind_last)
		shell->optpos = 0;
	ctx.idx = ctx.idx < 1 ? 1 : ctx.idx;
	arg = ctx.idx <= ctx.argc ? ctx.args[ctx.idx - 1] : NULL;
	if (!arg || arg[0] != '-' || !arg[1])
		return (finish(shell, &ctx, ctx.idx));
	if (ft_strcmp(arg, "--") == 0)
		return (finish(shell, &ctx, ctx.idx + 1));
	shell->optpos += shell->optpos == 0;
	parse_one(shell, &ctx);
	set_index(shell, ctx.idx);
	return (0);
}
