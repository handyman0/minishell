#include "../includes/builtins.h"
#include "../includes/minishell.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int	source_file(const char *path, t_shell *shell)
{
	static t_reader	reader;
	t_reader		saved;
	char			*line;
	int				fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (-1);
	saved = reader;
	ml_reader_init(&reader, fd);
	line = ml_reader_command(&reader, line_incomplete);
	while (line && shell->running)
	{
		run_line(line, shell);
		free(line);
		line = ml_reader_command(&reader, line_incomplete);
	}
	free(line);
	close(fd);
	reader = saved;
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
