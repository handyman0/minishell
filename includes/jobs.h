#ifndef JOBS_H
# define JOBS_H

# include "minishell.h"
# include <sys/types.h>

typedef enum e_jobstate
{
	JOB_RUNNING,
	JOB_STOPPED
}	t_jobstate;

struct s_job
{
	int				id;
	pid_t			pgid;
	pid_t			pids[2];
	int				npids;
	int				status;
	t_jobstate		state;
	char			*cmd;
	struct s_job	*next;
};

void	jobs_init(t_shell *shell, int interactive);
void	jobs_free(t_shell *shell, int hangup);
void	jobs_notify(t_shell *shell);
void	job_child_setup(t_shell *shell, pid_t pgid, int foreground);
void	job_parent_setup(t_shell *shell, pid_t pid, pid_t pgid);
int		job_monitor(t_shell *shell);
int		job_wait(t_shell *shell, t_node *node, pid_t *pids, int n);
void	job_add_background(t_shell *shell, t_node *node, pid_t pid);
int		job_resume(t_shell *shell, t_job *job, int foreground);
t_job	*job_find(t_shell *shell, char *spec);
char	*node_label(t_node *node);
char	*job_line(t_shell *shell, t_job *job, const char *state);
int		builtin_jobs(t_shell *shell);
int		builtin_fg(char **argv, t_shell *shell);
int		builtin_bg(char **argv, t_shell *shell);

#endif
