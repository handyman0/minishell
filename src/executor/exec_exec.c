#include "../../includes/executor.h"
#include "../../includes/signals.h"
#include "../../includes/output.h"
#include <errno.h>

static int	exec_fail(t_shell *shell, const char *name, int status)
{
	if (status == 127)
		ml_put_all(2, "minishell: exec: ", name, ": not found\n", NULL);
	else
		put_errno("exec", name);
	shell->running = shell->spare;
	return (status);
}

static int	exec_command(t_node *node, t_shell *shell, char **argv)
{
	char	*path;
	char	**env;

	path = find_path(argv[0], shell);
	if (!path)
		return (exec_fail(shell, argv[0], 127));
	env = env_to_array(shell->env);
	signals_child();
	trap_subshell_reset(shell);
	execve(path, argv, env);
	exec_script_fallback(path, argv, env);
	signals_ignore();
	free(path);
	free_str_array(env);
	(void)node;
	return (exec_fail(shell, argv[0], errno == ENOENT ? 127 : 126));
}

int	run_exec_builtin(t_node *node, t_shell *shell, int backup[4])
{
	char	**argv;
	int		status;

	argv = node->data.cmd.argv + 1;
	if (*argv && ft_strcmp(*argv, "--") == 0)
		argv++;
	if (!apply_redirections(node->data.cmd.redirs))
	{
		restore_fds(backup);
		shell->running = shell->spare;
		return (1);
	}
	if (!*argv)
	{
		assign_shell(shell, node->data.cmd.assigns);
		forget_fds(backup);
		close(backup[0]);
		close(backup[1]);
		close(backup[2]);
		return (0);
	}
	status = exec_command(node, shell, argv);
	restore_fds(backup);
	return (status);
}

void	exec_script_fallback(char *path, char **argv, char **env)
{
	char	**args;
	int		n;
	int		i;

	if (errno != ENOEXEC)
		return ;
	n = 0;
	while (argv[n])
		n++;
	args = ft_calloc(n + 2, sizeof(char *));
	if (!args)
		return ;
	args[0] = "minishell";
	args[1] = path;
	i = 1;
	while (i < n)
	{
		args[i + 1] = argv[i];
		i++;
	}
	execve("/proc/self/exe", args, env);
	free(args);
	errno = ENOEXEC;
}
