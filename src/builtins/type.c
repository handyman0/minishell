#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include "../includes/executor.h"

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
		return (ft_printf("%s is aliased to `%s'\n", name, alias), 0);
	if (is_builtin(name))
		return (ft_printf("%s is a shell builtin\n", name), 0);
	path = find_executable(name, shell);
	if (!path)
		return (not_found("type", name), 1);
	ft_printf("%s is %s\n", name, path);
	free(path);
	return (0);
}

static int	describe_which(char *name, t_shell *shell)
{
	char	*alias;
	char	*path;

	alias = env_get(shell->aliases, name);
	if (alias)
		return (ft_printf("%s: aliased to %s\n", name, alias), 0);
	if (is_builtin(name))
		return (ft_printf("%s: shell built-in command\n", name), 0);
	path = find_executable(name, shell);
	if (!path)
		return (not_found("which", name), 1);
	ft_printf("%s\n", path);
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
