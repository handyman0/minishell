/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:00:06 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/21 19:41:59 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/utils.h"
#include "../../includes/minishell.h"

int	is_redirection_token(t_token *token)
{
	if (!token)
		return (0);
	return (token->type == TOKEN_REDIR_IN || token->type == TOKEN_REDIR_OUT
		|| token->type == TOKEN_REDIR_APPEND
		|| token->type == TOKEN_HEREDOC);
}

int	handle_redirection(t_token **tokens, t_redir **redirs)
{
	t_redir	*new_redir;
	t_token	*current;

	if (!tokens || !*tokens)
		return (0);
	current = *tokens;
	new_redir = malloc(sizeof(t_redir));
	if (!new_redir)
		return (0);
	switch (current->type)
	{
		case TOKEN_REDIR_IN: new_redir->type = REDIR_IN; break ;
		case TOKEN_REDIR_OUT: new_redir->type = REDIR_OUT; break ;
		case TOKEN_REDIR_APPEND: new_redir->type = REDIR_APPEND; break ;
		case TOKEN_HEREDOC: new_redir->type = REDIR_HEREDOC; break ;
		default :
			free(new_redir);
			return (0);
	}
	new_redir->fd = (current->type == TOKEN_REDIR_OUT
			|| current->type == TOKEN_REDIR_APPEND);
	if (current->value)
		new_redir->fd = ft_atoi(current->value);
	new_redir->file = NULL;
	new_redir->next = NULL;
	new_redir->hd_fd = -1;
	current = current->next;
	if (!current || current->type != TOKEN_WORD)
	{
		free(new_redir);
		return (0);
	}
	new_redir->file = ft_strdup(current->value);
	if (new_redir->file && new_redir->type == REDIR_OUT
		&& new_redir->file[0] == '&' && ft_isdigit(new_redir->file[1]))
		new_redir->type = REDIR_DUP;
	if (!new_redir->file)
	{
		free(new_redir);
		return (0);
	}
	if (*redirs == NULL)
		*redirs = new_redir;
	else
	{
		t_redir *last = *redirs;
		while (last->next)
			last = last->next;
		last->next = new_redir;
	}
	*tokens = current->next;
	return (1);
}

void	free_redirs(t_redir *redirs)
{
	t_redir	*tmp;

	while (redirs)
	{
		tmp = redirs->next;
		if (redirs->file)
			free(redirs->file);
		if (redirs->hd_fd >= 0)
			close(redirs->hd_fd);
		free(redirs);
		redirs = tmp;
	}
}

t_node	*parse_expression(t_token **tokens)
{
	if (!tokens || !*tokens)
		return (NULL);
	return (parse_and_or(tokens));
}

t_node	*parse_and_or(t_token **tokens)
{
	t_node		*left;
	t_node		*node;
	t_toktype	op_type;

	left = parse_pipeline(tokens);
	if (!left)
		return (NULL);
	while (*tokens && ((*tokens)->type == TOKEN_AND
		|| (*tokens)->type == TOKEN_OR))
	{
		op_type = (*tokens)->type;
		*tokens = (*tokens)->next;
		node = malloc(sizeof(t_node));
		if (!node)
		{
			free_ast(left);
			return (NULL);
		}
		node->type = (op_type == TOKEN_AND) ? NODE_AND : NODE_OR;
		node->data.op.left = left;
		node->data.op.right = parse_pipeline(tokens);
		if (!node->data.op.right && !parse_failed())
			parse_fail_at(*tokens);
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

void	free_args_list(t_list *args)
{
	t_list	*tmp;

	while (args)
	{
		tmp = args->next;
		free(args);
		args = tmp;
	}
}

t_node	*parse_command(t_token **tokens)
{
	t_node	*node;
	t_list	*args;
	t_list	*new_node;
	t_list	*tail;
	int		count;

	if (!tokens || !*tokens)
		return (NULL);
	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->type = NODE_CMD;
	node->data.cmd.argv = NULL;
	node->data.cmd.redirs = NULL;
	node->data.cmd.assigns = NULL;
	args = NULL;
	tail = NULL;
	count = 0;
	while (*tokens)
	{
		if (is_redirection_token(*tokens))
		{
			if (!handle_redirection(tokens, &node->data.cmd.redirs))
			{
				free_args_list(args);
				free_redirs(node->data.cmd.redirs);
				free(node);
				return (NULL);
			}
			continue;
		}
		if ((*tokens)->type == TOKEN_WORD)
		{
			new_node = ft_lstnew(ft_strdup((*tokens)->value));
			if (!new_node)
			{
				free_args_list(args);
				free_redirs(node->data.cmd.redirs);
				free(node);
				return (NULL);
			}
			if (args)
				tail->next = new_node;
			else
				args = new_node;
			tail = new_node;
			count++;
			*tokens = (*tokens)->next;
		}
		else
			break;
	}
	if (count == 0 && !node->data.cmd.redirs)
	{
		parse_fail_at(*tokens);
		return (free(node), NULL);
	}
	if (count > 0)
	{
		node->data.cmd.argv = malloc(sizeof(char *) * (count + 1));
		if (!node->data.cmd.argv)
		{
			free_args_list(args);
			free_redirs(node->data.cmd.redirs);
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
		t_list	*tmp;
		while (args)
		{
			tmp = args->next;
			free(args);
			args = tmp;
		}
	}
	return (node);
}

static t_node	*parse_primary(t_token **tokens)
{
	t_node	*node;

	if (!tokens || !*tokens)
		return (NULL);
	if (is_closer_word(*tokens))
		return (parse_fail_at(*tokens));
	node = parse_compound(tokens);
	if (node || parse_failed())
		return (node);
	return (parse_command(tokens));
}

static t_node	*parse_plain_pipeline(t_token **tokens)
{
	t_node	*left;
	t_node	*node;

	left = parse_primary(tokens);
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
		node->data.op.right = parse_primary(tokens);
		if (!node->data.op.right && !parse_failed())
			parse_fail_at(*tokens);
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

t_node	*parse_pipeline(t_token **tokens)
{
	t_node	*node;
	int		negate;

	negate = 0;
	while (*tokens && (*tokens)->type == TOKEN_WORD
		&& ft_strcmp((*tokens)->value, "!") == 0)
	{
		negate = !negate;
		*tokens = (*tokens)->next;
	}
	node = parse_plain_pipeline(tokens);
	if (!node && !parse_failed())
		parse_fail_at(*tokens);
	if (node && negate)
		return (make_node(NODE_NOT, node, NULL));
	return (node);
}

t_node	*make_node(t_nodetype type, t_node *left, t_node *right)
{
	t_node	*node;

	node = malloc(sizeof(t_node));
	if (!node)
		return (NULL);
	node->type = type;
	node->data.op.left = left;
	node->data.op.right = right;
	return (node);
}


static void	free_command(t_node *node)
{
	int	i;

	if (node->data.cmd.assigns)
		free_str_array(node->data.cmd.assigns);

	if (node->data.cmd.argv)
	{
		i = 0;
		while (node->data.cmd.argv[i])
			free(node->data.cmd.argv[i++]);
		free(node->data.cmd.argv);
	}
	free_redirs(node->data.cmd.redirs);
}

int	is_ctl_node(t_node *node)
{
	return (node->type == NODE_IF || node->type == NODE_WHILE
		|| node->type == NODE_FOR || node->type == NODE_CASE
		|| node->type == NODE_GROUP || node->type == NODE_FUNC);
}

void	free_ctl(t_node *node)
{
	t_case	*item;
	t_case	*next;

	if (!node)
		return ;
	free_ast(node->data.ctl.a);
	free_ast(node->data.ctl.b);
	free_ast(node->data.ctl.c);
	free(node->data.ctl.name);
	free_str_array(node->data.ctl.words);
	free_redirs(node->data.ctl.redirs);
	item = node->data.ctl.cases;
	while (item)
	{
		next = item->next;
		free_str_array(item->patterns);
		free_ast(item->body);
		free(item);
		item = next;
	}
	free(node);
}

void	free_ast(t_node *node)
{
	t_node	*next;

	while (node)
	{
		next = NULL;
		if (is_ctl_node(node))
			return (free_ctl(node));
		if (node->type == NODE_CMD)
			free_command(node);
		else
		{
			free_ast(node->data.op.right);
			next = node->data.op.left;
		}
		free(node);
		node = next;
	}
}
