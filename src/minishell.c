/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:24:51 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 19:41:34 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"
#include "../includes/builtins.h"
#include "../includes/output.h"
#include "../includes/prompt.h"

static void	init_env_defaults(t_shell *shell)
{
	char	*level;
	char	*value;

	if (!env_get(shell->env, "PATH"))
		env_set(&shell->env, "PATH",
			"/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
	level = env_get(shell->env, "SHLVL");
	value = ft_itoa(level ? ft_atoi(level) + 1 : 1);
	if (value)
		env_set(&shell->env, "SHLVL", value);
	free(value);
}

static void	unexport(t_env *env, const char *key)
{
	while (env && ft_strcmp(env->key, key) != 0)
		env = env->next;
	if (env)
		env->exported = 0;
}

static void	init_shell(t_shell *shell, char **envp)
{
	char	cwd[1024];

	shell->env = env_init(envp);
	init_env_defaults(shell);
	shell->lineno = 0;
	shell->line_base = 0;
	shell->src_depth = 0;
	shell->exit_status = 0;
	shell->running = 1;
	shell->ast = NULL;
	shell->tokens = NULL;
	shell->aliases = NULL;
	shell->subst_status = 0;
	shell->subst_used = 0;
	shell->params = NULL;
	shell->nparams = 0;
	shell->argv0 = "minishell";
	shell->last_bg = 0;
	shell->pid = getpid();
	shell->brk = 0;
	shell->bypass_func = 0;
	shell->opts = 0;
	shell->noerrexit = 0;
	shell->spare = 0;
	shell->optpos = 0;
	shell->optind_last = 1;
	shell->optsilent = 0;
	env_set(&shell->env, "OPTIND", "1");
	unexport(shell->env, "OPTIND");
	env_set(&shell->env, "IFS", " \t\n");
	unexport(shell->env, "IFS");
	shell->expand_error = 0;
	shell->cont = 0;
	shell->ret = 0;
	shell->loop_depth = 0;
	shell->func_depth = 0;
	shell->funcs = NULL;
	shell->locals = NULL;
	ft_bzero(shell->traps, sizeof(shell->traps));
	ft_bzero(shell->trap_inherited, sizeof(shell->trap_inherited));
	shell->in_trap = 0;
	shell->bgs = NULL;
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		env_set(&shell->env, "PWD", cwd);
	else
		put_errno("getcwd", NULL);
}

static void	run_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*start;
	t_node	*ast;
	t_node	*saved_ast;
	t_token	*saved_tokens;

	saved_ast = shell->ast;
	saved_tokens = shell->tokens;
	start = tokens;
	shell->tokens = start;
	parse_reset();
	ast = parse_tokens(&tokens);
	if (ast && tokens && !parse_failed())
		parse_fail_at(tokens);
	if (parse_failed())
	{
		parse_report(shell);
		shell->running = shell->spare;
		free_ast(ast);
		ast = NULL;
	}
	if (ast && prepare_heredocs(ast, shell))
	{
		shell->exit_status = (g_signal == SIGINT) ? 130 : 1;
		free_ast(ast);
		ast = NULL;
	}
	if (ast && (shell->opts & OPT_N) && !shell->interactive)
		free_ast(ast), ast = NULL;
	if (ast)
	{
		shell->ast = ast;
		shell->exit_status = execute_tree(ast, shell);
		shell->ast = NULL;
		free_ast(ast);
	}
	free_tokens(start);
	shell->ast = saved_ast;
	shell->tokens = saved_tokens;
}

static void	shift_lines(t_token *tok, int base)
{
	while (tok)
	{
		tok->line += base;
		tok = tok->next;
	}
}

static void	run_chunk(char *line, t_shell *shell)
{
	t_token	*tokens;

	if (shell->opts & OPT_V)
		ml_put_all(2, line, "\n", NULL);
	if (check_quotes(line))
	{
		shell->exit_status = 2;
		shell->running = shell->spare;
		return ;
	}
	tokens = tokenize_line(line);
	if (!tokens)
		return ;
	shift_lines(tokens, shell->line_base);
	expand_aliases(&tokens, shell);
	if (check_syntax(tokens))
	{
		shell->exit_status = 2;
		shell->running = shell->spare;
		free_tokens(tokens);
		return ;
	}
	run_tokens(tokens, shell);
}

static int	next_chunk(const char *line, char **chunk)
{
	size_t	n;
	char	*part;

	n = 0;
	while (1)
	{
		if (ft_strnstr(line, "<<", ft_strlen(line)))
			n = ft_strlen(line) - 1;
		while (line[n] && line[n] != '\n')
			n++;
		if (line[n])
			n++;
		part = ft_substr(line, 0, n - (line[n - 1] == '\n' && line[n]));
		if (!part || !line[n] || !line_incomplete(part))
			break ;
		free(part);
	}
	*chunk = part;
	return ((int)n);
}

void	run_line(char *line, t_shell *shell)
{
	char	*chunk;
	int		base;
	int		n;
	int		off;

	base = shell->line_base;
	off = 0;
	while (shell->running && !shell->ret && line[off])
	{
		n = next_chunk(line + off, &chunk);
		if (!chunk)
			break ;
		if (!line[off + n])
			(free(chunk), chunk = NULL);
		run_chunk(chunk ? chunk : line + off, shell);
		free(chunk);
		while (n-- > 0)
			shell->line_base += line[off++] == '\n';
	}
	shell->line_base = base;
}

static char	*read_input(t_shell *shell, int interactive)
{
	char	*line;
	char	*prompt;

	if (!interactive)
		return (ml_reader_command(ml_stdin_reader(), line_incomplete));
	prompt = build_prompt(shell);
	line = le_readline(prompt ? prompt : "minishell> ", shell, LE_FANCY);
	free(prompt);
	return (line);
}

static void	load_rc(t_shell *shell)
{
	char	*home;
	char	*path;

	home = env_get(shell->env, "HOME");
	if (!home || !*home)
		return ;
	path = ft_strjoin(home, "/.minishellrc");
	if (!path)
		return ;
	source_file(path, shell);
	free(path);
	shell->exit_status = 0;
}

static char	*history_path(t_shell *shell)
{
	char	*home;

	home = env_get(shell->env, "HOME");
	if (!home || !*home)
		return (NULL);
	return (ft_strjoin(home, "/.minishell_history"));
}

static void	history_load(t_shell *shell)
{
	char	*path;

	path = history_path(shell);
	if (!path)
		return ;
	le_history_load(path);
	free(path);
}

static void	history_save(t_shell *shell)
{
	char	*path;

	path = history_path(shell);
	if (!path)
		return ;
	le_history_save(path);
	free(path);
}

static void	shell_loop(t_shell *shell, int interactive)
{
	char	*line;

	while (shell->running)
	{
		g_signal = 0;
		if (interactive)
			jobs_notify(shell);
		else
			bg_reap(shell);
		trap_run_pending(shell);
			jobs_notify(shell);
		line = read_input(shell, interactive);
		if (g_signal == SIGINT)
			shell->exit_status = 130;
		if (!line)
			break ;
		if (interactive && *line)
			le_history_add(line);
		shell->line_base = (int)ml_stdin_reader()->cmd_first - 1;
		if (!interactive)
			ml_reader_sync(ml_stdin_reader());
		run_line(line, shell);
		free(line);
	}
}

typedef struct s_opts
{
	char	*cmd;
	char	*script;
	int		use_stdin;
	int		bad;
	int		rest;
	unsigned int	opts;
	int		force_i;
}	t_opts;

static void	parse_flags(char *arg, int argc, char **argv, t_opts *o)
{
	int	k;

	k = 1;
	while (arg[k])
	{
		if (arg[k] == 'c' && arg[0] == '-')
		{
			o->bad = (o->rest + 1 >= argc);
			o->cmd = o->rest + 1 < argc ? argv[o->rest + 1] : NULL;
			o->rest++;
		}
		else if (arg[k] == 'i' && arg[0] == '-')
			o->force_i = 1;
		else if (arg[k] == 's')
			o->use_stdin = 1;
		else if (arg[0] == '+' && arg[k] == 'o' && o->rest + 1 < argc)
			o->opts &= ~option_bit_name(argv[++o->rest]);
		else if (arg[0] == '+')
			o->opts &= ~option_bit_letter(arg[k]);
		else if (arg[k] == 'o' && o->rest + 1 < argc)
			o->opts |= option_bit_name(argv[++o->rest]);
		else
			o->opts |= option_bit_letter(arg[k]);
		k++;
	}
}

static void	parse_args(int argc, char **argv, t_opts *o)
{
	ft_bzero(o, sizeof(t_opts));
	o->rest = 1;
	while (o->rest < argc && (argv[o->rest][0] == '-' || argv[o->rest][0] == '+')
		&& argv[o->rest][1])
	{
		if (ft_strcmp(argv[o->rest], "--") == 0)
		{
			o->rest++;
			break ;
		}
		parse_flags(argv[o->rest], argc, argv, o);
		o->rest++;
	}
	if (!o->cmd && !o->use_stdin && o->rest < argc)
		o->script = argv[o->rest++];
}

static void	set_params(t_shell *shell, char **args, int n)
{
	int	i;

	shell->params = ft_calloc(n + 1, sizeof(char *));
	shell->nparams = 0;
	i = 0;
	while (shell->params && i < n)
	{
		shell->params[i] = ft_strdup(args[i]);
		shell->nparams++;
		i++;
	}
}

static int	run_noninteractive(t_shell *shell, t_opts *o)
{
	setup_signals_noninteractive();
	if (o->cmd)
		run_line(o->cmd, shell);
	else if (source_file(o->script, shell) == -1)
	{
		put_errno(o->script, NULL);
		shell->exit_status = 127;
	}
	trap_run_exit(shell);
	return (shell->exit_status);
}

static void	cleanup(t_shell *shell, int hangup)
{
	jobs_free(shell, hangup);
	funcs_free(shell);
	traps_free(shell);
	bgs_free(shell);
	free_str_array(shell->params);
	env_free(shell->aliases);
	env_free(shell->env);
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	shell;
	t_opts	o;
	int		interactive;

	parse_args(argc, argv, &o);
	if (o.bad)
		return (ml_put_all(2, "minishell: -c: option requires an argument\n",
				NULL), 2);
	init_shell(&shell, envp);
	shell.opts = o.opts;
	g_noclobber = (o.opts & OPT_C) != 0;
	jobs_init(&shell, !o.cmd && !o.script);
	shell.spare = shell.interactive || o.force_i;
	if (o.cmd || o.script)
	{
		if (o.script || o.rest < argc)
			shell.argv0 = o.script ? o.script : argv[o.rest++];
		set_params(&shell, argv + o.rest, argc - o.rest);
		o.bad = run_noninteractive(&shell, &o);
		return (cleanup(&shell, 0), o.bad);
	}
	set_params(&shell, argv + o.rest, argc - o.rest);
	interactive = shell.interactive;
	if (interactive)
		setup_signals();
	else
		setup_signals_noninteractive();
	if (interactive)
	{
		history_load(&shell);
		load_rc(&shell);
	}
	shell_loop(&shell, interactive);
	trap_run_exit(&shell);
	if (interactive)
		history_save(&shell);
	cleanup(&shell, 1);
	if (interactive)
		ml_put_all(STDOUT_FILENO, "exit\n", NULL);
	return (shell.exit_status);
}
