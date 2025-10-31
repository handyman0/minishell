/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug_print.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 18:01:22 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/31 19:20:18 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/utils.h"

static const char	*token_type_str(t_toktype type)
{
	static const char	*names[] = {
		"WORD", "PIPE", "REDIR_IN", "REDIR_OUT", "REDIR_APPEND",
		"HEREDOC", "AND", "OR", "LPAREN", "RPAREN"
	};
	return (names[type]);
}

void	print_tokens(t_token *tokens)
{
	t_token *current;
	int		i;

	printf("TOKENS:\n");
	current = tokens;
	i = 0;
	while (current)
	{
		printf("	[%d] %-12s", i, token_type_str(current->type));
		if (current->value)
			printf("	-> '%s'", current->value);
		printf("\n");
		current = current->next;
		i++;
	}
	printf("\n");
}
