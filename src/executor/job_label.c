#include "../../includes/jobs.h"

static char	*glue(char *left, const char *op, char *right)
{
	char	*tmp;
	char	*out;

	tmp = ft_strjoin(left ? left : "", op);
	out = ft_strjoin(tmp, right ? right : "");
	free(tmp);
	free(left);
	free(right);
	return (out);
}

static char	*argv_label(char **argv)
{
	char	*out;
	int		i;

	out = ft_strdup("");
	i = 0;
	while (argv && argv[i])
	{
		out = glue(out, i ? " " : "", ft_strdup(argv[i]));
		i++;
	}
	return (out);
}

char	*node_label(t_node *node)
{
	if (!node)
		return (ft_strdup(""));
	if (node->type == NODE_CMD)
		return (argv_label(node->data.cmd.argv));
	if (node->type == NODE_PAREN)
		return (glue(glue(ft_strdup("("), "", node_label(node->data.ctl.a)),
				")", NULL));
	if (node->type == NODE_BG)
		return (glue(node_label(node->data.op.left), " &", NULL));
	if (node->type == NODE_PIPE)
		return (glue(node_label(node->data.op.left), " | ",
				node_label(node->data.op.right)));
	if (node->type == NODE_AND)
		return (glue(node_label(node->data.op.left), " && ",
				node_label(node->data.op.right)));
	if (node->type == NODE_OR)
		return (glue(node_label(node->data.op.left), " || ",
				node_label(node->data.op.right)));
	return (glue(node_label(node->data.op.left), "; ",
			node_label(node->data.op.right)));
}
