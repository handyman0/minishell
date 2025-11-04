/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_tree.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 15:06:39 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:32:45 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include "../../includes/minishell.h"
#include "../../includes/utils.h"
#include "../../includes/builtins.h"

static int	execute_builtin(t_node *node, t_shell *shell)
{
	if (ft_strcmp(node->data.cmd.argv[0], "echo") == 0)
		return (builtin_echo(node->data.cmd.argv));
	else if (ft_strcmp(node->data.cmd.argv[0], "cd") == 0)
		return (builtin_cd(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "pwd") == 0)
		return (builtin_pwd());
	else if (ft_strcmp(node->data.cmd.argv[0], "export") == 0)
		return (builtin_export(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "unset") == 0)
		return (builtin_unset(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "env") == 0)
		return (builtin_env(shell->env));
	else if (ft_strcmp(node->data.cmd.argv[0], "exit") == 0)
		return (builtin_exit(node->data.cmd.argv, shell));
	return (-1);
}

static int	execute_command(t_node *node, t_shell *shell)
{
	char	*path;
	char	**env_array;
	int		status;
	int		builtin_status;

	if (!node || node->type != NODE_CMD || !node->data.cmd.argv)
		return (1);
	builtin_status = execute_builtin(node, shell);
	if (builtin_status != -1)
		return (builtin_status);
	path = find_path(node->data.cmd.argv[0], shell);
	if (!path)
	{
		printf("minishell: %s: command not found\n", node->data.cmd.argv[0]);
		return (127);
	}
	env_array = env_to_array(shell->env);
	if (fork() == 0)
	{
		execve(path, node->data.cmd.argv, env_array);
		perror("minishell");
		exit(126);
	}
	free(path);
	free_str_array(env_array);
	wait(&status);
	return (WEXITSTATUS(status));
}

static int	execute_pipe(t_node *node, t_shell *shell)
{
	int		pipefd[2];
	int		status_left;
	int		status_right;
	pid_t	pid_left, pid_right;

	if (pipe(pipefd) == -1)
		return (1);

	pid_left = fork();
	if (pid_left == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		exit(execute_tree(node->data.op.left, shell));
	}

	pid_right = fork();
	if (pid_right == 0)
	{
		close(pipefd[1]);
		dup2(pipefd[0], STDIN_FILENO);  // ← CORRIGIDO: STDIN_FILENO
		close(pipefd[0]);
		exit(execute_tree(node->data.op.right, shell));
	}

	close(pipefd[0]);
	close(pipefd[1]);
	waitpid(pid_left, &status_left, 0);
	waitpid(pid_right, &status_right, 0);
	return (WEXITSTATUS(status_right));
}

int	execute_tree(t_node *node, t_shell *shell)
{
	if (!node)
		return (1);

	switch (node->type)
	{
		case NODE_CMD:
			return (execute_command(node, shell));
		case NODE_PIPE:
			return (execute_pipe(node, shell));
		default:
			printf("minishell: unknown node type\n");
			return (1);
	}
}
