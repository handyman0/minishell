#include "../../includes/parser.h"

static int	is_word(t_token *tok, const char *text)
{
	return (tok && tok->type == TOKEN_WORD && tok->value
		&& ft_strcmp(tok->value, text) == 0);
}

static t_token	*find_close(t_token *from)
{
	while (from && !is_word(from, "]]"))
		from = from->next;
	return (from);
}

static void	fold_range(t_token *open, t_token *close)
{
	t_token	*tok;
	t_token	*prev;
	t_token	*next;
	const char	*text;

	prev = open;
	tok = open->next;
	while (tok != close)
	{
		next = tok->next;
		text = tok->type == TOKEN_WORD ? NULL : token_text(tok);
		if (tok->type == TOKEN_SEMI)
		{
			prev->next = next;
			free(tok);
		}
		else
		{
			if (text)
				tok->value = ft_strdup(text);
			tok->type = TOKEN_WORD;
			prev = tok;
		}
		tok = next;
	}
}

void	fold_conds(t_token *head)
{
	t_token	*prev;
	t_token	*tok;
	t_token	*close;

	prev = NULL;
	tok = head;
	while (tok)
	{
		if (is_word(tok, "[[") && (!prev || opens_command(prev)))
		{
			close = find_close(tok->next);
			if (close)
			{
				fold_range(tok, close);
				tok = close;
			}
		}
		prev = tok;
		tok = tok->next;
	}
}
