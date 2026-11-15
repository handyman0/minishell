/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_heredoc.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:28:16 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/26 20:36:42 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../includes/executor.h"
#include "../../includes/parser.h"
#include <readline/readline.h>

static char	*read_heredoc_line(void)
{
	char	*line;
	size_t	len;

	if (isatty(STDIN_FILENO))
		return (readline("heredoc> "));
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
}

static char	*strip_quotes(const char *raw)
{
	char	*out;
	char	quote;
	int		i;
	int		j;

	out = malloc(ft_strlen(raw) + 1);
	if (!out)
		return (NULL);
	quote = 0;
	i = 0;
	j = 0;
	while (raw[i])
	{
		if (!quote && (raw[i] == '\'' || raw[i] == '"'))
			quote = raw[i];
		else if (quote && raw[i] == quote)
			quote = 0;
		else
			out[j++] = raw[i];
		i++;
	}
	out[j] = '\0';
	return (out);
}

static void	write_line(int fd, char *line, int expand, t_shell *shell)
{
	char	*expanded;

	if (expand)
	{
		expanded = expand_heredoc_line(line, shell);
		if (expanded)
		{
			ft_putendl_fd(expanded, fd);
			free(expanded);
			return ;
		}
	}
	ft_putendl_fd(line, fd);
}

void	exec_heredoc(const char *raw, t_shell *shell)
{
	int		fd[2];
	char	*line;
	char	*limiter;
	int		expand;

	if (pipe(fd) == -1)
		error_exit("pipe");
	expand = !ft_strchr(raw, '\'') && !ft_strchr(raw, '"');
	limiter = strip_quotes(raw);
	while (limiter)
	{
		line = read_heredoc_line();
		if (!line || ft_strcmp(line, limiter) == 0)
		{
			free(line);
			break ;
		}
		write_line(fd[1], line, expand, shell);
		free(line);
	}
	free(limiter);
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);
	close(fd[0]);
}
