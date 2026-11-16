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
#include <fcntl.h>
#include <signal.h>

static char	*read_heredoc_line(void)
{
	if (isatty(STDIN_FILENO))
		return (le_readline("heredoc> ", NULL, LE_PLAIN));
	return (ml_reader_line(ml_stdin_reader()));
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
		if (raw[i] == '\\' && raw[i + 1] && quote != '\'')
			out[j++] = raw[++i];
		else if (!quote && (raw[i] == '\'' || raw[i] == '"'))
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
	done = heredoc_fill(fd, limiter, !ft_strchr(raw, '\\') && !ft_strchr(raw, '\'')
			&& !ft_strchr(raw, '"'), shell);
	free(limiter);
	if (!done)
		return (close(fd), -1);
	lseek(fd, 0, SEEK_SET);
	return (fd);
}

static int	command_heredocs(t_node *node, t_shell *shell)
{
	t_redir	*redir;

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

int	prepare_heredocs(t_node *root, t_shell *shell)
{
	t_node	**stack;
	t_node	*cur;
	int		depth;
	int		bad;

	depth = 0;
	cur = root;
	while (cur && cur->type != NODE_CMD && ++depth)
		cur = cur->data.op.left;
	stack = malloc(sizeof(t_node *) * (depth + 1));
	if (!stack)
		return (1);
	cur = root;
	depth = 0;
	while (cur && cur->type != NODE_CMD)
	{
		stack[depth++] = cur;
		cur = cur->data.op.left;
	}
	bad = cur && command_heredocs(cur, shell);
	while (!bad && depth-- > 0)
		bad = prepare_heredocs(stack[depth]->data.op.right, shell);
	free(stack);
	return (bad);
}
