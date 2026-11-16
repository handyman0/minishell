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
		token_add_back(&head, &cur, TOKEN_WORD, ft_strdup(argv[i]));
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

static int	is_assignment(const char *word)
{
	int	i;

	if (!ft_isalpha(word[0]) && word[0] != '_')
		return (0);
	i = 0;
	while (ft_isalnum(word[i]) || word[i] == '_')
		i++;
	return (word[i] == '=');
}

static char	*build_assignment(char *word, t_shell *shell)
{
	char	*equal;
	char	*name;
	char	*value;
	char	*tmp;
	char	*result;

	equal = ft_strchr(word, '=');
	name = ft_substr(word, 0, equal - word);
	value = expand_variables(equal + 1, shell);
	tmp = ft_strjoin(name, "=");
	result = ft_strjoin(tmp, value ? value : "");
	free(name);
	free(value);
	free(tmp);
	return (result);
}

static char	**expand_assignments(char **argv, int count, t_shell *shell)
{
	char	**assigns;
	t_saved	**parts;
	char	*one[2];
	int		n;
	int		i;

	assigns = ft_calloc(count + 1, sizeof(char *));
	parts = ft_calloc(count + 1, sizeof(t_saved *));
	one[1] = NULL;
	i = -1;
	while (assigns && parts && ++i < count)
	{
		assigns[i] = build_assignment(argv[i], shell);
		one[0] = assigns[i];
		parts[i] = overlay_apply(shell, one, &n);
	}
	while (parts && i-- > 0)
		overlay_restore(shell, parts[i], 1);
	free(parts);
	return (assigns);
}

static void	take_assignments(t_node *node, t_shell *shell)
{
	char	**argv;
	int		count;
	int		i;

	argv = node->data.cmd.argv;
	count = 0;
	while (argv[count] && is_assignment(argv[count]))
		count++;
	if (count == 0)
		return ;
	node->data.cmd.assigns = expand_assignments(argv, count, shell);
	i = 0;
	while (i < count)
		free(argv[i++]);
	i = 0;
	while (argv[count + i])
		i++;
	ft_memmove(argv, argv + count, sizeof(char *) * (i + 1));
}

void	expand_command(t_node *node, t_shell *shell)
{
	t_token	*tokens;

	expand_redirs(node->data.cmd.redirs, shell);
	if (!node->data.cmd.argv)
		return ;
	take_assignments(node, shell);
	tokens = argv_to_tokens(node->data.cmd.argv);
	expand_tokens(&tokens, shell);
	free_str_array(node->data.cmd.argv);
	node->data.cmd.argv = tokens_to_argv(tokens);
}
