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

static const char	*token_text(t_token *tok)
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
	if (tok->type == TOKEN_REDIR_APPEND)
		return (">>");
	if (tok->type == TOKEN_HEREDOC)
		return ("<<");
	return (tok->value);
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

int	check_syntax(t_token *tokens)
{
	t_token	*cur;
	int		depth;

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

char	open_quote(const char *line)
{
	char	quote;

	quote = 0;
	while (*line)
	{
		if (*line == '\\' && quote != '\'' && line[1])
			line++;
		else if (!quote && (*line == '\'' || *line == '"'))
			quote = *line;
		else if (quote && *line == quote)
			quote = 0;
		line++;
	}
	return (quote);
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
