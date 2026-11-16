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

static int	job_matches(t_job *job, const char *s)
{
	if (ft_isdigit(*s))
		return (job->id == ft_atoi(s));
	if (*s == '?')
		return (ft_strnstr(job->cmd, s + 1, ft_strlen(job->cmd)) != NULL);
	return (ft_strncmp(job->cmd, s, ft_strlen(s)) == 0);
}

t_job	*job_find(t_shell *shell, char *spec)
{
	t_job	*job;
	t_job	*last;
	t_job	*before;
	t_job	*found;

	last = NULL;
	before = NULL;
	found = NULL;
	job = shell->jobs;
	if (spec && *spec == '%')
		spec++;
	while (job)
	{
		if (spec && *spec && *spec != '%' && *spec != '+' && *spec != '-'
			&& job_matches(job, spec))
			found = job;
		before = last;
		last = job;
		job = job->next;
	}
	if (!spec || !*spec || *spec == '%' || *spec == '+')
		return (last);
	if (*spec == '-')
		return (before);
	return (found);
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

static t_job	*pick_job(char *name, char *spec, t_shell *shell)
{
	t_job	*job;

	if (!job_monitor(shell))
	{
		ft_putstr_fd("minishell: ", STDERR_FILENO);
		ft_putstr_fd(name, STDERR_FILENO);
		ft_putendl_fd(": no job control", STDERR_FILENO);
		return (NULL);
	}
	job = job_find(shell, spec);
	if (!job)
	{
		ft_putstr_fd("minishell: ", STDERR_FILENO);
		ft_putstr_fd(name, STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
		ft_putstr_fd(spec ? spec : "current", STDERR_FILENO);
		ft_putendl_fd(": no such job", STDERR_FILENO);
	}
	return (job);
}

int	builtin_fg(char **argv, t_shell *shell)
{
	t_job	*job;

	job = pick_job("fg", argv[1], shell);
	if (!job)
		return (1);
	ft_putendl_fd(job->cmd, STDOUT_FILENO);
	return (job_resume(shell, job, 1));
}

static int	bg_one(char *spec, t_shell *shell)
{
	t_job	*job;
	char	*num;

	job = pick_job("bg", spec, shell);
	if (!job)
		return (1);
	job_resume(shell, job, 0);
	num = ft_itoa(job->id);
	ml_put_all(STDOUT_FILENO, "[", num, "] ", job->cmd, "\n", NULL);
	free(num);
	return (0);
}

int	builtin_bg(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	if (!argv[1])
		return (bg_one(NULL, shell));
	status = 0;
	i = 1;
	while (argv[i])
		status |= bg_one(argv[i++], shell);
	return (status);
}
