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

static char	*append_line(char *text, size_t *len, const char *line)
{
	size_t	n;
	char	*out;

	n = ft_strlen(line);
	out = malloc(*len + n + 2);
	if (!out)
		return (free(text), NULL);
	if (text)
		ft_memcpy(out, text, *len);
	ft_memcpy(out + *len, line, n);
	out[*len + n] = '\n';
	out[*len + n + 1] = '\0';
	*len += n + 1;
	free(text);
	return (out);
}

static char	*collect_body(const char *limiter, int strip, int *ok)
{
	char	*text;
	char	*line;
	size_t	len;
	char	*cmp;

	text = ft_strdup("");
	len = 0;
	*ok = 1;
	while (text)
	{
		line = read_heredoc_line();
		if (g_signal == SIGINT)
			return (*ok = 0, free(line), free(text), NULL);
		cmp = line;
		while (cmp && strip && *cmp == '\t')
			cmp++;
		if (!line || ft_strcmp(cmp, limiter) == 0)
			return (free(line), text);
		text = append_line(text, &len, cmp);
		free(line);
	}
	return (*ok = 0, NULL);
}

static int	write_body(int fd, char *text, int expand, t_shell *shell)
{
	char	*out;

	out = text;
	if (expand)
		out = expand_heredoc_line(text, shell);
	if (!out)
		return (0);
	write(fd, out, ft_strlen(out));
	if (out != text)
		free(out);
	return (1);
}

int	heredoc_read(const t_redir *redir, t_shell *shell)
{
	char	*limiter;
	char	*text;
	int		fd;
	int		ok;
	int		expand;

	fd = heredoc_tmpfile();
	limiter = heredoc_delimiter(redir->file);
	if (fd == -1 || !limiter)
		return (free(limiter), fd == -1 ? -1 : (close(fd), -1));
	g_signal = 0;
	expand = !ft_strchr(redir->file, '\\') && !ft_strchr(redir->file, '\'')
		&& !ft_strchr(redir->file, '"');
	ok = 1;
	if (redir->body)
		text = ft_strdup(redir->body);
	else
		text = collect_body(limiter, redir->strip, &ok);
	free(limiter);
	if (!ok || !text || !write_body(fd, text, expand, shell))
		return (free(text), close(fd), -1);
	free(text);
	lseek(fd, 0, SEEK_SET);
	return (fd);
}

static int	redirs_heredocs(t_redir *redir, t_shell *shell)
{
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			redir->hd_fd = heredoc_read(redir, shell);
			if (redir->hd_fd == -1)
				return (1);
		}
		redir = redir->next;
	}
	return (0);
}

static int	ctl_heredocs(t_node *node, t_shell *shell)
{
	t_case	*item;

	if (prepare_heredocs(node->data.ctl.a, shell)
		|| prepare_heredocs(node->data.ctl.b, shell)
		|| prepare_heredocs(node->data.ctl.c, shell))
		return (1);
	item = node->data.ctl.cases;
	while (item)
	{
		if (prepare_heredocs(item->body, shell))
			return (1);
		item = item->next;
	}
	return (redirs_heredocs(node->data.ctl.redirs, shell));
}

static int	leaf_heredocs(t_node *node, t_shell *shell)
{
	if (!node)
		return (0);
	if (node->type == NODE_CMD)
		return (redirs_heredocs(node->data.cmd.redirs, shell));
	return (ctl_heredocs(node, shell));
}

static int	is_leaf(t_node *node)
{
	return (node->type == NODE_CMD || is_ctl_node(node));
}

int	prepare_heredocs(t_node *root, t_shell *shell)
{
	t_node	**stack;
	t_node	*cur;
	int		depth;
	int		bad;

	depth = 0;
	cur = root;
	while (cur && !is_leaf(cur) && ++depth)
		cur = cur->data.op.left;
	stack = malloc(sizeof(t_node *) * (depth + 1));
	if (!stack)
		return (1);
	cur = root;
	depth = 0;
	while (cur && !is_leaf(cur))
	{
		stack[depth++] = cur;
		cur = cur->data.op.left;
	}
	bad = leaf_heredocs(cur, shell);
	while (!bad && depth-- > 0)
		bad = prepare_heredocs(stack[depth]->data.op.right, shell);
	free(stack);
	return (bad);
}

int	herestring_fd(const char *text)
{
	int	fd;

	fd = heredoc_tmpfile();
	if (fd == -1)
		return (-1);
	write(fd, text ? text : "", ft_strlen(text ? text : ""));
	write(fd, "\n", 1);
	lseek(fd, 0, SEEK_SET);
	return (fd);
}
