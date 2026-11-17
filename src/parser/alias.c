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

typedef struct s_elig
{
	t_token	*list[MAX_ALIAS_DEPTH * 2];
	int		count;
}	t_elig;

typedef struct s_alias
{
	t_shell	*shell;
	t_elig	el;
	char	*seen[MAX_ALIAS_DEPTH];
	int		expect_cmd;
	int		redir;
	int		kw;
}	t_alias;

static void	elig_add(t_elig *el, t_token *tok)
{
	if (tok && el->count < MAX_ALIAS_DEPTH * 2)
		el->list[el->count++] = tok;
}

static int	elig_has(t_elig *el, t_token *tok)
{
	int	i;

	i = 0;
	while (i < el->count)
		if (el->list[i++] == tok)
			return (1);
	return (0);
}

#define K_FNAME 1
#define K_CNAME 2
#define K_FIN 3
#define K_CIN 4
#define K_PAT 5
#define K_LIST 6
#define K_DO 7

static t_shell	*g_ctx;

t_shell	*alias_shell(void)
{
	return (g_ctx);
}

void	alias_context(t_shell *shell)
{
	g_ctx = shell;
}

static int	reserved_at(int kw, const char *word)
{
	if (kw == K_FNAME || kw == K_CNAME)
		return (0);
	if (kw == K_FIN || kw == K_CIN)
		return (ft_strcmp(word, "in") == 0 || ft_strcmp(word, "do") == 0);
	if (kw == K_DO)
		return (ft_strcmp(word, "do") == 0);
	if (kw == K_LIST)
		return (0);
	if (kw == K_PAT)
		return (ft_strcmp(word, "esac") == 0);
	return (is_reserved_word(word));
}

static int	expand_one(t_token **link, t_alias *a, int depth)
{
	t_token	*tok;
	t_token	*fresh;
	t_token	*tail;
	char	*value;

	tok = *link;
	if (tok->type != TOKEN_WORD || !tok->value || reserved_at(a->kw, tok->value)
		|| depth >= MAX_ALIAS_DEPTH || was_seen(a->seen, depth, tok->value))
		return (0);
	value = env_get(a->shell->aliases, tok->value);
	fresh = value ? tokenize_line(value) : NULL;
	if (!fresh && value)
	{
		*link = tok->next;
		if (ends_blank(value))
			elig_add(&a->el, tok->next);
		return (free(tok->value), free(tok), 10 + !ends_blank(value));
	}
	if (!fresh)
		return (0);
	a->seen[depth] = tok->value;
	tail = fresh;
	tail->line = tok->line;
	while (tail->next)
	{
		tail = tail->next;
		tail->line = tok->line;
	}
	tail->next = tok->next;
	*link = fresh;
	if (ends_blank(value))
		elig_add(&a->el, tail->next);
	expand_one(link, a, depth + 1);
	return (free(tok->value), free(tok), 1);
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

static int	is_word(t_token *tok, const char *text)
{
	return (tok->type == TOKEN_WORD && tok->value
		&& ft_strcmp(tok->value, text) == 0);
}

static int	kw_name(t_token *tok, t_alias *a)
{
	if (tok->type != TOKEN_WORD)
		return (a->kw = 0, 0);
	a->kw += 2;
	a->expect_cmd = 0;
	return (1);
}

static int	kw_in(t_token *tok, t_alias *a)
{
	if (is_word(tok, "in"))
	{
		a->kw = a->kw == K_CIN ? K_PAT : K_LIST;
		a->expect_cmd = 0;
		return (1);
	}
	if (is_word(tok, "do"))
		return (a->kw = 0, a->expect_cmd = 1, 1);
	if (tok->type == TOKEN_SEMI && a->kw == K_FIN)
		return (a->kw = K_DO, a->expect_cmd = 0, 1);
	return (a->kw = 0, 0);
}

static int	kw_state(t_token *tok, t_alias *a)
{
	if (a->kw == K_FNAME || a->kw == K_CNAME)
		return (kw_name(tok, a));
	if (a->kw == K_FIN || a->kw == K_CIN)
		return (kw_in(tok, a));
	if (a->kw == K_LIST)
	{
		a->expect_cmd = 0;
		if (tok->type == TOKEN_SEMI)
			a->kw = K_DO;
		return (1);
	}
	if (a->kw == K_DO)
	{
		a->expect_cmd = 0;
		if (is_word(tok, "do"))
			return (a->kw = 0, a->expect_cmd = 1, 1);
		return (tok->type == TOKEN_SEMI ? 1 : (a->kw = 0, 0));
	}
	if (tok->type == TOKEN_RPAREN)
		return (a->kw = 0, a->expect_cmd = 1, 1);
	if (is_word(tok, "esac"))
		return (a->kw = 0, a->expect_cmd = 0, 1);
	return (a->expect_cmd = 0, 1);
}

static void	classify(t_token *tok, t_alias *a)
{
	int	at_cmd;

	at_cmd = a->kw == 0 && a->expect_cmd && !a->redir;
	if (a->kw != 0 && kw_state(tok, a))
		return ;
	if (tok->type == TOKEN_WORD && a->redir)
		a->redir = 0;
	else if (tok->type == TOKEN_DSEMI)
		a->kw = K_PAT;
	else if (tok->type == TOKEN_WORD && a->expect_cmd
		&& is_assignment(tok->value))
		return ;
	else if (at_cmd && (is_word(tok, "for") || is_word(tok, "select")))
		a->kw = K_FNAME;
	else if (at_cmd && is_word(tok, "case"))
		a->kw = K_CNAME;
	else if (opens_command(tok))
		a->expect_cmd = 1;
	else if (is_redirection_token(tok))
		a->redir = 1;
	else
		a->expect_cmd = 0;
	if (a->kw != 0)
		a->expect_cmd = 0;
}

static void	drop_newline(t_token *prev, t_token **link)
{
	t_token	*gone;

	if (!prev || !*link || (*link)->type != TOKEN_SEMI || !(*link)->value
		|| (prev->type != TOKEN_PIPE && prev->type != TOKEN_AND
			&& prev->type != TOKEN_OR))
		return ;
	gone = *link;
	*link = gone->next;
	free(gone->value);
	free(gone);
}

void	expand_aliases(t_token **tokens, t_shell *shell)
{
	t_token	**link;
	t_token	*prev;
	t_alias	a;
	int		res;
	int		noexp;

	link = tokens;
	prev = NULL;
	ft_bzero(&a, sizeof(a));
	a.shell = shell;
	a.expect_cmd = 1;
	noexp = 0;
	while (*link && shell->aliases)
	{
		if (elig_has(&a.el, *link))
			a.expect_cmd = 1;
		res = 0;
		if (a.expect_cmd && !a.redir && !noexp)
			res = expand_one(link, &a, 0);
		noexp = 0;
		if (res >= 10)
		{
			drop_newline(prev, link);
			noexp = res == 10;
			continue ;
		}
		classify(*link, &a);
		prev = *link;
		link = &(*link)->next;
	}
}
