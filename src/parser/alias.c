#include "../../includes/parser.h"
#include "../../includes/minishell.h"

#define MAX_ALIAS_DEPTH 8

static int	is_plain_word(const char *value)
{
	return (value && *value && !ft_strchr(value, '\'')
		&& !ft_strchr(value, '"') && !ft_strchr(value, '$'));
}

static int	was_seen(char **seen, int depth, const char *name)
{
	while (depth-- > 0)
		if (ft_strcmp(seen[depth], name) == 0)
			return (1);
	return (0);
}

static int	expand_one(t_token **link, t_shell *shell, char **seen, int depth)
{
	t_token	*tok;
	t_token	*fresh;
	t_token	*tail;
	char	*value;

	tok = *link;
	if (tok->type != TOKEN_WORD || !is_plain_word(tok->value)
		|| depth >= MAX_ALIAS_DEPTH || was_seen(seen, depth, tok->value))
		return (0);
	value = env_get(shell->aliases, tok->value);
	fresh = value ? tokenize_line(value) : NULL;
	if (!fresh)
		return (0);
	seen[depth] = tok->value;
	tail = fresh;
	while (tail->next)
		tail = tail->next;
	tail->next = tok->next;
	*link = fresh;
	expand_one(link, shell, seen, depth + 1);
	free(tok->value);
	free(tok);
	return (1);
}

static int	opens_command(t_token *tok)
{
	return (tok->type == TOKEN_PIPE || tok->type == TOKEN_AND
		|| tok->type == TOKEN_OR || tok->type == TOKEN_SEMI
		|| tok->type == TOKEN_BG || tok->type == TOKEN_LPAREN);
}

void	expand_aliases(t_token **tokens, t_shell *shell)
{
	t_token	**link;
	char	*seen[MAX_ALIAS_DEPTH];
	int		expect_cmd;
	int		redir;

	link = tokens;
	expect_cmd = 1;
	redir = 0;
	while (*link && shell->aliases)
	{
		if (expect_cmd && !redir)
			expand_one(link, shell, seen, 0);
		if (opens_command(*link))
			expect_cmd = 1;
		else if (is_redirection_token(*link))
			redir = 1;
		else if ((*link)->type == TOKEN_WORD && redir)
			redir = 0;
		else
			expect_cmd = 0;
		link = &(*link)->next;
	}
}
