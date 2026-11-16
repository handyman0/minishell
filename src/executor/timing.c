#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

static void	put_time(const char *label, double secs)
{
	char	*mins;
	char	*whole;
	char	*frac;
	int		ms;

	ms = (int)(secs * 1000 + 0.5);
	mins = ft_itoa(ms / 60000);
	whole = ft_itoa((ms / 1000) % 60);
	frac = ft_itoa(ms % 1000 + 1000);
	ml_put_all(2, label, "\t", mins, "m", whole, ".", frac + 1, "s\n", NULL);
	free(mins);
	free(whole);
	free(frac);
}

static double	now(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec + tv.tv_usec / 1e6);
}

static void	drop_first(char **argv)
{
	int	i;

	free(argv[0]);
	i = 0;
	while (argv[i])
	{
		argv[i] = argv[i + 1];
		i++;
	}
}

int	run_timed(t_node *node, t_shell *shell, int (*run)(t_node *, t_shell *))
{
	struct tms	before;
	struct tms	after;
	double		start;
	double		tick;
	int			status;

	drop_first(node->data.cmd.argv);
	times(&before);
	start = now();
	status = 0;
	if (node->data.cmd.argv[0])
		status = run(node, shell);
	times(&after);
	tick = (double)sysconf(_SC_CLK_TCK);
	ml_put_all(2, "\n", NULL);
	put_time("real", now() - start);
	put_time("user", (after.tms_cutime - before.tms_cutime) / tick
		+ (after.tms_utime - before.tms_utime) / tick);
	put_time("sys", (after.tms_cstime - before.tms_cstime) / tick
		+ (after.tms_stime - before.tms_stime) / tick);
	return (status);
}
