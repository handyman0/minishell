#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include "../../includes/signals.h"
#include <sys/wait.h>
#include <errno.h>

#define BG_KEEP 1024

static int	g_bg_id = 0;

static t_bg	*bg_find(t_shell *shell, pid_t pid)
{
	t_bg	*bg;

	bg = shell->bgs;
	while (bg && bg->pid != pid)
		bg = bg->next;
	return (bg);
}

static void	bg_prune(t_shell *shell)
{
	t_bg	*bg;
	t_bg	*next;
	int		count;

	count = 0;
	bg = shell->bgs;
	while (bg && ++count < BG_KEEP)
		bg = bg->next;
	while (bg && bg->next)
	{
		next = bg->next;
		if (next->done)
		{
			bg->next = next->next;
			free(next);
		}
		else
			bg = next;
	}
}

void	bg_add(t_shell *shell, pid_t pid)
{
	t_bg	*bg;

	bg = ft_calloc(1, sizeof(t_bg));
	if (!bg)
		return ;
	bg->pid = pid;
	bg->id = ++g_bg_id;
	bg->next = shell->bgs;
	shell->bgs = bg;
	bg_prune(shell);
}

void	bg_mark(t_shell *shell, pid_t pid, int status)
{
	t_bg	*bg;

	bg = bg_find(shell, pid);
	if (!bg)
		return ;
	bg->done = 1;
	bg->status = status;
}

static int	decode(int raw)
{
	if (WIFSIGNALED(raw))
		return (128 + WTERMSIG(raw));
	return (WEXITSTATUS(raw));
}

void	bg_reap(t_shell *shell)
{
	t_bg	*bg;
	int		raw;

	bg = shell->bgs;
	while (bg)
	{
		if (!bg->done && waitpid(bg->pid, &raw, WNOHANG) > 0)
		{
			bg->done = 1;
			bg->status = decode(raw);
		}
		bg = bg->next;
	}
}

void	bgs_free(t_shell *shell)
{
	t_bg	*next;

	g_bg_id = 0;
	while (shell->bgs)
	{
		next = shell->bgs->next;
		free(shell->bgs);
		shell->bgs = next;
	}
}

static int	interrupted(t_shell *shell)
{
	int	sig;

	sig = 1;
	while (sig < 65)
	{
		if (g_trap_pending[sig] && shell->traps[sig])
		{
			trap_run_pending(shell);
			return (128 + sig);
		}
		sig++;
	}
	return (0);
}

static int	wait_for(t_shell *shell, t_bg *bg)
{
	int	raw;
	int	stop;

	while (!bg->done)
	{
		if (waitpid(bg->pid, &raw, 0) == bg->pid)
		{
			bg->done = 1;
			bg->status = decode(raw);
		}
		else if (errno == EINTR)
		{
			stop = interrupted(shell);
			if (stop)
				return (stop);
		}
		else
			bg->done = 1;
	}
	return (-1);
}

static pid_t	job_target(char *arg, t_shell *shell)
{
	t_job	*job;
	t_bg	*bg;
	int		id;

	job = job_find(shell, arg);
	if (job)
		return (job->pids[job->npids - 1] ? job->pids[job->npids - 1]
			: job->pgid);
	id = (arg[1] == '%' || arg[1] == '+' || !arg[1]) ? 0 : ft_atoi(arg + 1);
	bg = shell->bgs;
	while (bg && id && bg->id != id)
		bg = bg->next;
	return (bg ? bg->pid : -1);
}

pid_t	job_spec_pid(char *arg, t_shell *shell)
{
	int	i;

	if (arg[0] == '%')
		return (job_target(arg, shell));
	i = 0;
	while (ft_isdigit(arg[i]))
		i++;
	if (arg[i] || i == 0)
		return (-1);
	return (ft_atoi(arg));
}

static int	wait_all(t_shell *shell)
{
	t_bg	*bg;
	int		stop;

	bg = shell->bgs;
	while (bg)
	{
		if (!bg->done)
		{
			stop = wait_for(shell, bg);
			if (stop >= 0)
				return (stop);
			bg = shell->bgs;
			continue ;
		}
		bg = bg->next;
	}
	return (0);
}

int	builtin_wait(char **argv, t_shell *shell)
{
	t_bg	*bg;
	pid_t	pid;
	int		status;
	int		stop;
	int		i;

	if (!argv[1])
		return (wait_all(shell));
	status = 0;
	i = 1;
	while (argv[i])
	{
		pid = job_spec_pid(argv[i++], shell);
		bg = pid > 0 ? bg_find(shell, pid) : NULL;
		if (!bg)
		{
			status = 127;
			continue ;
		}
		stop = wait_for(shell, bg);
		if (stop >= 0)
			return (stop);
		status = bg->status;
	}
	return (status);
}

void	bg_forget(t_shell *shell, pid_t pid)
{
	t_bg	**link;
	t_bg	*gone;

	link = &shell->bgs;
	while (*link && (*link)->pid != pid)
		link = &(*link)->next;
	if (!*link)
		return ;
	gone = *link;
	*link = gone->next;
	free(gone);
}
