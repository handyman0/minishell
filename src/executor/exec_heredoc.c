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
#include "../../includes/signals.h"
#include "../../includes/reader.h"
#include <readline/readline.h>
#include <fcntl.h>
#include <signal.h>

static char	*read_heredoc_line(void)
{
	if (isatty(STDIN_FILENO))
		return (readline("heredoc> "));
	return (reader_line(stdin_reader()));
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

static int	heredoc_tmpfile(void)
{
	char	path[] = "/tmp/.minishell_hd_XXXXXX";
	int		fd;

	fd = mkstemp(path);
	if (fd == -1)
		return (-1);
	unlink(path);
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	return (fd);
}

static int	heredoc_event(void)
{
	if (g_signal == SIGINT)
		rl_done = 1;
	return (0);
}

static void	heredoc_sigint(int sig)
{
	g_signal = sig;
	write(STDOUT_FILENO, "\n", 1);
}

static int	heredoc_fill(int fd, char *limiter, int expand, t_shell *shell)
{
	char	*line;

	while (1)
	{
		line = read_heredoc_line();
		if (g_signal == SIGINT)
			return (free(line), 0);
		if (!line || ft_strcmp(line, limiter) == 0)
		{
			free(line);
			return (1);
		}
		write_line(fd, line, expand, shell);
		free(line);
	}
}

int	heredoc_read(const char *raw, t_shell *shell)
{
	char	*limiter;
	int		fd;
	int		done;

	fd = heredoc_tmpfile();
	limiter = strip_quotes(raw);
	if (fd == -1 || !limiter)
		return (free(limiter), fd == -1 ? -1 : (close(fd), -1));
	g_signal = 0;
	rl_event_hook = heredoc_event;
	if (shell->interactive)
		signal(SIGINT, heredoc_sigint);
	done = heredoc_fill(fd, limiter, !ft_strchr(raw, '\'')
			&& !ft_strchr(raw, '"'), shell);
	rl_event_hook = NULL;
	if (shell->interactive)
		setup_signals();
	free(limiter);
	if (!done)
		return (close(fd), -1);
	lseek(fd, 0, SEEK_SET);
	return (fd);
}

int	prepare_heredocs(t_node *node, t_shell *shell)
{
	t_redir	*redir;

	if (!node)
		return (0);
	if (node->type != NODE_CMD)
		return (prepare_heredocs(node->data.op.left, shell)
			|| prepare_heredocs(node->data.op.right, shell));
	redir = node->data.cmd.redirs;
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			redir->hd_fd = heredoc_read(redir->file, shell);
			if (redir->hd_fd == -1)
				return (1);
		}
		redir = redir->next;
	}
	return (0);
}
