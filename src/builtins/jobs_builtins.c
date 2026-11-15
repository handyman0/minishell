#include "../../includes/jobs.h"
#include "../../includes/builtins.h"

static char	*cat(char *dst, const char *src)
{
	char	*out;

	out = ft_strjoin(dst, src ? src : "");
	free(dst);
	return (out);
}

static char	mark_of(t_shell *shell, t_job *job)
{
	t_job	*cur;
	t_job	*last;
	t_job	*before;

	last = NULL;
	before = NULL;
	cur = shell->jobs;
	while (cur)
	{
		before = last;
		last = cur;
		cur = cur->next;
	}
	if (job == last)
		return ('+');
	if (job == before)
		return ('-');
	return (' ');
}

char	*job_line(t_shell *shell, t_job *job, const char *state)
{
	char	*out;
	char	*num;
	char	mark[2];
	size_t	len;

	mark[0] = mark_of(shell, job);
	mark[1] = '\0';
	num = ft_itoa(job->id);
	out = cat(cat(cat(ft_strdup("["), num), "]"), mark);
	free(num);
	out = cat(cat(out, "  "), state);
	len = ft_strlen(state);
	while (len++ < 23)
		out = cat(out, " ");
	out = cat(out, job->cmd);
	if (ft_strcmp(state, "Running") == 0)
		out = cat(out, " &");
	return (cat(out, "\n"));
}

t_job	*job_find(t_shell *shell, char *spec)
{
	t_job	*job;
	t_job	*last;
	int		id;

	last = NULL;
	job = shell->jobs;
	if (spec && *spec == '%')
		spec++;
	id = spec ? ft_atoi(spec) : 0;
	while (job)
	{
		if (spec && job->id == id)
			return (job);
		last = job;
		job = job->next;
	}
	return (spec ? NULL : last);
}

int	builtin_jobs(t_shell *shell)
{
	t_job	*job;
	char	*line;

	jobs_notify(shell);
	job = shell->jobs;
	while (job)
	{
		line = job_line(shell, job,
				job->state == JOB_STOPPED ? "Stopped" : "Running");
		if (line)
			ft_putstr_fd(line, STDOUT_FILENO);
		free(line);
		job = job->next;
	}
	return (0);
}

static t_job	*pick_job(char *name, char **argv, t_shell *shell)
{
	t_job	*job;

	if (!shell->interactive || shell->in_child)
	{
		ft_putstr_fd("minishell: ", STDERR_FILENO);
		ft_putstr_fd(name, STDERR_FILENO);
		ft_putendl_fd(": no job control", STDERR_FILENO);
		return (NULL);
	}
	job = job_find(shell, argv[1]);
	if (!job)
	{
		ft_putstr_fd("minishell: ", STDERR_FILENO);
		ft_putstr_fd(name, STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
		ft_putstr_fd(argv[1] ? argv[1] : "current", STDERR_FILENO);
		ft_putendl_fd(": no such job", STDERR_FILENO);
	}
	return (job);
}

int	builtin_fg(char **argv, t_shell *shell)
{
	t_job	*job;

	job = pick_job("fg", argv, shell);
	if (!job)
		return (1);
	ft_putendl_fd(job->cmd, STDOUT_FILENO);
	return (job_resume(shell, job, 1));
}

int	builtin_bg(char **argv, t_shell *shell)
{
	t_job	*job;
	char	*line;

	job = pick_job("bg", argv, shell);
	if (!job)
		return (1);
	job_resume(shell, job, 0);
	line = job_line(shell, job, "Running");
	if (line)
		ft_putstr_fd(line, STDOUT_FILENO);
	free(line);
	return (0);
}
