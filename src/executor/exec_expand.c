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
		if (redir->type == REDIR_HERESTR)
			redir->hd_fd = herestring_fd(expanded);
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
	return (assign_eq_index(word) > 0);
}

static char	**literal_elements(const char *inner, t_shell *shell, int *n)
{
	t_token	*tokens;
	char	*copy;
	char	**words;

	copy = ft_strdup(inner);
	tokens = tokenize_line(copy);
	free(copy);
	expand_tokens(&tokens, shell);
	words = tokens_to_argv(tokens);
	*n = 0;
	while (words && words[*n])
		(*n)++;
	return (words);
}

static char	*array_literal(const char *base, int plus, const char *raw,
				t_shell *shell)
{
	char	*inner;
	char	**fresh;
	char	**old;
	char	**all;
	int		nf;
	int		no;

	inner = ft_substr(raw, 1, ft_strlen(raw) - 2);
	fresh = literal_elements(inner, shell, &nf);
	old = plus && base ? arr_split(base, &no) : NULL;
	no = old ? arr_count(base) : 0;
	all = ft_calloc(no + nf + 1, sizeof(char *));
	ft_memcpy(all, old, sizeof(char *) * no);
	ft_memcpy(all + no, fresh, sizeof(char *) * nf);
	free(old);
	free(fresh);
	free(inner);
	inner = arr_join(all, no + nf);
	return (free_str_array(all), inner);
}

static char	*assigned_value(const char *name, int eq, const char *word,
				t_shell *shell)
{
	char	*base;
	char	*sub;
	char	*val;
	char	*res;
	int		plus;
	int		open;

	plus = word[eq - 1] == '+';
	base = env_get(shell->env, name);
	if (word[eq + 1] == '(' && word[ft_strlen(word) - 1] == ')')
		return (array_literal(base, plus, word + eq + 1, shell));
	val = expand_variables((char *)word + eq + 1, shell);
	sub = NULL;
	if (word[eq - 1 - plus] == ']')
	{
		open = eq - 1 - plus;
		while (open > 0 && word[open] != '[')
			open--;
		sub = ft_substr(word, open + 1, eq - 1 - plus - open - 1);
	}
	res = assign_with_index(base, sub, plus, val, shell);
	return (free(sub), free(val), res);
}

static char	*build_assignment(char *word, t_shell *shell)
{
	char	*name;
	char	*value;
	char	*result;
	int		eq;
	int		n;

	eq = assign_eq_index(word);
	n = 0;
	while (ft_isalnum(word[n]) || word[n] == '_')
		n++;
	name = ft_substr(word, 0, n);
	value = assigned_value(name, eq, word, shell);
	result = ft_strjoin(name, "=");
	free(name);
	name = ft_strjoin(result, value ? value : "");
	return (free(result), free(value), name);
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

static int	is_decl_builtin(const char *name)
{
	return (name && (ft_strcmp(name, "declare") == 0
			|| ft_strcmp(name, "typeset") == 0 || ft_strcmp(name, "local") == 0
			|| ft_strcmp(name, "export") == 0
			|| ft_strcmp(name, "readonly") == 0));
}

static char	**append_word(char **list, int *n, char *word)
{
	char	**grown;

	grown = ft_calloc(*n + 2, sizeof(char *));
	if (!grown)
		return (free(word), list);
	ft_memcpy(grown, list, sizeof(char *) * *n);
	free(list);
	grown[(*n)++] = word;
	return (grown);
}

static char	**expand_decl(char **argv, t_shell *shell)
{
	char	**out;
	char	**part;
	char	*one[2];
	int		n;
	int		i;
	int		k;

	out = ft_calloc(1, sizeof(char *));
	n = 0;
	i = -1;
	one[1] = NULL;
	while (argv[++i])
	{
		if (i > 0 && assign_eq_index(argv[i]) > 0
			&& ft_strchr(argv[i], '('))
			out = append_word(out, &n, build_assignment(argv[i], shell));
		else
		{
			one[0] = argv[i];
			part = expand_words(one, shell);
			k = 0;
			while (part && part[k])
				out = append_word(out, &n, ft_strdup(part[k++]));
			free_str_array(part);
		}
	}
	return (out);
}

void	expand_command(t_node *node, t_shell *shell)
{
	t_token	*tokens;
	char	**decl;

	expand_redirs(node->data.cmd.redirs, shell);
	if (!node->data.cmd.argv)
		return ;
	take_assignments(node, shell);
	if (node->data.cmd.argv[0] && is_decl_builtin(node->data.cmd.argv[0]))
	{
		decl = expand_decl(node->data.cmd.argv, shell);
		free_str_array(node->data.cmd.argv);
		node->data.cmd.argv = decl;
		return ;
	}
	tokens = argv_to_tokens(node->data.cmd.argv);
	expand_tokens(&tokens, shell);
	free_str_array(node->data.cmd.argv);
	node->data.cmd.argv = tokens_to_argv(tokens);
}

char	**expand_words(char **raw, t_shell *shell)
{
	t_token	*tokens;

	tokens = argv_to_tokens(raw);
	expand_tokens(&tokens, shell);
	return (tokens_to_argv(tokens));
}
