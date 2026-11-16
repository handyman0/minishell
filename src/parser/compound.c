#include "../../includes/parser.h"
#include "../../includes/output.h"

static struct
{
	int		failed;
	int		eof;
	char	near[64];
}	g_perr;

void	parse_reset(void)
{
	g_perr.failed = 0;
	g_perr.eof = 0;
	g_perr.near[0] = '\0';
}

int	parse_failed(void)
{
	return (g_perr.failed);
}

int	parse_hit_eof(void)
{
	return (g_perr.eof);
}

void	parse_report(t_shell *shell)
{
	if (!g_perr.failed)
		return ;
	shell->exit_status = 2;
	if (g_perr.eof)
		ml_put_all(2, "minishell: syntax error: unexpected end of file\n",
			NULL);
	else
		ml_put_all(2, "minishell: syntax error near unexpected token `",
			g_perr.near, "'\n", NULL);
}

void	*parse_fail_at(t_token *tok)
{
	const char	*text;

	if (g_perr.failed)
		return (NULL);
	g_perr.failed = 1;
	if (!tok)
		g_perr.eof = 1;
	else
	{
		text = token_text(tok);
		ft_strlcpy(g_perr.near, text ? text : "?", sizeof(g_perr.near));
	}
	return (NULL);
}

static int	is_word(t_token *tok, const char *word)
{
	return (tok && tok->type == TOKEN_WORD && ft_strcmp(tok->value, word) == 0);
}

static int	stop_then(t_token *t)
{
	return (is_word(t, "then"));
}

static int	stop_else(t_token *t)
{
	return (is_word(t, "elif") || is_word(t, "else") || is_word(t, "fi"));
}

static int	stop_fi(t_token *t)
{
	return (is_word(t, "fi"));
}

static int	stop_do(t_token *t)
{
	return (is_word(t, "do"));
}

static int	stop_done(t_token *t)
{
	return (is_word(t, "done"));
}

static int	stop_case(t_token *t)
{
	return (is_word(t, "esac") || t->type == TOKEN_DSEMI);
}

static int	stop_brace(t_token *t)
{
	return (is_word(t, "}"));
}

static int	stop_paren(t_token *t)
{
	return (t->type == TOKEN_RPAREN);
}

static int	stop_top(t_token *t)
{
	return (t->type == TOKEN_RPAREN || t->type == TOKEN_DSEMI);
}

static void	skip_newlines(t_token **tokens)
{
	while (*tokens && (*tokens)->type == TOKEN_SEMI)
		*tokens = (*tokens)->next;
}

t_node	*parse_list(t_token **tokens, int (*stop)(t_token *))
{
	t_node	*list;
	t_node	*item;
	int		sep;

	list = NULL;
	skip_newlines(tokens);
	while (*tokens && !stop(*tokens))
	{
		item = parse_expression(tokens);
		if (!item)
			return (free_ast(list), NULL);
		sep = *tokens && ((*tokens)->type == TOKEN_SEMI
				|| (*tokens)->type == TOKEN_BG);
		if (sep && (*tokens)->type == TOKEN_BG)
			item = make_node(NODE_BG, item, NULL);
		if (sep)
			*tokens = (*tokens)->next;
		list = list ? make_node(NODE_SEQ, list, item) : item;
		skip_newlines(tokens);
		if (!sep)
			break ;
	}
	return (list);
}

static t_node	*need_list(t_token **tokens, int (*stop)(t_token *))
{
	t_node	*list;

	list = parse_list(tokens, stop);
	if (!list && !g_perr.failed)
		parse_fail_at(*tokens);
	return (list);
}

static int	expect(t_token **tokens, const char *word)
{
	skip_newlines(tokens);
	if (is_word(*tokens, word))
	{
		*tokens = (*tokens)->next;
		return (1);
	}
	parse_fail_at(*tokens);
	return (0);
}

static t_node	*new_ctl(t_nodetype type)
{
	t_node	*node;

	node = ft_calloc(1, sizeof(t_node));
	if (node)
		node->type = type;
	return (node);
}

static t_node	*parse_if_body(t_token **tokens)
{
	t_node	*node;

	node = new_ctl(NODE_IF);
	if (!node)
		return (NULL);
	node->data.ctl.a = need_list(tokens, stop_then);
	if (!node->data.ctl.a || !expect(tokens, "then"))
		return (free_ctl(node), NULL);
	node->data.ctl.b = need_list(tokens, stop_else);
	if (!node->data.ctl.b)
		return (free_ctl(node), NULL);
	if (is_word(*tokens, "elif"))
	{
		*tokens = (*tokens)->next;
		node->data.ctl.c = parse_if_body(tokens);
	}
	else if (is_word(*tokens, "else"))
	{
		*tokens = (*tokens)->next;
		node->data.ctl.c = need_list(tokens, stop_fi);
	}
	if ((is_word(*tokens, "elif") || is_word(*tokens, "else"))
		&& !node->data.ctl.c)
		return (free_ctl(node), NULL);
	return (node);
}

static t_node	*parse_if(t_token **tokens)
{
	t_node	*node;

	*tokens = (*tokens)->next;
	node = parse_if_body(tokens);
	if (node && !expect(tokens, "fi"))
		return (free_ctl(node), NULL);
	return (node);
}

static t_node	*parse_while(t_token **tokens)
{
	t_node	*node;

	node = new_ctl(NODE_WHILE);
	if (!node)
		return (NULL);
	node->data.ctl.flag = is_word(*tokens, "until");
	*tokens = (*tokens)->next;
	node->data.ctl.a = need_list(tokens, stop_do);
	if (!node->data.ctl.a || !expect(tokens, "do"))
		return (free_ctl(node), NULL);
	node->data.ctl.b = need_list(tokens, stop_done);
	if (!node->data.ctl.b || !expect(tokens, "done"))
		return (free_ctl(node), NULL);
	return (node);
}

static int	valid_name(const char *s)
{
	int	i;

	if (!ft_isalpha(s[0]) && s[0] != '_')
		return (0);
	i = 0;
	while (ft_isalnum(s[i]) || s[i] == '_')
		i++;
	return (s[i] == '\0');
}

static char	**add_word(char **list, const char *word)
{
	char	**out;
	int		n;
	int		i;

	n = 0;
	while (list && list[n])
		n++;
	out = ft_calloc(n + 2, sizeof(char *));
	i = 0;
	while (out && i < n)
	{
		out[i] = list[i];
		i++;
	}
	if (out)
		out[n] = ft_strdup(word);
	free(list);
	return (out);
}

static t_node	*parse_cfor(t_node *node, t_token **tokens)
{
	char	*spec;
	char	*part;
	int		n;
	int		i;
	int		start;

	spec = ft_substr((*tokens)->value, 2, ft_strlen((*tokens)->value) - 4);
	node->data.ctl.name = ft_strdup("");
	node->data.ctl.flag = 2;
	node->data.ctl.words = ft_calloc(1, sizeof(char *));
	n = 0;
	i = 0;
	start = 0;
	while (spec && n < 3)
	{
		if (!spec[i] || spec[i] == ';')
		{
			part = ft_substr(spec, start, i - start);
			node->data.ctl.words = add_word(node->data.ctl.words, part);
			free(part);
			n++;
			start = i + 1;
		}
		if (!spec[i++])
			break ;
	}
	free(spec);
	*tokens = (*tokens)->next;
	return (node);
}

static t_node	*parse_for(t_token **tokens)
{
	t_node	*node;

	node = new_ctl(NODE_FOR);
	*tokens = (*tokens)->next;
	if (node && *tokens && (*tokens)->type == TOKEN_WORD && (*tokens)->value
		&& (*tokens)->value[0] == '(' && (*tokens)->value[1] == '(')
	{
		parse_cfor(node, tokens);
		skip_newlines(tokens);
		if (!expect(tokens, "do"))
			return (free_ctl(node), NULL);
		node->data.ctl.a = need_list(tokens, stop_done);
		if (!node->data.ctl.a || !expect(tokens, "done"))
			return (free_ctl(node), NULL);
		return (node);
	}
	if (!node || !*tokens || (*tokens)->type != TOKEN_WORD
		|| !valid_name((*tokens)->value))
		return (parse_fail_at(*tokens), free_ctl(node), NULL);
	node->data.ctl.name = ft_strdup((*tokens)->value);
	*tokens = (*tokens)->next;
	skip_newlines(tokens);
	if (is_word(*tokens, "in"))
	{
		node->data.ctl.flag = 1;
		node->data.ctl.words = ft_calloc(1, sizeof(char *));
		*tokens = (*tokens)->next;
		while (*tokens && (*tokens)->type == TOKEN_WORD)
		{
			node->data.ctl.words = add_word(node->data.ctl.words,
					(*tokens)->value);
			*tokens = (*tokens)->next;
		}
	}
	if (!expect(tokens, "do"))
		return (free_ctl(node), NULL);
	node->data.ctl.a = need_list(tokens, stop_done);
	if (!node->data.ctl.a || !expect(tokens, "done"))
		return (free_ctl(node), NULL);
	return (node);
}

static t_case	*parse_case_item(t_token **tokens)
{
	t_case	*item;

	item = ft_calloc(1, sizeof(t_case));
	if (!item)
		return (NULL);
	if (*tokens && (*tokens)->type == TOKEN_LPAREN)
		*tokens = (*tokens)->next;
	while (*tokens && (*tokens)->type == TOKEN_WORD)
	{
		item->patterns = add_word(item->patterns, (*tokens)->value);
		*tokens = (*tokens)->next;
		if (!*tokens || (*tokens)->type != TOKEN_PIPE)
			break ;
		*tokens = (*tokens)->next;
	}
	if (!item->patterns || !*tokens || (*tokens)->type != TOKEN_RPAREN)
		return (parse_fail_at(*tokens), free_str_array(item->patterns),
			free(item), NULL);
	*tokens = (*tokens)->next;
	item->body = parse_list(tokens, stop_case);
	if (*tokens && (*tokens)->type == TOKEN_DSEMI)
		*tokens = (*tokens)->next;
	skip_newlines(tokens);
	return (item);
}

static t_node	*parse_case(t_token **tokens)
{
	t_node	*node;
	t_case	**tail;

	node = new_ctl(NODE_CASE);
	*tokens = (*tokens)->next;
	if (!node || !*tokens || (*tokens)->type != TOKEN_WORD)
		return (parse_fail_at(*tokens), free_ctl(node), NULL);
	node->data.ctl.words = add_word(NULL, (*tokens)->value);
	*tokens = (*tokens)->next;
	if (!expect(tokens, "in"))
		return (free_ctl(node), NULL);
	skip_newlines(tokens);
	tail = &node->data.ctl.cases;
	while (*tokens && !is_word(*tokens, "esac"))
	{
		*tail = parse_case_item(tokens);
		if (!*tail)
			return (free_ctl(node), NULL);
		tail = &(*tail)->next;
	}
	if (!expect(tokens, "esac"))
		return (free_ctl(node), NULL);
	return (node);
}

static t_node	*parse_group(t_token **tokens)
{
	t_node	*node;

	node = new_ctl(NODE_GROUP);
	*tokens = (*tokens)->next;
	if (!node)
		return (NULL);
	node->data.ctl.a = need_list(tokens, stop_brace);
	if (!node->data.ctl.a || !expect(tokens, "}"))
		return (free_ctl(node), NULL);
	return (node);
}

static t_node	*parse_subshell(t_token **tokens)
{
	t_node	*inner;
	t_node	*node;

	*tokens = (*tokens)->next;
	inner = need_list(tokens, stop_paren);
	if (!inner)
		return (NULL);
	if (!*tokens || (*tokens)->type != TOKEN_RPAREN)
		return (parse_fail_at(*tokens), free_ast(inner), NULL);
	*tokens = (*tokens)->next;
	node = new_ctl(NODE_PAREN);
	if (node)
		node->data.ctl.a = inner;
	return (node ? node : (free_ast(inner), NULL));
}

static int	is_funcdef(t_token *tok)
{
	return (tok->type == TOKEN_WORD && valid_name(tok->value) && tok->next
		&& tok->next->type == TOKEN_LPAREN && tok->next->next
		&& tok->next->next->type == TOKEN_RPAREN);
}

static t_node	*parse_funcdef(t_token **tokens)
{
	t_node	*node;
	t_node	*body;

	node = new_ctl(NODE_FUNC);
	if (is_word(*tokens, "function"))
		*tokens = (*tokens)->next;
	if (!node || !*tokens || (*tokens)->type != TOKEN_WORD)
		return (parse_fail_at(*tokens), free_ctl(node), NULL);
	node->data.ctl.name = ft_strdup((*tokens)->value);
	*tokens = (*tokens)->next;
	if (*tokens && (*tokens)->type == TOKEN_LPAREN && (*tokens)->next
		&& (*tokens)->next->type == TOKEN_RPAREN)
		*tokens = (*tokens)->next->next;
	skip_newlines(tokens);
	body = parse_compound(tokens);
	if (!body)
	{
		if (!g_perr.failed)
			parse_fail_at(*tokens);
		return (free_ctl(node), NULL);
	}
	node->data.ctl.a = body;
	return (node);
}

static t_node	*dispatch(t_token **tokens)
{
	t_token	*tok;

	tok = *tokens;
	if (tok->type == TOKEN_LPAREN)
		return (parse_subshell(tokens));
	if (tok->type != TOKEN_WORD)
		return (NULL);
	if (is_word(tok, "if"))
		return (parse_if(tokens));
	if (is_word(tok, "while") || is_word(tok, "until"))
		return (parse_while(tokens));
	if (is_word(tok, "for"))
		return (parse_for(tokens));
	if (is_word(tok, "case"))
		return (parse_case(tokens));
	if (is_word(tok, "{"))
		return (parse_group(tokens));
	if (is_word(tok, "function") || is_funcdef(tok))
		return (parse_funcdef(tokens));
	return (NULL);
}

t_node	*parse_compound(t_token **tokens)
{
	t_node	*node;

	if (!tokens || !*tokens)
		return (NULL);
	node = dispatch(tokens);
	while (node && is_ctl_node(node) && node->type != NODE_FUNC
		&& is_redirection_token(*tokens))
	{
		if (!handle_redirection(tokens, &node->data.ctl.redirs))
			return (parse_fail_at(*tokens), free_ast(node), NULL);
	}
	return (node);
}

t_node	*parse_tokens(t_token **tokens)
{
	if (!tokens || !*tokens)
		return (NULL);
	return (parse_list(tokens, stop_top));
}

int	is_closer_word(t_token *tok)
{
	return (is_word(tok, "then") || is_word(tok, "else")
		|| is_word(tok, "elif") || is_word(tok, "fi") || is_word(tok, "do")
		|| is_word(tok, "done") || is_word(tok, "esac")
		|| is_word(tok, "}"));
}
