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
	int				base;
	int				lineno;
	int				ran;

	ran = 0;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (-1);
	saved = reader;
	base = shell->line_base;
	lineno = shell->lineno;
	ml_reader_init(&reader, fd);
	shell->src_depth++;
	line = ml_reader_command(&reader, line_incomplete);
	while (line && shell->running && !shell->ret)
	{
		shell->line_base = (int)reader.cmd_first - 1;
		run_line(line, shell);
		ran = 1;
		free(line);
		line = ml_reader_command(&reader, line_incomplete);
	}
	free(line);
	close(fd);
	if (!ran)
		shell->exit_status = 0;
	shell->src_depth--;
	shell->ret = 0;
	reader = saved;
	shell->line_base = base;
	shell->lineno = lineno;
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
		shell->running = shell->spare;
		return (1);
	}
	return (status);
}
