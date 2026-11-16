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
	shell->running = shell->interactive;
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
	execve(path, argv, env);
	signals_ignore();
	free(path);
	free_str_array(env);
	(void)node;
	return (exec_fail(shell, argv[0], errno == ENOENT ? 127 : 126));
}

int	run_exec_builtin(t_node *node, t_shell *shell, int backup[3])
{
	char	**argv;
	int		status;

	argv = node->data.cmd.argv + 1;
	if (*argv && ft_strcmp(*argv, "--") == 0)
		argv++;
	if (!apply_redirections(node->data.cmd.redirs))
	{
		restore_fds(backup);
		shell->running = shell->interactive;
		return (1);
	}
	if (!*argv)
	{
		close(backup[0]);
		close(backup[1]);
		close(backup[2]);
		return (0);
	}
	status = exec_command(node, shell, argv);
	restore_fds(backup);
	return (status);
}
