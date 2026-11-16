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
		return (ml_reader_line(ml_stdin_reader()));
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

int	main(int argc, char **argv, char **envp)
{
	t_shell	shell;
	int		interactive;

	init_shell(&shell, envp);
	jobs_init(&shell, !(argc >= 3 && ft_strcmp(argv[1], "-c") == 0));
	if (argc >= 3 && ft_strcmp(argv[1], "-c") == 0)
	{
		setup_signals_noninteractive();
		run_line(argv[2], &shell);
		jobs_free(&shell, 0);
		env_free(shell.aliases);
		env_free(shell.env);
		return (shell.exit_status);
	}
	interactive = shell.interactive;
	if (interactive)
		setup_signals();
	else
		setup_signals_noninteractive();
	if (interactive)
		history_load(&shell);
	if (interactive)
		load_rc(&shell);
	shell_loop(&shell, interactive);
	if (interactive)
		history_save(&shell);
	jobs_free(&shell, 1);
	env_free(shell.aliases);
	env_free(shell.env);
	if (interactive)
		ml_put_all(STDOUT_FILENO, "exit\n", NULL);
	return (shell.exit_status);
}
