/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_tree.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 15:06:39 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 19:41:41 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include "../../includes/minishell.h"
#include "../../includes/utils.h"
#include "../../includes/builtins.h"
#include <errno.h>
#include <string.h>
#include <signal.h>

static int	execute_and(t_node *node, t_shell *shell)
{
	int	left_status;

	left_status = execute_tree(node->data.op.left, shell);
	if (left_status == 0)
		return (execute_tree(node->data.op.right, shell));
	return (left_status);
}

static int	execute_or(t_node *node, t_shell *shell)
{
	int	left_status;

	left_status = execute_tree(node->data.op.left, shell);
	if (left_status != 0)
		return (execute_tree(node->data.op.right, shell));
	return (left_status);
}

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

static int	apply_redirections(t_redir *redirs, char *cmd_name)
{
	t_redir	*current;
	int		fd;

	current = redirs;
	while (current)
	{
		if (current->type == REDIR_HEREDOC)
		{
			/* heredoc: prepare stdin from limiter content */
			exec_heredoc(current->file);
			current = current->next;
			continue ;
		}
		if (current->type == REDIR_IN)
		{
			fd = open(current->file, O_RDONLY);
			if (fd == -1)
			{
				if (cmd_name)
					fprintf(stderr, "%s: %s: %s\n", cmd_name, current->file, strerror(errno));
				else
					perror(current->file);
				return (0);
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		else if (current->type == REDIR_OUT)
		{
			fd = open(current->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd == -1)
			{
				if (cmd_name)
					fprintf(stderr, "%s: %s: %s\n", cmd_name, current->file, strerror(errno));
				else
					perror(current->file);
				return (0);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		else if (current->type == REDIR_APPEND)
		{
			fd = open(current->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (fd == -1)
			{
				if (cmd_name)
					fprintf(stderr, "%s: %s: %s\n", cmd_name, current->file, strerror(errno));
				else
					perror(current->file);
				return (0);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		current = current->next;
	}
	return (1);
}

static int	execute_command(t_node *node, t_shell *shell)
{
	char	*path;
	char	**env_array;
	int		status;
	int		builtin_status;
	int		stdin_backup, stdout_backup;
	pid_t	pid;

	if (!node || node->type != NODE_CMD || !node->data.cmd.argv)
		return (1);

	/* if the command string is empty (result of empty expansion), treat as no-op */
	if (node->data.cmd.argv[0] && node->data.cmd.argv[0][0] == '\0')
		return (0);

	stdin_backup = dup(STDIN_FILENO);
	stdout_backup = dup(STDOUT_FILENO);

	/* detect builtin without applying redirections so we can decide where to apply them */
	int	is_builtin = 0;
	if (node->data.cmd.argv && node->data.cmd.argv[0])
	{
		if (ft_strcmp(node->data.cmd.argv[0], "echo") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "cd") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "pwd") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "export") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "unset") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "env") == 0
			|| ft_strcmp(node->data.cmd.argv[0], "exit") == 0)
			is_builtin = 1;
	}

	/* If it's a builtin, apply redirections in current process and run it */
	if (is_builtin)
	{
		if (!apply_redirections(node->data.cmd.redirs,
				node->data.cmd.argv ? node->data.cmd.argv[0] : NULL))
		{
			dup2(stdin_backup, STDIN_FILENO);
			dup2(stdout_backup, STDOUT_FILENO);
			close(stdin_backup);
			close(stdout_backup);
			return (1);
		}
		builtin_status = execute_builtin(node, shell);
		dup2(stdin_backup, STDIN_FILENO);
		dup2(stdout_backup, STDOUT_FILENO);
		close(stdin_backup);
		close(stdout_backup);
		return (builtin_status);
	}

	path = find_path(node->data.cmd.argv[0], shell);
	if (!path)
	{
		/* if the command contains a slash, report a file-not-found style error */
		if (ft_strchr(node->data.cmd.argv[0], '/'))
			fprintf(stderr, "%s: %s\n", node->data.cmd.argv[0], strerror(ENOENT));
		else
			fprintf(stderr, "minishell: %s: command not found\n", node->data.cmd.argv[0]);
		dup2(stdin_backup, STDIN_FILENO);
		dup2(stdout_backup, STDOUT_FILENO);
		close(stdin_backup);
		close(stdout_backup);
		return (127);
	}

	env_array = env_to_array(shell->env);

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		dup2(stdin_backup, STDIN_FILENO);
		dup2(stdout_backup, STDOUT_FILENO);
		close(stdin_backup);
		close(stdout_backup);
		free(path);
		free_str_array(env_array);
		return (1);
	}

	if (pid == 0)
	{
		/* restore default signal handlers in child so it behaves like a normal program */
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		/* child: apply redirections then exec */
		if (!apply_redirections(node->data.cmd.redirs,
				node->data.cmd.argv ? node->data.cmd.argv[0] : NULL))
		{
			free(path);
			free_str_array(env_array);
			exit(1);
		}
		execve(path, node->data.cmd.argv, env_array);
		perror("minishell");
		free(path);
		free_str_array(env_array);
		exit(126);
	}

	free(path);
	free_str_array(env_array);
	wait(&status);

	dup2(stdin_backup, STDIN_FILENO);
	dup2(stdout_backup, STDOUT_FILENO);
	close(stdin_backup);
	close(stdout_backup);

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
		/* restore default signals in pipeline children */
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		exit(execute_tree(node->data.op.left, shell));
	}

	pid_right = fork();
	if (pid_right == 0)
	{
		/* restore default signals in pipeline children */
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		close(pipefd[1]);
		dup2(pipefd[0], STDIN_FILENO);
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
		case NODE_AND:
			return (execute_and(node, shell));
		case NODE_OR:
			return (execute_or(node, shell));
		default:
			printf("minishell: unknown node type\n");
			return (1);
	}
}
