#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

static int	list_signals(char **names, t_shell *shell)
{
	int	n;
	int	i;

	(void)shell;
	if (!*names)
	{
		n = 1;
		while (n < 32)
		{
			ml_put_all(STDOUT_FILENO, n > 1 ? " " : "", signal_name(n), NULL);
			n++;
		}
		return (ml_put_all(STDOUT_FILENO, "\n", NULL), 0);
	}
	i = 0;
	while (names[i])
	{
		n = ft_atoi(names[i]);
		n = n > 128 ? n - 128 : n;
		if (n < 1 || n > 64 || ft_strcmp(signal_name(n), "?") == 0)
			return (ml_put_all(2, "minishell: kill: ", names[i],
					": invalid signal specification\n", NULL), 1);
		ml_put_all(STDOUT_FILENO, signal_name(n), "\n", NULL);
		i++;
	}
	return (0);
}

static int	parse_signal(char **argv, int *i, int *sig)
{
	char	*spec;

	spec = argv[*i];
	if (ft_strcmp(spec, "-s") == 0 || ft_strcmp(spec, "-n") == 0)
	{
		if (!argv[*i + 1])
			return (0);
		spec = argv[++(*i)];
	}
	else
		spec++;
	*sig = signal_number(spec);
	(*i)++;
	return (*sig >= 0);
}

static int	send_signal(char *target, int sig, t_shell *shell)
{
	pid_t	pid;
	int		i;

	i = target[0] == '-';
	while (ft_isdigit(target[i]))
		i++;
	if (target[0] == '%')
		pid = job_spec_pid(target, shell);
	else if (target[i] || !target[(target[0] == '-')])
		pid = 0;
	else
		pid = ft_atoi(target);
	if (pid == 0 || pid == -1 || kill(pid, sig) != 0)
		return (ml_put_all(2, "minishell: kill: ", target,
				": no such process or job\n", NULL), 1);
	return (0);
}

int	builtin_kill(char **argv, t_shell *shell)
{
	int	i;
	int	sig;
	int	status;

	i = 1;
	sig = SIGTERM;
	if (argv[i] && (ft_strcmp(argv[i], "-l") == 0
			|| ft_strcmp(argv[i], "-L") == 0))
		return (list_signals(argv + i + 1, shell));
	if (argv[i] && argv[i][0] == '-' && argv[i][1]
		&& ft_strcmp(argv[i], "--") != 0 && !parse_signal(argv, &i, &sig))
		return (ml_put_all(2, "minishell: kill: invalid signal\n", NULL), 1);
	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	if (!argv[i])
		return (ml_put_all(2, "minishell: kill: usage: kill [-s sig] pid...\n",
				NULL), 2);
	status = 0;
	while (argv[i])
		status |= send_signal(argv[i++], sig, shell);
	return (status);
}
