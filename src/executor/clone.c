#include "../../includes/executor.h"

static char	**copy_strings(char **src)
{
	char	**out;
	int		n;
	int		i;

	if (!src)
		return (NULL);
	n = 0;
	while (src[n])
		n++;
	out = ft_calloc(n + 1, sizeof(char *));
	i = 0;
	while (out && i < n)
	{
		out[i] = ft_strdup(src[i]);
		i++;
	}
	return (out);
}

static t_redir	*copy_redir(t_redir *src)
{
	t_redir	*out;

	out = malloc(sizeof(t_redir));
	if (!out)
		return (NULL);
	*out = *src;
	out->file = ft_strdup(src->file);
	out->next = NULL;
	out->hd_fd = -1;
	if (src->hd_fd >= 0)
	{
		lseek(src->hd_fd, 0, SEEK_SET);
		out->hd_fd = fcntl(src->hd_fd, F_DUPFD_CLOEXEC, 3);
	}
	return (out);
}

static t_redir	*copy_redirs(t_redir *src)
{
	t_redir	*head;
	t_redir	*tail;
	t_redir	*node;

	head = NULL;
	tail = NULL;
	while (src)
	{
		node = copy_redir(src);
		if (!node)
			break ;
		if (tail)
			tail->next = node;
		else
			head = node;
		tail = node;
		src = src->next;
	}
	return (head);
}

t_node	*clone_command(t_node *node)
{
	t_node	*copy;

	copy = malloc(sizeof(t_node));
	if (!copy)
		return (NULL);
	copy->type = NODE_CMD;
	copy->data.cmd.argv = copy_strings(node->data.cmd.argv);
	copy->data.cmd.assigns = NULL;
	copy->data.cmd.redirs = copy_redirs(node->data.cmd.redirs);
	return (copy);
}
