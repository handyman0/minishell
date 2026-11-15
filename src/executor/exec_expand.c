#include "../../includes/executor.h"
#include "../../includes/parser.h"

static t_token	*argv_to_tokens(char **argv)
{
	t_token	*head;
	t_token	*cur;
	int		i;

	head = NULL;
	cur = NULL;
	i = 0;
	while (argv[i])
	{
		token_add_back(&head, &cur, TOKEN_WORD, argv[i]);
		i++;
	}
	return (head);
}

static char	**tokens_to_argv(t_token *tokens)
{
	t_token	*tmp;
	char	**argv;
	int		count;
	int		i;

	count = 0;
	tmp = tokens;
	while (tmp && ++count)
		tmp = tmp->next;
	argv = malloc(sizeof(char *) * (count + 1));
	i = 0;
	while (tokens)
	{
		tmp = tokens->next;
		if (argv)
			argv[i++] = tokens->value;
		else
			free(tokens->value);
		free(tokens);
		tokens = tmp;
	}
	if (argv)
		argv[i] = NULL;
	return (argv);
}

static void	expand_redirs(t_redir *redir, t_shell *shell)
{
	char	*expanded;

	while (redir)
	{
		expanded = NULL;
		if (redir->type != REDIR_HEREDOC)
			expanded = expand_variables(redir->file, shell);
		if (expanded)
		{
			free(redir->file);
			redir->file = expanded;
		}
		redir = redir->next;
	}
}

void	expand_command(t_node *node, t_shell *shell)
{
	t_token	*tokens;

	expand_redirs(node->data.cmd.redirs, shell);
	if (!node->data.cmd.argv)
		return ;
	tokens = argv_to_tokens(node->data.cmd.argv);
	free(node->data.cmd.argv);
	expand_wildcards(&tokens);
	process_tokens_before_parsing(&tokens, shell);
	node->data.cmd.argv = tokens_to_argv(tokens);
}
