#include "../../includes/jobs.h"
#include "../../includes/signals.h"
#include <sys/wait.h>
#include <errno.h>

static int	job_control(t_shell *shell)
{
	return (shell->interactive && !shell->in_child);
}

void	jobs_init(t_shell *shell, int interactive)
{
	shell->jobs = NULL;
	shell->next_job_id = 1;
	shell->in_child = 0;
	shell->interactive = interactive && isatty(STDIN_FILENO);
	shell->pgid = getpgrp();
	if (!shell->interactive)
		return ;
	if (tcgetpgrp(STDIN_FILENO) == -1)
	{
		shell->interactive = 0;
		return ;
	}
	while (tcgetpgrp(STDIN_FILENO) != getpgrp())
		kill(-getpgrp(), SIGTTIN);
	setpgid(0, 0);
	shell->pgid = getpgrp();
	tcsetpgrp(STDIN_FILENO, shell->pgid);
	tcgetattr(STDIN_FILENO, &shell->tmodes);
}

void	job_child_setup(t_shell *shell, pid_t pgid, int foreground)
{
	if (job_control(shell))
	{
		setpgid(0, pgid);
		if (foreground)
			tcsetpgrp(STDIN_FILENO, getpgrp());
	}
	shell->in_child = 1;
	signals_child();
	trap_subshell_reset(shell);
	bgs_free(shell);
}

void	job_parent_setup(t_shell *shell, pid_t pid, pid_t pgid)
{
	if (job_control(shell))
		setpgid(pid, pgid ? pgid : pid);
}

static pid_t	retry_wait(pid_t pid, int *status, int flags)
{
	pid_t	r;

	r = waitpid(pid, status, flags);
	while (r < 0 && errno == EINTR)
		r = waitpid(pid, status, flags);
	return (r);
}

static int	wait_pids(t_job *job, int flags, int *stopped, int *last)
{
	int	status;
	int	i;

	i = 0;
	while (i < job->npids)
	{
		if (job->pids[i] > 0 && retry_wait(job->pids[i], &status, flags) > 0)
		{
			if (WIFSTOPPED(status))
				*stopped = 1;
			else
				job->pids[i] = 0;
			if (i == job->npids - 1)
				*last = status;
		}
		i++;
	}
	return (*stopped);
}

static void	give_terminal(t_shell *shell, pid_t pgid, int restore_modes)
{
	tcsetpgrp(STDIN_FILENO, pgid);
	if (restore_modes)
		tcsetattr(STDIN_FILENO, TCSADRAIN, &shell->tmodes);
}

static int	wait_job(t_shell *shell, t_job *job, int resume)
{
	int	stopped;
	int	last;
	int	jc;

	jc = job_control(shell);
	stopped = 0;
	last = 0;
	if (!shell->in_child && shell->interactive)
		signals_ignore();
	if (jc)
		give_terminal(shell, job->pgid, 0);
	if (resume)
		kill(-job->pgid, SIGCONT);
	wait_pids(job, jc ? WUNTRACED : 0, &stopped, &last);
	if (jc)
		give_terminal(shell, shell->pgid, 1);
	if (!shell->in_child && shell->interactive)
		setup_signals();
	if (stopped)
		return (job->state = JOB_STOPPED, 128 + SIGTSTP);
	if (WIFSIGNALED(last) && WTERMSIG(last) == SIGINT)
		write(STDERR_FILENO, "\n", 1);
	if (WIFSIGNALED(last))
		return (128 + WTERMSIG(last));
	return (WEXITSTATUS(last));
}

static void	announce(t_shell *shell, t_job *job, const char *state)
{
	char	*line;

	line = job_line(shell, job, state);
	if (line)
	{
		ft_putstr_fd(line, STDERR_FILENO);
		free(line);
	}
}

static t_job	*job_store(t_shell *shell, t_job *tmp, char *label)
{
	t_job	*job;
	t_job	*last;

	job = malloc(sizeof(t_job));
	if (!job)
		return (free(label), NULL);
	*job = *tmp;
	job->id = shell->next_job_id++;
	job->cmd = label;
	job->next = NULL;
	if (!shell->jobs)
		shell->jobs = job;
	else
	{
		last = shell->jobs;
		while (last->next)
			last = last->next;
		last->next = job;
	}
	return (job);
}

int	job_wait(t_shell *shell, t_node *node, pid_t *pids, int n)
{
	t_job	tmp;
	t_job	*job;
	int		status;

	ft_bzero(&tmp, sizeof(t_job));
	tmp.pgid = pids[0];
	tmp.npids = n;
	while (n-- > 0)
		tmp.pids[n] = pids[n];
	status = wait_job(shell, &tmp, 0);
	if (tmp.state == JOB_STOPPED)
	{
		job = job_store(shell, &tmp, node_label(node));
		if (job)
		{
			write(STDERR_FILENO, "\n", 1);
			announce(shell, job, "Stopped");
			job->status = -1;
		}
	}
	return (status);
}

void	job_add_background(t_shell *shell, t_node *node, pid_t pid)
{
	t_job	tmp;
	t_job	*job;
	char	*num;

	if (!job_control(shell))
		return ;
	ft_bzero(&tmp, sizeof(t_job));
	tmp.pgid = pid;
	tmp.pids[0] = pid;
	tmp.npids = 1;
	job = job_store(shell, &tmp, node_label(node));
	if (!job)
		return ;
	num = ft_itoa(job->id);
	ft_putstr_fd("[", STDERR_FILENO);
	ft_putstr_fd(num, STDERR_FILENO);
	ft_putstr_fd("] ", STDERR_FILENO);
	free(num);
	num = ft_itoa(pid);
	ft_putendl_fd(num, STDERR_FILENO);
	free(num);
}

static void	job_remove(t_shell *shell, t_job *job)
{
	t_job	**link;

	link = &shell->jobs;
	while (*link && *link != job)
		link = &(*link)->next;
	if (*link)
		*link = job->next;
	free(job->cmd);
	free(job);
}

int	job_resume(t_shell *shell, t_job *job, int foreground)
{
	int	status;

	job->state = JOB_RUNNING;
	job->status = 0;
	if (!foreground)
		return (kill(-job->pgid, SIGCONT), 0);
	status = wait_job(shell, job, 1);
	if (job->state == JOB_STOPPED)
	{
		write(STDERR_FILENO, "\n", 1);
		announce(shell, job, "Stopped");
		job->status = -1;
	}
	else
		job_remove(shell, job);
	return (status);
}

static int	exit_code(int raw)
{
	if (WIFSIGNALED(raw))
		return (128 + WTERMSIG(raw));
	return (WEXITSTATUS(raw));
}

static int	poll_job(t_shell *shell, t_job *job)
{
	int	raw;
	int	i;
	int	alive;

	alive = 0;
	i = 0;
	while (i < job->npids)
	{
		if (job->pids[i] > 0
			&& waitpid(job->pids[i], &raw, WNOHANG | WUNTRACED) > 0)
		{
			if (WIFSTOPPED(raw))
				job->state = JOB_STOPPED;
			else
			{
				bg_mark(shell, job->pids[i], exit_code(raw));
				job->pids[i] = 0;
				if (i == job->npids - 1)
					job->status = exit_code(raw);
			}
		}
		alive += job->pids[i] > 0;
		i++;
	}
	return (alive == 0);
}

static void	announce_done(t_shell *shell, t_job *job)
{
	char	*num;
	char	*state;

	if (job->status == 0)
		return (announce(shell, job, "Done"));
	num = ft_itoa(job->status);
	state = ft_strjoin("Exit ", num);
	announce(shell, job, state);
	free(num);
	free(state);
}

void	jobs_notify(t_shell *shell)
{
	t_job	*job;
	t_job	*next;

	job = shell->jobs;
	while (job)
	{
		next = job->next;
		if (job->state == JOB_RUNNING && poll_job(shell, job))
		{
			announce_done(shell, job);
			job_remove(shell, job);
		}
		else if (job->state == JOB_STOPPED && job->status == 0)
		{
			job->status = -1;
			announce(shell, job, "Stopped");
		}
		job = next;
	}
}

void	jobs_free(t_shell *shell, int hangup)
{
	t_job	*job;
	t_job	*next;

	job = shell->jobs;
	while (job)
	{
		next = job->next;
		if (hangup && job->state == JOB_STOPPED)
		{
			kill(-job->pgid, SIGHUP);
			kill(-job->pgid, SIGCONT);
		}
		free(job->cmd);
		free(job);
		job = next;
	}
	shell->jobs = NULL;
}
