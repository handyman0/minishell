#include "../../includes/parser.h"
#include "../../includes/minishell.h"

#define MAX_ALIAS_DEPTH 16

static const char	*g_opens[] = {"if", "then", "elif", "else", "do", "while",
	"until", "{", "!", "time", NULL};
static const char	*g_closes[] = {"fi", "done", "esac", "}", "in", NULL};

static int	in_list(const char **list, const char *word)
{
	int	i;

	i = 0;
	while (list[i])
		if (ft_strcmp(list[i++], word) == 0)
			return (1);
	return (0);
}

int	is_reserved_word(const char *word)
{
	return (in_list(g_opens, word) || in_list(g_closes, word)
		|| ft_strcmp(word, "for") == 0 || ft_strcmp(word, "case") == 0
		|| ft_strcmp(word, "select") == 0 || ft_strcmp(word, "[[") == 0
		|| ft_strcmp(word, "function") == 0);
}

static int	was_seen(char **seen, int depth, const char *name)
{
	while (depth-- > 0)
		if (ft_strcmp(seen[depth], name) == 0)
			return (1);
	return (0);
}

static int	ends_blank(const char *value)
{
	size_t	n;

	n = ft_strlen(value);
	return (n > 0 && (value[n - 1] == ' ' || value[n - 1] == '\t'));
}

static int	expand_one(t_token **link, t_shell *shell, char **seen, int depth)
{
	t_token	*tok;
	t_token	*fresh;
	t_token	*tail;
	char	*value;
	int		blank;

	tok = *link;
	if (tok->type != TOKEN_WORD || !tok->value || is_reserved_word(tok->value)
		|| depth >= MAX_ALIAS_DEPTH || was_seen(seen, depth, tok->value))
		return (0);
	value = env_get(shell->aliases, tok->value);
	fresh = value ? tokenize_line(value) : NULL;
	if (!fresh && value)
	{
		*link = tok->next;
		blank = ends_blank(value);
		return (free(tok->value), free(tok), 10 + blank);
	}
	if (!fresh)
		return (0);
	blank = ends_blank(value);
	seen[depth] = tok->value;
	tail = fresh;
	tail->line = tok->line;
	while (tail->next)
	{
		tail = tail->next;
		tail->line = tok->line;
	}
	tail->next = tok->next;
	*link = fresh;
	if (expand_one(link, shell, seen, depth + 1) == 2)
		blank = 1;
	free(tok->value);
	free(tok);
	return (1 + blank);
}

int	opens_command(t_token *tok)
{
	return (tok->type == TOKEN_PIPE || tok->type == TOKEN_AND
		|| tok->type == TOKEN_OR || tok->type == TOKEN_SEMI
		|| tok->type == TOKEN_BG || tok->type == TOKEN_LPAREN
		|| tok->type == TOKEN_RPAREN || tok->type == TOKEN_DSEMI
		|| (tok->type == TOKEN_WORD && tok->value
			&& in_list(g_opens, tok->value)));
}

static int	is_assignment(const char *word)
{
	int	i;

	if (!word || !(ft_isalpha(word[0]) || word[0] == '_'))
		return (0);
	i = 1;
	while (ft_isalnum(word[i]) || word[i] == '_')
		i++;
	return (word[i] == '=');
}

static void	classify(t_token *tok, int *expect_cmd, int *redir)
{
	if (tok->type == TOKEN_WORD && *redir)
		*redir = 0;
	else if (tok->type == TOKEN_WORD && *expect_cmd
		&& is_assignment(tok->value))
		return ;
	else if (opens_command(tok))
		*expect_cmd = 1;
	else if (is_redirection_token(tok))
		*redir = 1;
	else
		*expect_cmd = 0;
}

void	expand_aliases(t_token **tokens, t_shell *shell)
{
	t_token	**link;
	t_token	*resume;
	t_token	*after;
	char	*seen[MAX_ALIAS_DEPTH];
	int		expect_cmd;
	int		redir;
	int		res;
	int		noexp;

	link = tokens;
	expect_cmd = 1;
	redir = 0;
	resume = NULL;
	noexp = 0;
	while (*link && shell->aliases)
	{
		if (*link == resume)
			expect_cmd = 1;
		after = (*link)->next;
		res = 0;
		if (expect_cmd && !redir && !noexp)
			res = expand_one(link, shell, seen, 0);
		noexp = 0;
		if (res >= 10)
		{
			noexp = res == 10;
			continue ;
		}
		if (res == 2)
			resume = after;
		classify(*link, &expect_cmd, &redir);
		link = &(*link)->next;
	}
}
