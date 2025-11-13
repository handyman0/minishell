/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:00:06 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/13 17:04:43 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/utils.h"
#include "../../includes/minishell.h"

int	is_redirection_token(t_token *token)
{
	if (!token)
		return (0);
	return (token->type == TOKEN_REDIR_APPEND || token->type == TOKEN_REDIR_OUT || token->type == TOKEN_REDIR_APPEND || token->type == TOKEN_HEREDOC);
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
	new_redir->type = (t_redir_type)current->type;
	new_redir->file = NULL;
	new_redir->next = NULL;

	// Avançar para o token do arquivo
	current = current->next;
	if (!current || current->type != TOKEN_WORD)
	{
		free(new_redir);
		return (0);
	}
	new_redir->file = ft_strdup(current->value);
	if (!new_redir->file)
	{
		free(new_redir);
		return (0);
	}

	// Adicionar à lista de redirecionamentos
	if (*redirs == NULL)
		*redirs = new_redir;
	else
	{
		t_redir *last = *redirs;
		while (last->next)
			last = last->next;
		last->next = new_redir;
	}
	// Avança tokens
	*tokens = current->next;
	return (1);
}

void	free_redirs(t_redir *redirs)
{
	t_redir *tmp;
	while (redirs)
	{
		tmp = redirs->next;
		free(redirs->file);
		free(redirs);
		redirs = tmp;
	}
}

t_node	*parse_expression(t_token **tokens)
{
	t_node	*node;

	if (!tokens || !*tokens)
		return (NULL);
	if ((*tokens)->type == TOKEN_LPAREN)
	{
		*tokens = (*tokens)->next;
		node = parse_and_or(tokens);
		if (*tokens && (*tokens)->type == TOKEN_RPAREN)
			*tokens = (*tokens)->next;
		return (node);
	}
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
	while (*tokens && ((*tokens)->type == TOKEN_AND || (*tokens)->type == TOKEN_OR))
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
		if (args->content)
			free(args->content);
		free(args);
		args = tmp;
	}
}

t_node	*parse_command(t_token **tokens)
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
	while (*tokens)
	{
		// handle redirections
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
			ft_lstadd_back(&args, new_node);
			count++;
			*tokens = (*tokens)->next;
		}
		else
			break;

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
	}

	return (node);
}

t_node	*parse_pipeline(t_token **tokens)
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

void	process_tokens_before_parsing(t_token **tokens, t_shell *shell)
{
	if (!tokens || !*tokens || !shell)
		return ;
	t_token *current = *tokens;
	while (current)
	{
		if (current->type == TOKEN_WORD && current->value)
		{
			char *expanded = expand_variables(current->value, shell);
			if (expanded)
			{
				char	*clean = remove_quotes(expanded);
				free(expanded);
				free(current->value);
				current->value = clean;
			}
		}
		current = current->next;
	}
}

t_node	*parse_tokens(t_token **tokens)
{
	if (!tokens || !*tokens)
		return (NULL);
	return (parse_expression(tokens));
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
		free_redirs(node->data.cmd.redirs);
	}
	else
	{
		free_ast(node->data.op.left);
		free_ast(node->data.op.right);
	}
	free(node);
}
