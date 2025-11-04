/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 19:24:51 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 17:44:29 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"
#include <readline/readline.h>
#include <readline/history.h>

static void	init_shell(t_shell *shell, char **envp)
{
	shell->env = env_init(envp);
	shell->exit_status = 0;
	shell->running = 1;
	// configurar sinais
	setup_signals();
}

static void	shell_loop(t_shell *shell)
{
	char	*line;
	t_token *tokens;
	t_node	*ast;

	while (shell->running)
	{
		line = readline("minishell> ");
		if (!line)
			break ;
		if (*line)
			add_history(line);
		tokens = tokenize_line(line);
		if (tokens)
		{
			ast = parse_tokens(&tokens);
			if (ast)
			{
				shell->exit_status = execute_tree(ast, shell);
				free_ast(ast);
			}
			free_tokens(tokens);
		}
		free(line);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	shell;

	(void)argc;
	(void)argv;

	init_shell(&shell, envp);
	shell_loop(&shell);
	env_free(shell.env);

	printf("exit\n");
	return (shell.exit_status);
}
