#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include "../includes/executor.h"
#include "../includes/output.h"

static void	not_found(char *cmd, char *name)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(name, STDERR_FILENO);
	ft_putendl_fd(": not found", STDERR_FILENO);
}

static int	describe_type(char *name, t_shell *shell)
{
	char	*alias;
	char	*path;

	alias = env_get(shell->aliases, name);
	if (alias)
		return (put_all(STDOUT_FILENO, name, " is aliased to `", alias, "'\n", NULL), 0);
	if (is_builtin(name))
		return (put_all(STDOUT_FILENO, name, " is a shell builtin\n", NULL), 0);
	path = find_executable(name, shell);
	if (!path)
		return (not_found("type", name), 1);
	put_all(STDOUT_FILENO, name, " is ", path, "\n", NULL);
	free(path);
	return (0);
}

static int	describe_which(char *name, t_shell *shell)
{
	char	*alias;
	char	*path;

	alias = env_get(shell->aliases, name);
	if (alias)
		return (put_all(STDOUT_FILENO, name, ": aliased to ", alias, "\n", NULL), 0);
	if (is_builtin(name))
		return (put_all(STDOUT_FILENO, name, ": shell built-in command\n", NULL), 0);
	path = find_executable(name, shell);
	if (!path)
		return (not_found("which", name), 1);
	put_all(STDOUT_FILENO, path, "\n", NULL);
	free(path);
	return (0);
}

int	builtin_type(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	status = 0;
	i = 1;
	while (argv[i])
		status |= describe_type(argv[i++], shell);
	return (status);
}

int	builtin_which(char **argv, t_shell *shell)
{
	int	status;
	int	i;

	if (!argv[1])
		return (1);
	status = 0;
	i = 1;
	while (argv[i])
		status |= describe_which(argv[i++], shell);
	return (status);
}
