/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:00:06 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/03 14:56:54 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/utils.h"

static void	free_args_list(t_list *args)
{
	t_list	*tmp;

	while (args)
	{
		tmp = args->next;
		if (args->content)
			free(args->content);
		free(args);
		args = tmp;
	}
}

static t_node	*parse_command(t_token **tokens)
{
	t_node	*node;
	t_list	*args;
	t_list	*new_node;
	int		count;

	if (!tokens || !*tokens)
		return (NULL);

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->type = NODE_CMD;
	node->data.cmd.argv = NULL;
	node->data.cmd.redirs = NULL;

	args = NULL;
	count = 0;
	while (*tokens && (*tokens)->type == TOKEN_WORD)
	{
		new_node = ft_lstnew(ft_strdup((*tokens)->value));
		if (!new_node)
		{
			free_args_list(args);
			free(node);
			return (NULL);
		}
		ft_lstadd_back(&args, new_node);
		count++;
		*tokens = (*tokens)->next;
	}

	if (count > 0)
	{
		node->data.cmd.argv = malloc(sizeof(char *) * (count + 1));
		if (!node->data.cmd.argv)
		{
			free_args_list(args);
			free(node);
			return (NULL);
		}

		t_list *current = args;
		int i = 0;
		while (current && i < count)
		{
			node->data.cmd.argv[i] = current->content;
			current = current->next;
			i++;
		}
		node->data.cmd.argv[count] = NULL;

		// Libera apenas os nós da lista, não o conteúdo (já no array)
		t_list *tmp;
		while (args)
		{
			tmp = args->next;
			free(args);
			args = tmp;
		}
	}

	return (node);
}

static t_node	*parse_pipeline(t_token **tokens)
{
	t_node	*left;
	t_node	*node;

	left = parse_command(tokens);
	if (!left)
		return (NULL);

	while (*tokens && (*tokens)->type == TOKEN_PIPE)
	{
		*tokens = (*tokens)->next;

		node = malloc(sizeof(t_node));
		if (!node)
		{
			free_ast(left);
			return (NULL);
		}
		node->type = NODE_PIPE;
		node->data.op.left = left;
		node->data.op.right = parse_command(tokens);

		if (!node->data.op.right)
		{
			free(node);
			free_ast(left);
			return (NULL);
		}
		left = node;
	}
	return (left);
}

t_node	*parse_tokens(t_token **tokens)
{
	if (!tokens || !*tokens)
		return (NULL);
	return (parse_pipeline(tokens));
}

void	free_ast(t_node *node)
{
	int	i;

	if (!node)
		return;

	if (node->type == NODE_CMD)
	{
		if (node->data.cmd.argv)
		{
			i = 0;
			while (node->data.cmd.argv[i])
			{
				free(node->data.cmd.argv[i]);
				i++;
			}
			free(node->data.cmd.argv);
		}
		// TODO: free redirections
	}
	else
	{
		free_ast(node->data.op.left);
		free_ast(node->data.op.right);
	}
	free(node);
}
