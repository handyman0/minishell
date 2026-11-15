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

static void	init_shell(t_shell *shell, char **envp)
{
	char	cwd[1024];

	shell->env = env_init(envp);
	shell->exit_status = 0;
	shell->running = 1;
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		env_set(&shell->env, "PWD", cwd);
	else
		perror("minishell: getcwd");
}

static void	run_line(char *line, t_shell *shell)
{
	t_token	*tokens;
	t_token	*tokens_start;
	t_node	*ast;

	tokens = tokenize_line(line);
	if (!tokens)
		return ;
	tokens_start = tokens;
	ast = parse_tokens(&tokens);
	if (ast)
	{
		shell->exit_status = execute_tree(ast, shell);
		free_ast(ast);
	}
	free_tokens(tokens_start);
}

static char	*read_input(int interactive)
{
	char	*line;
	size_t	len;

	if (interactive)
		return (readline("minishell> "));
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
}

static void	shell_loop(t_shell *shell, int interactive)
{
	char	*line;

	while (shell->running)
	{
		g_signal = 0;
		line = read_input(interactive);
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
	if (argc >= 3 && ft_strcmp(argv[1], "-c") == 0)
	{
		setup_signals_noninteractive();
		run_line(argv[2], &shell);
		env_free(shell.env);
		return (shell.exit_status);
	}
	interactive = isatty(STDIN_FILENO);
	if (interactive)
		setup_signals();
	else
		setup_signals_noninteractive();
	shell_loop(&shell, interactive);
	env_free(shell.env);
	if (interactive)
		printf("exit\n");
	return (shell.exit_status);
}
