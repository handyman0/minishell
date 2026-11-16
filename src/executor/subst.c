#include "../../includes/executor.h"
#include "../../includes/signals.h"
#include "../../includes/jobs.h"
#include <sys/wait.h>
#include <errno.h>

static void	read_all(int fd, t_out *out)
{
	char	buf[4096];
	ssize_t	n;

	n = read(fd, buf, sizeof(buf));
	while (n != 0)
	{
		if (n < 0 && errno != EINTR)
			return ;
		if (n > 0)
			out_put(out, buf, n);
		n = read(fd, buf, sizeof(buf));
	}
}

static void	child_run(t_shell *shell, int fds[2], char *cmd, int reads)
{
	int	status;

	if (reads)
	{
		close(fds[1]);
		dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
	}
	else
	{
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
	}
	shell->in_child = 1;
	signals_child();
	trap_subshell_reset(shell);
	run_line(cmd, shell);
	trap_run_exit(shell);
	status = shell->exit_status;
	free(cmd);
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
	exit(status);
}

static char	*finish_output(t_out *out)
{
	char	*result;

	while (out->len > 0 && out->s[out->len - 1] == '\n')
		out->len--;
	result = malloc(out->len + 1);
	if (result)
	{
		ft_memcpy(result, out->s, out->len);
		result[out->len] = '\0';
	}
	free(out->s);
	return (result ? result : ft_strdup(""));
}

static void	collect(t_shell *shell, pid_t pid, int fd, t_out *out)
{
	int	status;

	if (!shell->in_child && shell->interactive)
		signals_ignore();
	read_all(fd, out);
	close(fd);
	while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
		;
	if (!shell->in_child && shell->interactive)
		setup_signals();
	shell->subst_used = 1;
	shell->subst_status = WIFSIGNALED(status) ? 128 + WTERMSIG(status)
		: WEXITSTATUS(status);
}

char	*run_substitution(t_shell *shell, char *cmd)
{
	t_out	out;
	int		fds[2];
	pid_t	pid;

	out.s = NULL;
	out.len = 0;
	out.cap = 0;
	if (pipe(fds) == -1)
		return (free(cmd), ft_strdup(""));
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), free(cmd), ft_strdup(""));
	if (pid == 0)
		child_run(shell, fds, cmd, 0);
	close(fds[1]);
	collect(shell, pid, fds[0], &out);
	free(cmd);
	return (finish_output(&out));
}

char	*run_procsub(t_shell *shell, char *cmd, int reads)
{
	int		fds[2];
	pid_t	pid;
	char	*num;
	char	*path;

	if (shell->ps_n >= PS_MAX || pipe(fds) == -1)
		return (free(cmd), ft_strdup(""));
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), free(cmd), ft_strdup(""));
	if (pid == 0)
		child_run(shell, fds, cmd, reads);
	close(fds[reads ? 0 : 1]);
	shell->ps_fd[shell->ps_n] = fds[reads ? 1 : 0];
	shell->ps_pid[shell->ps_n++] = pid;
	free(cmd);
	num = ft_itoa(fds[reads ? 1 : 0]);
	path = ft_strjoin("/dev/fd/", num);
	return (free(num), path);
}

void	procsub_cleanup(t_shell *shell)
{
	while (shell->ps_n > 0)
	{
		shell->ps_n--;
		close(shell->ps_fd[shell->ps_n]);
		bg_add(shell, shell->ps_pid[shell->ps_n]);
	}
}
