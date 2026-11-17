#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include "../../includes/signals.h"

typedef struct s_signame
{
	const char	*name;
	int			num;
}	t_signame;

static const t_signame	g_sigs[] = {{"EXIT", 0}, {"HUP", SIGHUP},
{"INT", SIGINT}, {"QUIT", SIGQUIT}, {"ILL", SIGILL}, {"TRAP", SIGTRAP},
{"ABRT", SIGABRT}, {"BUS", SIGBUS}, {"FPE", SIGFPE}, {"KILL", SIGKILL},
{"USR1", SIGUSR1}, {"SEGV", SIGSEGV}, {"USR2", SIGUSR2}, {"PIPE", SIGPIPE},
{"ALRM", SIGALRM}, {"TERM", SIGTERM}, {"CHLD", SIGCHLD}, {"CONT", SIGCONT},
{"STOP", SIGSTOP}, {"TSTP", SIGTSTP}, {"TTIN", SIGTTIN}, {"TTOU", SIGTTOU},
{"URG", SIGURG}, {"XCPU", SIGXCPU}, {"XFSZ", SIGXFSZ},
{"VTALRM", SIGVTALRM}, {"PROF", SIGPROF}, {"WINCH", SIGWINCH}, {"IO", SIGIO},
{"SYS", SIGSYS}, {"ERR", 65}, {"DEBUG", 66}, {NULL, 0}};

static t_shell	*g_trap_shell = NULL;

int	signal_number(const char *s)
{
	int	i;
	int	n;

	if (ft_isdigit(s[0]))
	{
		n = 0;
		i = 0;
		while (ft_isdigit(s[i]))
			n = n * 10 + (s[i++] - '0');
		return (s[i] || n > 64 ? -1 : n);
	}
	if (ft_strncmp(s, "SIG", 3) == 0)
		s += 3;
	i = 0;
	while (g_sigs[i].name)
	{
		if (ft_strcmp(s, g_sigs[i].name) == 0)
			return (g_sigs[i].num);
		i++;
	}
	return (-1);
}

const char	*signal_name(int num)
{
	int	i;

	i = 0;
	while (g_sigs[i].name)
	{
		if (g_sigs[i].num == num)
			return (g_sigs[i].name);
		i++;
	}
	return ("?");
}

static void	default_disposition(t_shell *shell, int sig)
{
	int	managed;

	managed = (sig == SIGINT || sig == SIGQUIT || sig == SIGTSTP
			|| sig == SIGTTIN || sig == SIGTTOU);
	if (managed && shell->interactive && !shell->in_child)
		signal(sig, SIG_IGN);
	else
		signal(sig, SIG_DFL);
}

static void	install(t_shell *shell, int sig)
{
	struct sigaction	sa;
	char				*action;

	action = shell->trap_inherited[sig] ? NULL : shell->traps[sig];
	if (sig == 0 || sig >= 65 || sig == SIGKILL || sig == SIGSTOP)
		return ;
	if (!action)
		return (default_disposition(shell, sig));
	if (!*action)
		return ((void)signal(sig, SIG_IGN));
	ft_bzero(&sa, sizeof(sa));
	sa.sa_handler = trap_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, NULL);
}

void	trap_reapply(void)
{
	int	sig;

	if (!g_trap_shell)
		return ;
	sig = 1;
	while (sig < 65)
	{
		if (g_trap_shell->traps[sig])
			install(g_trap_shell, sig);
		sig++;
	}
}

static void	print_traps(t_shell *shell)
{
	char	*quoted;
	int		sig;

	sig = 0;
	while (sig < 67)
	{
		if (shell->traps[sig])
		{
			quoted = quote_word(shell->traps[sig]);
			ml_put_all(STDOUT_FILENO, "trap -- ", quoted, " ",
				signal_name(sig), "\n", NULL);
			free(quoted);
		}
		sig++;
	}
}

static int	set_trap(t_shell *shell, int sig, const char *action, int reset)
{
	if (sig == SIGKILL || sig == SIGSTOP)
		return (0);
	shell->trap_inherited[sig] = 0;
	free(shell->traps[sig]);
	shell->traps[sig] = reset ? NULL : ft_strdup(action);
	g_trap_shell = shell;
	install(shell, sig);
	return (0);
}

static int	all_digits(const char *s)
{
	if (!*s)
		return (0);
	while (*s)
		if (!ft_isdigit(*s++))
			return (0);
	return (1);
}

int	builtin_trap(char **argv, t_shell *shell)
{
	const char	*action;
	int			i;
	int			reset;
	int			status;
	int			sig;

	i = 1;
	g_trap_shell = shell;
	if (argv[i] && ft_strcmp(argv[i], "--") == 0)
		i++;
	if (!argv[i] || (ft_strcmp(argv[i], "-p") == 0 && !argv[i + 1]))
		return (print_traps(shell), 0);
	reset = all_digits(argv[i]) || ft_strcmp(argv[i], "-") == 0;
	action = "";
	if (!all_digits(argv[i]))
		action = argv[i++];
	if (!reset && !argv[i])
		return (ml_put_all(2, "minishell: trap: usage: trap action condition\n",
				NULL), 2);
	status = 0;
	while (argv[i])
	{
		sig = signal_number(argv[i]);
		if (sig < 0)
			status |= (ml_put_all(2, "minishell: trap: ", argv[i],
						": invalid signal specification\n", NULL), 1);
		else
			status |= set_trap(shell, sig, action, reset);
		i++;
	}
	return (status);
}

static void	run_action(t_shell *shell, const char *action)
{
	char	*text;
	int		saved;
	int		prev;

	text = ft_strdup(action);
	saved = shell->exit_status;
	prev = shell->trap_status;
	shell->trap_status = saved;
	shell->in_trap++;
	run_line(text, shell);
	shell->in_trap--;
	shell->trap_status = prev;
	if (shell->running)
		shell->exit_status = saved;
	free(text);
}

void	trap_run_pending(t_shell *shell)
{
	int	sig;

	if (shell->in_trap)
		return ;
	sig = 1;
	while (sig < 65)
	{
		if (g_trap_pending[sig])
		{
			g_trap_pending[sig] = 0;
			if (shell->traps[sig] && *shell->traps[sig] && shell->running
				&& !shell->trap_inherited[sig])
				run_action(shell, shell->traps[sig]);
		}
		sig++;
	}
}

void	trap_run_exit(t_shell *shell)
{
	char	*action;
	int		saved;
	int		prev;

	if (!shell->traps[0] || !*shell->traps[0] || shell->trap_inherited[0])
		return ;
	action = shell->traps[0];
	shell->traps[0] = NULL;
	saved = shell->exit_status;
	shell->running = 1;
	prev = shell->trap_status;
	shell->trap_status = saved;
	shell->in_trap++;
	run_line(action, shell);
	shell->in_trap--;
	shell->trap_status = prev;
	if (shell->running)
		shell->exit_status = saved;
	free(action);
}

void	trap_subshell_reset(t_shell *shell)
{
	int	sig;

	sig = 0;
	while (sig < 67)
	{
		if (shell->traps[sig] && *shell->traps[sig])
			shell->trap_inherited[sig] = 1;
		if (shell->traps[sig])
			install(shell, sig);
		sig++;
	}
}

void	traps_free(t_shell *shell)
{
	int	sig;

	sig = 0;
	while (sig < 67)
		free(shell->traps[sig++]);
	ft_bzero(shell->traps, sizeof(shell->traps));
}

static void	run_special(t_shell *shell, int slot)
{
	char	*action;
	int		saved;
	int		prev;

	if (shell->in_trap || !shell->traps[slot] || !*shell->traps[slot]
		|| shell->trap_inherited[slot])
		return ;
	action = ft_strdup(shell->traps[slot]);
	saved = shell->exit_status;
	prev = shell->trap_status;
	shell->trap_status = saved;
	shell->in_trap++;
	run_line(action, shell);
	shell->in_trap--;
	shell->trap_status = prev;
	if (shell->running)
		shell->exit_status = saved;
	free(action);
}

void	trap_run_err(t_shell *shell, int status)
{
	if (status != 0 && shell->noerrexit == 0)
		run_special(shell, 65);
}

void	trap_run_debug(t_shell *shell)
{
	run_special(shell, 66);
}
