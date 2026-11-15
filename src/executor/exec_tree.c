/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_tree.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 15:06:39 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 20:11:37 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/executor.h"
#include "../../includes/minishell.h"
#include "../../includes/utils.h"
#include "../../includes/builtins.h"
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "../../includes/signals.h"

static int	execute_and(t_node *node, t_shell *shell)
{
	int	left_status;

	left_status = execute_tree(node->data.op.left, shell);
	shell->exit_status = left_status;
	if (left_status == 0)
		return (execute_tree(node->data.op.right, shell));
	return (left_status);
}

static int	execute_or(t_node *node, t_shell *shell)
{
	int	left_status;

	left_status = execute_tree(node->data.op.left, shell);
	shell->exit_status = left_status;
	if (left_status != 0)
		return (execute_tree(node->data.op.right, shell));
	return (left_status);
}

static int	execute_seq(t_node *node, t_shell *shell)
{
	int	status;

	status = execute_tree(node->data.op.left, shell);
	if (!shell->running)
		return (status);
	shell->exit_status = status;
	return (execute_tree(node->data.op.right, shell));
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

static void	restore_fds(int backup[3])
{
	int	i;

	i = 0;
	while (i < 3)
	{
		dup2(backup[i], i);
		close(backup[i]);
		i++;
	}
}

static int	open_redir(t_redir *redir)
{
	if (redir->type == REDIR_IN)
		return (open(redir->file, O_RDONLY));
	if (redir->type == REDIR_OUT)
		return (open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644));
	return (open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644));
}

static int	apply_redirections(t_redir *redirs, char *cmd_name, t_shell *shell)
{
	int	fd;

	while (redirs)
	{
		if (redirs->type == REDIR_HEREDOC)
			exec_heredoc(redirs->file, shell);
		else if (redirs->type == REDIR_DUP)
			dup2(ft_atoi(redirs->file + 1), redirs->fd);
		else
		{
			fd = open_redir(redirs);
			if (fd == -1)
			{
				fprintf(stderr, "%s: %s: %s\n", cmd_name ? cmd_name
					: "minishell", redirs->file, strerror(errno));
				return (0);
			}
			dup2(fd, redirs->fd);
			close(fd);
		}
		redirs = redirs->next;
	}
	return (1);
}

static int	is_dir(char *path)
{
	struct stat	st;

	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static int	execute_command(t_node *node, t_shell *shell)
{
	char	*path;
	char	**env_array;
	int		status;
	int		builtin_status;
	int		backup[3];
	pid_t	pid;

	if (!node || node->type != NODE_CMD || !node->data.cmd.argv)
		return (1);
	expand_command(node, shell);
	if (!node->data.cmd.argv || !node->data.cmd.argv[0])
		return (0);

	backup[0] = dup(STDIN_FILENO);
	backup[1] = dup(STDOUT_FILENO);
	backup[2] = dup(STDERR_FILENO);
	if (is_builtin(node->data.cmd.argv[0]))
	{
		if (!apply_redirections(node->data.cmd.redirs,
				node->data.cmd.argv ? node->data.cmd.argv[0] : NULL, shell))
		{
			restore_fds(backup);
			return (1);
		}
		builtin_status = execute_builtin(node, shell);
		restore_fds(backup);
		return (builtin_status);
	}

	path = find_path(node->data.cmd.argv[0], shell);
	if (!path)
	{
		if (ft_strchr(node->data.cmd.argv[0], '/'))
			fprintf(stderr, "minishell: %s: %s\n", node->data.cmd.argv[0], strerror(ENOENT));
		else
			fprintf(stderr, "minishell: %s: command not found\n", node->data.cmd.argv[0]);
		restore_fds(backup);
		return (127);
	}

	env_array = env_to_array(shell->env);

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		restore_fds(backup);
		free(path);
		free_str_array(env_array);
		return (1);
	}

	if (pid == 0)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		if (!apply_redirections(node->data.cmd.redirs,
				node->data.cmd.argv ? node->data.cmd.argv[0] : NULL, shell))
		{
			free(path);
			free_str_array(env_array);
			exit(1);
		}
		execve(path, node->data.cmd.argv, env_array);
		fprintf(stderr, "minishell: %s: %s\n", node->data.cmd.argv[0], errno == EACCES && is_dir(path) ? "Is a directory" : strerror(errno));
		free(path);
		free_str_array(env_array);
		exit(126);
	}

	free(path);
	free_str_array(env_array);
	signals_ignore();
	waitpid(pid, &status, 0);
	if (isatty(STDIN_FILENO))
		setup_signals();
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		write(STDERR_FILENO, "\n", 1);
	restore_fds(backup);

	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (WEXITSTATUS(status));
}

static int	pipe_child(t_node *node, t_shell *shell)
{
	int	status;

	status = execute_tree(node, shell);
	free_ast(shell->ast);
	free_tokens(shell->tokens);
	env_free(shell->env);
	return (status);
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
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		exit(pipe_child(node->data.op.left, shell));
	}

	pid_right = fork();
	if (pid_right == 0)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		close(pipefd[1]);
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[0]);
		exit(pipe_child(node->data.op.right, shell));
	}

	close(pipefd[0]);
	close(pipefd[1]);
	signals_ignore();
	waitpid(pid_left, &status_left, 0);
	waitpid(pid_right, &status_right, 0);
	if (isatty(STDIN_FILENO))
		setup_signals();
	if (WIFSIGNALED(status_right))
	{
		if (WTERMSIG(status_right) == SIGINT)
			write(STDERR_FILENO, "\n", 1);
		return (128 + WTERMSIG(status_right));
	}
	return (WEXITSTATUS(status_right));
}

static int	execute_subshell(t_node *node, t_shell *shell)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		exit(pipe_child(node->data.op.left, shell));
	}
	signals_ignore();
	waitpid(pid, &status, 0);
	if (isatty(STDIN_FILENO))
		setup_signals();
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (WEXITSTATUS(status));
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
		case NODE_PAREN:
			return (execute_subshell(node, shell));
		case NODE_SEQ:
			return (execute_seq(node, shell));
		default:
			printf("minishell: unknown node type\n");
			return (1);
	}
}
