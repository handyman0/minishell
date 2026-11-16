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

static void	errexit_check(t_shell *shell, int status)
{
	trap_run_err(shell, status);
	if (status != 0 && (shell->opts & OPT_E) && shell->noerrexit == 0
		&& !shell->spare)
	{
		shell->exit_status = status;
		shell->running = 0;
	}
}

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
	shell->noerrexit += depth > 0 && stack[depth - 1]->type != NODE_SEQ;
	status = execute_tree(cur, shell);
	shell->noerrexit -= depth > 0 && stack[depth - 1]->type != NODE_SEQ;
	while (depth-- > 0)
	{
		shell->noerrexit += depth > 0 && stack[depth - 1]->type != NODE_SEQ;
		status = chain_step(stack[depth], status, shell);
		shell->noerrexit -= depth > 0 && stack[depth - 1]->type != NODE_SEQ;
	}
	if (stack != local)
		free(stack);
	return (status);
}

int	execute_builtin(t_node *node, t_shell *shell)
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
		return (builtin_pwd(node->data.cmd.argv, shell));
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
	backup[0] = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 10);
	backup[1] = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 10);
	backup[2] = fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 10);
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


int	g_noclobber;

static int	open_out(t_redir *redir)
{
	struct stat	st;

	if (redir->type == REDIR_OUT && g_noclobber)
	{
		if (stat(redir->file, &st) == 0 && S_ISREG(st.st_mode))
		{
			errno = EEXIST;
			return (-1);
		}
		if (stat(redir->file, &st) == 0)
			return (open(redir->file, O_WRONLY));
		return (open(redir->file, O_WRONLY | O_CREAT | O_EXCL, 0644));
	}
	return (open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644));
}

static int	open_redir(t_redir *redir)
{
	if (redir->type == REDIR_IN)
		return (open(redir->file, O_RDONLY));
	if (redir->type == REDIR_RDWR)
		return (open(redir->file, O_RDWR | O_CREAT, 0666));
	if (redir->type == REDIR_OUT || redir->type == REDIR_CLOBBER)
		return (open_out(redir));
	return (open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0666));
}

static int	redir_fail(const char *what, const char *msg)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd((char *)what, 2);
	ft_putstr_fd(": ", 2);
	ft_putendl_fd((char *)msg, 2);
	return (0);
}

static int	apply_dup(t_redir *redir)
{
	int	src;
	int	mode;
	int	i;

	if (ft_strcmp(redir->file, "-") == 0)
		return (close(redir->fd), 1);
	i = 0;
	while (ft_isdigit(redir->file[i]))
		i++;
	if (i == 0 || redir->file[i])
		return (redir_fail(redir->file, "bad file descriptor"));
	src = ft_atoi(redir->file);
	mode = fcntl(src, F_GETFL);
	if (mode == -1)
		return (redir_fail(redir->file, "bad file descriptor"));
	mode &= O_ACCMODE;
	if ((redir->type == REDIR_DUP_IN && mode == O_WRONLY)
		|| (redir->type == REDIR_DUP && mode == O_RDONLY))
		return (redir_fail(redir->file, "bad file descriptor"));
	if (src != redir->fd && dup2(src, redir->fd) == -1)
		return (redir_fail(redir->file, "bad file descriptor"));
	return (1);
}

static int	apply_one(t_redir *redir)
{
	int	fd;

	if (redir->type == REDIR_HEREDOC || redir->type == REDIR_HERESTR)
	{
		lseek(redir->hd_fd, 0, SEEK_SET);
		if (redir->hd_fd != redir->fd)
			dup2(redir->hd_fd, redir->fd);
		if (redir->hd_fd != redir->fd)
			close(redir->hd_fd);
		redir->hd_fd = -1;
		return (1);
	}
	if (redir->type == REDIR_DUP || redir->type == REDIR_DUP_IN)
		return (apply_dup(redir));
	fd = open_redir(redir);
	if (fd == -1)
	{
		put_errno(redir->file, NULL);
		return (0);
	}
	if (fd != redir->fd)
	{
		dup2(fd, redir->fd);
		close(fd);
	}
	return (1);
}

int	apply_redirections(t_redir *redirs)
{
	while (redirs)
	{
		if (!apply_one(redirs))
			return (0);
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
	if (node->data.cmd.argv[0] && ft_strcmp(node->data.cmd.argv[0], "time") == 0
		&& !find_func(shell, "time"))
		return (run_timed(node, shell, run_command));
	if (node->data.cmd.argv[0] && ft_strcmp(node->data.cmd.argv[0], "[[") == 0)
		return (cond_command(node->data.cmd.argv, shell));
	if (node->data.cmd.argv[0] && node->data.cmd.argv[0][0] == '('
		&& node->data.cmd.argv[0][1] == '(' && !node->data.cmd.argv[1])
		return (arith_command(node->data.cmd.argv[0], shell));
	expand_command(node, shell);
	if (shell->expand_error)
	{
		shell->expand_error = 0;
		shell->running = shell->spare;
		return (1);
	}
	if (shell->opts & OPT_X)
		xtrace_print(shell, node->data.cmd.argv, node->data.cmd.assigns);
	if (!node->data.cmd.argv || !node->data.cmd.argv[0])
	{
		if (node->data.cmd.redirs)
		{
			save_fds(backup);
			status = apply_redirections(node->data.cmd.redirs);
			restore_fds(backup);
			if (!status)
				return (1);
		}
		if (assign_shell(shell, node->data.cmd.assigns))
			return (1);
		return (shell->subst_used ? shell->subst_status : 0);
	}
	if (!assigns_ok(shell, node->data.cmd.assigns))
		return (1);
	backup[0] = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 10);
	backup[1] = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 10);
	backup[2] = fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 10);
	if (ft_strcmp(node->data.cmd.argv[0], "exec") == 0)
		return (run_exec_builtin(node, shell, backup));
	func = shell->bypass_func || is_special_builtin(node->data.cmd.argv[0])
		? NULL : find_func(shell, node->data.cmd.argv[0]);
	shell->bypass_func = 0;
	if (func || is_builtin(node->data.cmd.argv[0]))
	{
		if (!apply_redirections(node->data.cmd.redirs))
		{
			restore_fds(backup);
			if (!func && is_special_builtin(node->data.cmd.argv[0]))
				shell->running = shell->spare;
			return (1);
		}
		saved = NULL;
		if (node->data.cmd.assigns && is_special_builtin(node->data.cmd.argv[0]))
			assign_shell(shell, node->data.cmd.assigns);
		else if (node->data.cmd.assigns)
			saved = overlay_apply(shell, node->data.cmd.assigns, &nsaved);
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
		exec_script_fallback(path, node->data.cmd.argv, env_array);
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

static t_node	*unwrap_subshell(t_node *node)
{
	if (node && node->type == NODE_PAREN && !node->data.ctl.redirs)
		return (node->data.ctl.a);
	return (node);
}

static int	pipe_child(t_node *node, t_shell *shell)
{
	int	status;

	status = execute_tree(node, shell);
	shell->exit_status = status;
	trap_run_exit(shell);
	status = shell->exit_status;
	free_ast(shell->ast);
	free_tokens(shell->tokens);
	jobs_free(shell, 0);
	funcs_free(shell);
	traps_free(shell);
	bgs_free(shell);
	free(shell->pipe_st);
	free_str_array(shell->params);
	env_free(shell->aliases);
	env_free(shell->env);
	return (status);
}

static void	pipe_stage(t_node *node, t_shell *shell, int fds[2], int mode)
{
	job_child_setup(shell, (mode & 1) ? 0 : getpgrp(), !(mode & 2));
	if (mode & 1)
		dup2(fds[1], STDOUT_FILENO);
	else
		dup2(fds[0], STDIN_FILENO);
	close(fds[0]);
	close(fds[1]);
	exit(pipe_child(unwrap_subshell(node), shell));
}

static int	spawn_pipe(t_node *node, t_shell *shell, pid_t pids[2], int bg)
{
	int	fds[2];

	if (pipe(fds) == -1)
		return (0);
	pids[0] = fork();
	if (pids[0] == 0)
		pipe_stage(node->data.op.left, shell, fds, 1 | (bg << 1));
	job_parent_setup(shell, pids[0], 0);
	pids[1] = fork();
	if (pids[1] == 0)
	{
		if (job_monitor(shell))
			setpgid(0, pids[0]);
		pipe_stage(node->data.op.right, shell, fds, 0 | (bg << 1));
	}
	job_parent_setup(shell, pids[1], pids[0]);
	close(fds[0]);
	close(fds[1]);
	return (1);
}

static int	collect_stages(t_node *node, t_node **stages, int max)
{
	int	n;

	if (node->type != NODE_PIPE)
	{
		if (max > 0)
			stages[0] = node;
		return (1);
	}
	n = collect_stages(node->data.op.left, stages, max);
	return (n + collect_stages(node->data.op.right, stages + n,
			max > n ? max - n : 0));
}

static void	stage_child(t_node *stage, t_shell *shell, pid_t pgid)
{
	if (job_monitor(shell) && pgid)
		setpgid(0, pgid);
	job_child_setup(shell, pgid ? getpgrp() : 0, 1);
	exit(pipe_child(unwrap_subshell(stage), shell));
}

static void	stage_fds(int io[2], int fds[2], int prev)
{
	if (io[0] != -1)
		dup2(io[0], STDIN_FILENO);
	if (io[1] != -1)
		dup2(io[1], STDOUT_FILENO);
	if (prev != -1)
		close(prev);
	if (fds[0] != -1)
		close(fds[0]);
	if (fds[1] != -1)
		close(fds[1]);
}

void	set_pipestatus(t_shell *shell, const int *codes, int n)
{
	int	*grown;

	if (n > shell->pipe_cap)
	{
		grown = ft_calloc(n + 1, sizeof(int));
		if (!grown)
			return ;
		free(shell->pipe_st);
		shell->pipe_st = grown;
		shell->pipe_cap = n;
	}
	shell->npipe = n;
	ft_memcpy(shell->pipe_st, codes, sizeof(int) * n);
}

static int	spawn_stages(t_node **stages, int n, t_shell *shell, pid_t *pids)
{
	int	fds[2];
	t_node	*stage;
	pid_t	pgid;
	int	prev;
	int	io[2];
	int	i;

	prev = -1;
	i = -1;
	while (++i < n)
	{
		fds[0] = -1;
		fds[1] = -1;
		if (i < n - 1 && pipe(fds) == -1)
			return (i);
		pids[i] = fork();
		if (pids[i] == 0)
		{
			io[0] = prev;
			io[1] = fds[1];
			stage_fds(io, fds, prev);
			pgid = i ? pids[0] : 0;
			free(pids);
			stage = stages[i];
			free(stages);
			stage_child(stage, shell, pgid);
		}
		job_parent_setup(shell, pids[i], i ? pids[0] : 0);
		if (prev != -1)
			close(prev);
		if (fds[1] != -1)
			close(fds[1]);
		prev = fds[0];
	}
	return (n);
}

static void	pipefail(t_shell *shell, const int *codes, int n, int *status)
{
	if (!(shell->opts & OPT_PIPEFAIL) || *status == 128 + SIGTSTP)
		return ;
	*status = 0;
	while (n-- > 0)
	{
		if (codes[n] != 0)
		{
			*status = codes[n];
			return ;
		}
	}
}

static int	execute_pipe(t_node *node, t_shell *shell)
{
	t_node	**stages;
	pid_t	*pids;
	int		*codes;
	int		n;
	int		status;

	n = collect_stages(node, NULL, 0);
	stages = malloc(sizeof(t_node *) * n);
	pids = ft_calloc(n + 1, sizeof(pid_t));
	if (!stages || !pids)
		return (free(stages), free(pids), 1);
	collect_stages(node, stages, n);
	if (spawn_stages(stages, n, shell, pids) != n)
		return (free(stages), free(pids), 1);
	codes = ft_calloc(n + 1, sizeof(int));
	if (!codes)
		return (free(stages), free(pids), 1);
	status = job_wait_codes(shell, node, pids, n, codes);
	set_pipestatus(shell, codes, n);
	pipefail(shell, codes, n, &status);
	free(stages);
	free(pids);
	free(codes);
	errexit_check(shell, status);
	return (status);
}

int	execute_subshell(t_node *node, t_shell *shell)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
	{
		job_child_setup(shell, 0, 1);
		exit(pipe_child(node->data.ctl.a, shell));
	}
	job_parent_setup(shell, pid, 0);
	status = job_wait(shell, node, &pid, 1);
	errexit_check(shell, status);
	return (status);
}

static int	background_pipe(t_node *node, t_shell *shell)
{
	pid_t	pids[2];

	if (!spawn_pipe(node->data.op.left, shell, pids, 1))
		return (1);
	shell->last_bg = pids[1];
	bg_add(shell, pids[1]);
	job_add_background(shell, node->data.op.left, pids[0], pids[1]);
	return (0);
}

static int	execute_background(t_node *node, t_shell *shell)
{
	pid_t	pid;
	int		fd;

	if (job_monitor(shell) && node->data.op.left->type == NODE_PIPE)
		return (background_pipe(node, shell));

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
	{
		job_child_setup(shell, 0, 0);
		if (!job_monitor(shell))
		{
			fd = open("/dev/null", O_RDONLY);
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		exit(pipe_child(unwrap_subshell(node->data.op.left), shell));
	}
	job_parent_setup(shell, pid, 0);
	shell->last_bg = pid;
	bg_add(shell, pid);
	job_add_background(shell, node->data.op.left, pid, pid);
	return (0);
}

static int	execute_command(t_node *node, t_shell *shell)
{
	t_node	*copy;
	int		status;

	if (!node || node->type != NODE_CMD)
		return (1);
	if ((shell->opts & OPT_N) && !shell->spare)
		return (0);
	if (node->data.cmd.line > 0)
		shell->lineno = node->data.cmd.line;
	trap_run_debug(shell);
	copy = clone_command(node);
	if (!copy)
		return (1);
	status = run_command(copy, shell);
	procsub_cleanup(shell);
	shell->exit_status = status;
	set_pipestatus(shell, &status, 1);
	errexit_check(shell, status);
	trap_run_pending(shell);
	free_ast(copy);
	return (shell->running ? status : shell->exit_status);
}

int	execute_tree(t_node *node, t_shell *shell)
{
	int	status;

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
			shell->noerrexit++;
			status = execute_tree(node->data.op.left, shell);
			shell->noerrexit--;
			return (status == 0);
		case NODE_BG:
			return (execute_background(node, shell));
		default:
			if (is_ctl_node(node))
				return (execute_ctl(node, shell));
			ml_put_all(STDERR_FILENO, "minishell: unknown node type\n", NULL);
			return (1);
	}
}
