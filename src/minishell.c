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

static void	init_shell(t_shell *shell, char **envp)
{
	char	cwd[1024];

	shell->env = env_init(envp);
	init_env_defaults(shell);
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
	shell->brk = 0;
	shell->bypass_func = 0;
	shell->cont = 0;
	shell->ret = 0;
	shell->loop_depth = 0;
	shell->func_depth = 0;
	shell->funcs = NULL;
	shell->locals = NULL;
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
		free_ast(ast);
		ast = NULL;
	}
	if (ast && prepare_heredocs(ast, shell))
	{
		shell->exit_status = (g_signal == SIGINT) ? 130 : 1;
		free_ast(ast);
		ast = NULL;
	}
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

void	run_line(char *line, t_shell *shell)
{
	t_token	*tokens;

	if (check_quotes(line))
	{
		shell->exit_status = 2;
		return ;
	}
	tokens = tokenize_line(line);
	if (!tokens)
		return ;
	expand_aliases(&tokens, shell);
	if (check_syntax(tokens))
	{
		shell->exit_status = 2;
		free_tokens(tokens);
		return ;
	}
	run_tokens(tokens, shell);
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
		line = read_input(shell, interactive);
		if (g_signal == SIGINT)
			shell->exit_status = 130;
		if (!line)
			break ;
		if (interactive && *line)
			le_history_add(line);
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
}	t_opts;

static void	parse_args(int argc, char **argv, t_opts *o)
{
	int	i;

	ft_bzero(o, sizeof(t_opts));
	i = 1;
	while (i < argc && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0)
		{
			i++;
			break ;
		}
		if (ft_strcmp(argv[i], "-c") == 0)
		{
			o->bad = (i + 1 >= argc);
			o->cmd = i + 1 < argc ? argv[i + 1] : NULL;
			i++;
		}
		else if (ft_strcmp(argv[i], "-s") == 0)
			o->use_stdin = 1;
		i++;
	}
	if (!o->cmd && !o->use_stdin && i < argc)
		o->script = argv[i++];
	o->rest = i;
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
	return (shell->exit_status);
}

static void	cleanup(t_shell *shell, int hangup)
{
	jobs_free(shell, hangup);
	funcs_free(shell);
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
	jobs_init(&shell, !o.cmd && !o.script);
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
	if (interactive)
		history_save(&shell);
	cleanup(&shell, 1);
	if (interactive)
		ml_put_all(STDOUT_FILENO, "exit\n", NULL);
	return (shell.exit_status);
}
