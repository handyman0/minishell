#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int	source_file(const char *path, t_shell *shell)
{
	char	*line;
	size_t	len;
	int		fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (-1);
	line = get_next_line(fd);
	while (line && shell->running)
	{
		len = ft_strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		run_line(line, shell);
		free(line);
		line = get_next_line(fd);
	}
	free(line);
	close(fd);
	return (shell->exit_status);
}

int	builtin_source(char **argv, t_shell *shell)
{
	int	status;

	if (!argv[1])
	{
		ft_putendl_fd("minishell: source: nome de arquivo requerido",
			STDERR_FILENO);
		return (2);
	}
	status = source_file(argv[1], shell);
	if (status == -1)
	{
		ft_putstr_fd("minishell: source: ", STDERR_FILENO);
		ft_putstr_fd(argv[1], STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
		ft_putendl_fd(strerror(errno), STDERR_FILENO);
		return (1);
	}
	return (status);
}
