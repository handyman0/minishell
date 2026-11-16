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

static void	child_run(t_shell *shell, int fds[2], char *cmd)
{
	int	status;

	close(fds[0]);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	shell->in_child = 1;
	signals_child();
	run_line(cmd, shell);
	status = shell->exit_status;
	free(cmd);
	free_ast(shell->ast);
	free_tokens(shell->tokens);
	jobs_free(shell, 0);
	env_free(shell->aliases);
	env_free(shell->env);
	exit(status);
}

static char	*finish_output(t_out *out)
{
	char	*result;

	while (out->len > 0 && out->s[out->len - 1] == '\n')
		out->len--;
	if (!out->s)
		return (ft_strdup(""));
	result = ft_substr(out->s, 0, out->len);
	free(out->s);
	return (result);
}

static void	collect(t_shell *shell, pid_t pid, int fd, t_out *out)
{
	int	status;

	if (!shell->in_child)
		signals_ignore();
	read_all(fd, out);
	close(fd);
	waitpid(pid, &status, 0);
	if (!shell->in_child)
	{
		if (shell->interactive)
			setup_signals();
		else
			setup_signals_noninteractive();
	}
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
		child_run(shell, fds, cmd);
	close(fds[1]);
	collect(shell, pid, fds[0], &out);
	free(cmd);
	return (finish_output(&out));
}
