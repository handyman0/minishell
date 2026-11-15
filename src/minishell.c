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
#include <readline/readline.h>
#include <readline/history.h>

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
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		env_set(&shell->env, "PWD", cwd);
	else
		perror("minishell: getcwd");
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
	ast = parse_tokens(&tokens);
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

static char	*build_prompt(t_shell *shell)
{
	char	cwd[1024];
	char	*home;
	char	*path;
	char	*tmp;
	char	*prompt;

	if (!getcwd(cwd, sizeof(cwd)))
		return (ft_strdup("minishell> "));
	home = env_get(shell->env, "HOME");
	path = ft_strdup(cwd);
	if (home && *home && path && ft_strncmp(cwd, home, ft_strlen(home)) == 0
		&& (cwd[ft_strlen(home)] == '/' || !cwd[ft_strlen(home)]))
	{
		free(path);
		path = ft_strjoin("~", cwd + ft_strlen(home));
	}
	if (!path)
		return (ft_strdup("minishell> "));
	tmp = ft_strjoin("minishell:", path);
	free(path);
	if (!tmp)
		return (ft_strdup("minishell> "));
	prompt = ft_strjoin(tmp, "> ");
	free(tmp);
	return (prompt);
}

static char	*read_input(t_shell *shell, int interactive)
{
	char	*line;
	char	*prompt;
	size_t	len;

	if (interactive)
	{
		prompt = build_prompt(shell);
		line = readline(prompt ? prompt : "minishell> ");
		free(prompt);
		return (line);
	}
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
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
	read_history(path);
	free(path);
}

static void	history_save(t_shell *shell)
{
	char	*path;

	path = history_path(shell);
	if (!path)
		return ;
	stifle_history(1000);
	write_history(path);
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
			add_history(line);
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
	{
		setup_completion(&shell);
		setup_suggestions(&shell);
	}
	shell_loop(&shell, interactive);
	if (interactive)
		history_save(&shell);
	jobs_free(&shell, 1);
	env_free(shell.aliases);
	env_free(shell.env);
	if (interactive)
		printf("exit\n");
	return (shell.exit_status);
}
