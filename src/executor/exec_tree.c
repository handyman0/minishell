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
#include "../../includes/output.h"
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "../../includes/signals.h"
#include "../../includes/jobs.h"

static int	is_chain(t_node *node)
{
	return (node->type == NODE_SEQ || node->type == NODE_AND
		|| node->type == NODE_OR);
}

static int	chain_step(t_node *node, int status, t_shell *shell)
{
	if (!shell->running || shell->brk || shell->cont || shell->ret)
		return (status);
	shell->exit_status = status;
	if (node->type == NODE_SEQ
		|| (node->type == NODE_AND) == (status == 0))
		return (execute_tree(node->data.op.right, shell));
	return (status);
}

static int	execute_chain(t_node *root, t_shell *shell)
{
	t_node	*local[64];
	t_node	**stack;
	t_node	*cur;
	int		depth;
	int		status;

	depth = 0;
	cur = root;
	while (cur && is_chain(cur))
	{
		depth++;
		cur = cur->data.op.left;
	}
	stack = local;
	if (depth > 64)
		stack = malloc(sizeof(t_node *) * depth);
	if (!stack)
		return (1);
	cur = root;
	depth = 0;
	while (cur && is_chain(cur))
	{
		stack[depth++] = cur;
		cur = cur->data.op.left;
	}
	status = execute_tree(cur, shell);
	while (depth-- > 0)
		status = chain_step(stack[depth], status, shell);
	if (stack != local)
		free(stack);
	return (status);
}

static int	execute_builtin(t_node *node, t_shell *shell)
{
	int	(*ext)(char **, t_shell *);

	ext = find_ext_builtin(node->data.cmd.argv[0]);
	if (ext)
		return (ext(node->data.cmd.argv, shell));
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
	else if (ft_strcmp(node->data.cmd.argv[0], "jobs") == 0)
		return (builtin_jobs(shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "fg") == 0)
		return (builtin_fg(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "bg") == 0)
		return (builtin_bg(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "alias") == 0)
		return (builtin_alias(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "unalias") == 0)
		return (builtin_unalias(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "source") == 0)
		return (builtin_source(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "type") == 0)
		return (builtin_type(node->data.cmd.argv, shell));
	else if (ft_strcmp(node->data.cmd.argv[0], "which") == 0)
		return (builtin_which(node->data.cmd.argv, shell));
	return (-1);
}

void	save_fds(int backup[3])
{
	backup[0] = dup(STDIN_FILENO);
	backup[1] = dup(STDOUT_FILENO);
	backup[2] = dup(STDERR_FILENO);
}

void	restore_fds(int backup[3])
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

int	apply_redirections(t_redir *redirs)
{
	int	fd;

	while (redirs)
	{
		if (redirs->type == REDIR_HEREDOC)
		{
			lseek(redirs->hd_fd, 0, SEEK_SET);
			dup2(redirs->hd_fd, STDIN_FILENO);
			close(redirs->hd_fd);
			redirs->hd_fd = -1;
		}
		else if (redirs->type == REDIR_DUP)
			dup2(ft_atoi(redirs->file + 1), redirs->fd);
		else
		{
			fd = open_redir(redirs);
			if (fd == -1)
			{
				put_errno(redirs->file, NULL);
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

static int	run_command(t_node *node, t_shell *shell)
{
	char	*path;
	char	**env_array;
	int		status;
	int		builtin_status;
	int		backup[3];
	t_saved	*saved;
	t_func	*func;
	int		nsaved;
	pid_t	pid;

	if (!node || node->type != NODE_CMD || !node->data.cmd.argv)
		return (1);
	shell->subst_used = 0;
	expand_command(node, shell);
	if (shell->expand_error)
	{
		shell->expand_error = 0;
		shell->running = shell->interactive;
		return (1);
	}
	if (!node->data.cmd.argv || !node->data.cmd.argv[0])
	{
		assign_shell(shell, node->data.cmd.assigns);
		return (shell->subst_used ? shell->subst_status : 0);
	}

	backup[0] = dup(STDIN_FILENO);
	backup[1] = dup(STDOUT_FILENO);
	backup[2] = dup(STDERR_FILENO);
	func = shell->bypass_func ? NULL : find_func(shell, node->data.cmd.argv[0]);
	shell->bypass_func = 0;
	if (func || is_builtin(node->data.cmd.argv[0]))
	{
		if (!apply_redirections(node->data.cmd.redirs))
		{
			restore_fds(backup);
			if (!func && is_special_builtin(node->data.cmd.argv[0]))
				shell->running = shell->interactive;
			return (1);
		}
		saved = node->data.cmd.assigns
			? overlay_apply(shell, node->data.cmd.assigns, &nsaved) : NULL;
		builtin_status = func ? call_function(shell, func, node->data.cmd.argv)
			: execute_builtin(node, shell);
		if (saved)
			overlay_restore(shell, saved, nsaved);
		restore_fds(backup);
		return (builtin_status);
	}

	path = find_path(node->data.cmd.argv[0], shell);
	if (!path)
	{
		if (ft_strchr(node->data.cmd.argv[0], '/'))
			ml_put_all(STDERR_FILENO, "minishell: ", node->data.cmd.argv[0],
				": No such file or directory\n", NULL);
		else
			ml_put_all(STDERR_FILENO, "minishell: ", node->data.cmd.argv[0],
				": command not found\n", NULL);
		restore_fds(backup);
		return (127);
	}

	saved = node->data.cmd.assigns
		? overlay_apply(shell, node->data.cmd.assigns, &nsaved) : NULL;
	env_array = env_to_array(shell->env);
	if (saved)
		overlay_restore(shell, saved, nsaved);

	pid = fork();
	if (pid == -1)
	{
		put_errno("fork", NULL);
		restore_fds(backup);
		free(path);
		free_str_array(env_array);
		return (1);
	}

	if (pid == 0)
	{
		job_child_setup(shell, 0, 1);
		close(backup[0]);
		close(backup[1]);
		close(backup[2]);
		if (!apply_redirections(node->data.cmd.redirs))
		{
			free(path);
			free_str_array(env_array);
			exit(1);
		}
		execve(path, node->data.cmd.argv, env_array);
		if (errno == EACCES && is_dir(path))
			ml_put_all(STDERR_FILENO, "minishell: ", node->data.cmd.argv[0],
				": Is a directory\n", NULL);
		else
			put_errno(node->data.cmd.argv[0], NULL);
		free(path);
		free_str_array(env_array);
		exit(126);
	}

	free(path);
	free_str_array(env_array);
	job_parent_setup(shell, pid, 0);
	status = job_wait(shell, node, &pid, 1);
	restore_fds(backup);
	return (status);
}

static int	pipe_child(t_node *node, t_shell *shell)
{
	int	status;

	status = execute_tree(node, shell);
	free_ast(shell->ast);
	free_tokens(shell->tokens);
	jobs_free(shell, 0);
	funcs_free(shell);
	free_str_array(shell->params);
	env_free(shell->aliases);
	env_free(shell->env);
	return (status);
}

static void	pipe_stage(t_node *node, t_shell *shell, int fds[2], int is_left)
{
	job_child_setup(shell, is_left ? 0 : getpgrp(), 1);
	if (is_left)
		dup2(fds[1], STDOUT_FILENO);
	else
		dup2(fds[0], STDIN_FILENO);
	close(fds[0]);
	close(fds[1]);
	exit(pipe_child(node, shell));
}

static int	execute_pipe(t_node *node, t_shell *shell)
{
	int		fds[2];
	pid_t	pids[2];
	int		status;

	if (pipe(fds) == -1)
		return (1);
	pids[0] = fork();
	if (pids[0] == 0)
		pipe_stage(node->data.op.left, shell, fds, 1);
	job_parent_setup(shell, pids[0], 0);
	pids[1] = fork();
	if (pids[1] == 0)
	{
		if (shell->interactive && !shell->in_child)
			setpgid(0, pids[0]);
		pipe_stage(node->data.op.right, shell, fds, 0);
	}
	job_parent_setup(shell, pids[1], pids[0]);
	close(fds[0]);
	close(fds[1]);
	status = job_wait(shell, node, pids, 2);
	return (status);
}

static int	execute_subshell(t_node *node, t_shell *shell)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
	{
		job_child_setup(shell, 0, 1);
		exit(pipe_child(node->data.op.left, shell));
	}
	job_parent_setup(shell, pid, 0);
	return (job_wait(shell, node, &pid, 1));
}

static int	execute_background(t_node *node, t_shell *shell)
{
	pid_t	pid;
	int		fd;

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
	{
		job_child_setup(shell, 0, 0);
		if (!shell->interactive)
		{
			fd = open("/dev/null", O_RDONLY);
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		exit(pipe_child(node->data.op.left, shell));
	}
	job_parent_setup(shell, pid, 0);
	shell->last_bg = pid;
	job_add_background(shell, node->data.op.left, pid);
	return (0);
}

static int	execute_command(t_node *node, t_shell *shell)
{
	t_node	*copy;
	int		status;

	if (!node || node->type != NODE_CMD)
		return (1);
	copy = clone_command(node);
	if (!copy)
		return (1);
	status = run_command(copy, shell);
	free_ast(copy);
	return (status);
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
		case NODE_OR:
		case NODE_SEQ:
			return (execute_chain(node, shell));
		case NODE_NOT:
			return (execute_tree(node->data.op.left, shell) == 0);
		case NODE_PAREN:
			return (execute_subshell(node, shell));
		case NODE_BG:
			return (execute_background(node, shell));
		default:
			if (is_ctl_node(node))
				return (execute_ctl(node, shell));
			ml_put_all(STDERR_FILENO, "minishell: unknown node type\n", NULL);
			return (1);
	}
}
