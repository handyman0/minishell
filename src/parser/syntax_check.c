#include "../../includes/parser.h"
#include "../../includes/minishell.h"

static int	is_binop(t_token *tok)
{
	return (tok->type == TOKEN_PIPE || tok->type == TOKEN_AND
		|| tok->type == TOKEN_OR);
}

static int	is_sep(t_token *tok)
{
	return (tok->type == TOKEN_SEMI || tok->type == TOKEN_BG);
}

const char	*token_text(t_token *tok)
{
	if (!tok)
		return ("newline");
	if (tok->type == TOKEN_PIPE)
		return ("|");
	if (tok->type == TOKEN_AND)
		return ("&&");
	if (tok->type == TOKEN_OR)
		return ("||");
	if (tok->type == TOKEN_SEMI)
		return (";");
	if (tok->type == TOKEN_BG)
		return ("&");
	if (tok->type == TOKEN_LPAREN)
		return ("(");
	if (tok->type == TOKEN_RPAREN)
		return (")");
	if (tok->type == TOKEN_REDIR_IN)
		return ("<");
	if (tok->type == TOKEN_REDIR_OUT)
		return (">");
	if (tok->type == TOKEN_REDIR_CLOBBER)
		return (">|");
	if (tok->type == TOKEN_REDIR_APPEND)
		return (">>");
	if (tok->type == TOKEN_HEREDOC)
		return ("<<");
	if (tok->type == TOKEN_HEREDOC_DASH)
		return ("<<-");
	if (tok->type == TOKEN_DUP_IN)
		return ("<&");
	if (tok->type == TOKEN_DUP_OUT)
		return (">&");
	if (tok->type == TOKEN_RDWR)
		return ("<>");
	if (tok->type == TOKEN_HERESTR)
		return ("<<<");
	if (tok->type == TOKEN_DSEMI)
		return (";;");
	return (tok->value ? tok->value : "?");
}

static int	syntax_error(t_token *tok)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	ft_putstr_fd((char *)token_text(tok), 2);
	ft_putstr_fd("'\n", 2);
	return (2);
}

static int	bad_follow(t_token *cur)
{
	t_token	*next;

	next = cur->next;
	if (cur->type == TOKEN_WORD && next && next->type == TOKEN_LPAREN)
		return (1);
	if (is_binop(cur) || cur->type == TOKEN_LPAREN)
		return (!next || is_binop(next) || is_sep(next)
			|| next->type == TOKEN_RPAREN);
	if (is_redirection_token(cur))
		return (!next || next->type != TOKEN_WORD);
	if (is_sep(cur))
		return (next && (is_binop(next) || is_sep(next)));
	if (cur->type == TOKEN_RPAREN)
		return (next && (next->type == TOKEN_WORD
				|| next->type == TOKEN_LPAREN));
	return (0);
}

static int	has_compound_syntax(t_token *tok)
{
	while (tok)
	{
		if (tok->type == TOKEN_WORD && (ft_strcmp(tok->value, "case") == 0
				|| ft_strcmp(tok->value, "function") == 0))
			return (1);
		if (tok->type == TOKEN_WORD && tok->next
			&& tok->next->type == TOKEN_LPAREN && tok->next->next
			&& tok->next->next->type == TOKEN_RPAREN)
			return (1);
		tok = tok->next;
	}
	return (0);
}

int	check_syntax(t_token *tokens)
{
	t_token	*cur;
	int		depth;

	if (has_compound_syntax(tokens))
		return (0);
	if (tokens && (is_binop(tokens) || is_sep(tokens)
			|| tokens->type == TOKEN_RPAREN))
		return (syntax_error(tokens));
	depth = 0;
	cur = tokens;
	while (cur)
	{
		if (cur->type == TOKEN_LPAREN)
			depth++;
		if (cur->type == TOKEN_RPAREN && --depth < 0)
			return (syntax_error(cur));
		if (bad_follow(cur))
			return (syntax_error(cur->next));
		cur = cur->next;
	}
	if (depth > 0)
		return (syntax_error(NULL));
	return (0);
}

static int	skip_open(const char *line, int i, char *unclosed)
{
	int	next;

	if (line[i] == '`')
	{
		next = skip_backtick(line, i);
		*unclosed = '`';
	}
	else if (line[i] == '$' && line[i + 1] == '{')
	{
		next = skip_brace(line, i);
		*unclosed = '}';
	}
	else if (line[i] == '$')
	{
		next = skip_dollar_paren(line, i);
		*unclosed = ')';
	}
	else
	{
		next = skip_quote(line, i);
		*unclosed = line[i];
	}
	return (next);
}

char	open_quote(const char *line)
{
	char	unclosed;
	int		i;
	int		next;

	i = 0;
	while (line[i])
	{
		if (line[i] == '\\' && line[i + 1])
			i += 2;
		else if (line[i] == '\'' || line[i] == '"' || line[i] == '`'
			|| (line[i] == '$' && (line[i + 1] == '(' || line[i + 1] == '{')))
		{
			next = skip_open(line, i, &unclosed);
			if (next == -1)
				return (unclosed);
			i = next;
		}
		else
			i++;
	}
	return (0);
}

int	check_quotes(char *line)
{
	char	quote;

	quote = open_quote(line);
	if (!quote)
		return (0);
	ft_putstr_fd("minishell: unexpected EOF while looking for matching `", 2);
	ft_putchar_fd(quote, 2);
	ft_putstr_fd("'\n", 2);
	return (2);
}

static int	trailing_backslash(const char *line)
{
	char	quote;

	quote = 0;
	while (*line)
	{
		if (*line == '\\' && quote != '\'')
		{
			if (!line[1])
				return (1);
			line++;
		}
		else if (!quote && (*line == '\'' || *line == '"'))
			quote = *line;
		else if (quote && *line == quote)
			quote = 0;
		line++;
	}
	return (0);
}

int	line_incomplete(char *line)
{
	t_token	*tokens;
	t_token	*cursor;
	t_node	*ast;
	int		result;

	if (open_quote(line) || trailing_backslash(line))
		return (1);
	tokens = tokenize_line(line);
	cursor = tokens;
	parse_reset();
	ast = parse_tokens(&cursor);
	result = parse_failed() && parse_hit_eof();
	free_ast(ast);
	free_tokens(tokens);
	parse_reset();
	return (result);
}
