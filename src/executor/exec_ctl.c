#include "../../includes/executor.h"
#include "../../includes/output.h"

#define FUNC_MAX 400

static int	pending(t_shell *shell)
{
	return (!shell->running || shell->brk || shell->cont || shell->ret);
}

static int	loop_control(t_shell *shell)
{
	if (!shell->running || shell->ret)
		return (1);
	if (shell->brk)
	{
		shell->brk--;
		return (1);
	}
	if (shell->cont)
	{
		shell->cont--;
		return (shell->cont > 0);
	}
	return (0);
}

static int	execute_if(t_node *node, t_shell *shell)
{
	int	status;

	shell->noerrexit++;
	status = execute_tree(node->data.ctl.a, shell);
	shell->noerrexit--;
	shell->exit_status = status;
	if (pending(shell))
		return (status);
	if (status == 0)
		return (execute_tree(node->data.ctl.b, shell));
	if (node->data.ctl.c)
		return (execute_tree(node->data.ctl.c, shell));
	return (0);
}

static int	execute_while(t_node *node, t_shell *shell)
{
	int	status;
	int	cond;

	status = 0;
	shell->loop_depth++;
	while (1)
	{
		shell->noerrexit++;
		cond = execute_tree(node->data.ctl.a, shell);
		shell->noerrexit--;
		if (pending(shell) || (cond == 0) == (node->data.ctl.flag != 0))
			break ;
		status = execute_tree(node->data.ctl.b, shell);
		shell->exit_status = status;
		if (loop_control(shell))
			break ;
	}
	shell->loop_depth--;
	return (status);
}

static char	**copy_params(t_shell *shell)
{
	char	**out;
	int		i;

	out = ft_calloc(shell->nparams + 1, sizeof(char *));
	i = 0;
	while (out && i < shell->nparams)
	{
		out[i] = ft_strdup(shell->params[i]);
		i++;
	}
	return (out);
}

static int	execute_for(t_node *node, t_shell *shell)
{
	char	**items;
	int		status;
	int		i;

	items = copy_params(shell);
	if (node->data.ctl.flag)
	{
		free_str_array(items);
		items = expand_words(node->data.ctl.words, shell);
	}
	status = 0;
	shell->loop_depth++;
	i = 0;
	while (items && items[i])
	{
		if (env_is_readonly(shell->env, node->data.ctl.name))
		{
			ml_put_all(2, "minishell: ", node->data.ctl.name,
				": readonly variable\n", NULL);
			shell->running = shell->spare;
			status = 1;
			break ;
		}
		env_assign(&shell->env, node->data.ctl.name, items[i++]);
		status = execute_tree(node->data.ctl.a, shell);
		shell->exit_status = status;
		if (loop_control(shell))
			break ;
	}
	shell->loop_depth--;
	free_str_array(items);
	return (status);
}

static int	case_matches(t_case *item, const char *word, t_shell *shell)
{
	char	*pattern;
	int		hit;
	int		i;

	hit = 0;
	i = 0;
	while (!hit && item->patterns[i])
	{
		pattern = expand_variables(item->patterns[i++], shell);
		hit = pattern && glob_match(pattern, word);
		free(pattern);
	}
	return (hit);
}

static int	execute_case(t_node *node, t_shell *shell)
{
	t_case	*item;
	char	*word;
	int		status;

	word = expand_variables(node->data.ctl.words[0], shell);
	status = 0;
	item = node->data.ctl.cases;
	while (word && item)
	{
		if (case_matches(item, word, shell))
		{
			if (item->body)
				status = execute_tree(item->body, shell);
			break ;
		}
		item = item->next;
	}
	free(word);
	return (status);
}

static int	execute_group(t_node *node, t_shell *shell)
{
	return (execute_tree(node->data.ctl.a, shell));
}

t_func	*find_func(t_shell *shell, const char *name)
{
	t_func	*func;

	func = shell->funcs;
	while (func && ft_strcmp(func->name, name) != 0)
		func = func->next;
	return (func);
}

static int	define_function(t_node *node, t_shell *shell)
{
	t_func	*func;

	if (!node->data.ctl.a)
		return (0);
	func = find_func(shell, node->data.ctl.name);
	if (func)
		free_ast(func->body);
	else
	{
		func = ft_calloc(1, sizeof(t_func));
		if (!func)
			return (1);
		func->name = ft_strdup(node->data.ctl.name);
		func->next = shell->funcs;
		shell->funcs = func;
	}
	func->body = node->data.ctl.a;
	node->data.ctl.a = NULL;
	return (0);
}

void	funcs_free(t_shell *shell)
{
	t_func	*func;
	t_func	*next;

	func = shell->funcs;
	while (func)
	{
		next = func->next;
		free_ast(func->body);
		free(func->name);
		free(func);
		func = next;
	}
	shell->funcs = NULL;
}

static t_env	*find_var(t_env *env, const char *key)
{
	while (env && ft_strcmp(env->key, key) != 0)
		env = env->next;
	return (env);
}

void	push_local(t_shell *shell, const char *key)
{
	t_local	*local;
	t_env	*var;

	local = ft_calloc(1, sizeof(t_local));
	if (!local)
		return ;
	var = find_var(shell->env, key);
	local->key = ft_strdup(key);
	local->had = var != NULL;
	local->old = var ? ft_strdup(var->value) : NULL;
	local->was_exported = var && var->exported;
	local->depth = shell->func_depth;
	local->next = shell->locals;
	shell->locals = local;
}

static void	pop_locals(t_shell *shell, int depth)
{
	t_local	*local;
	t_env	*var;

	while (shell->locals && shell->locals->depth >= depth)
	{
		local = shell->locals;
		shell->locals = local->next;
		if (!local->had)
			env_unset(&shell->env, local->key);
		else
		{
			env_set(&shell->env, local->key, local->old);
			var = find_var(shell->env, local->key);
			if (var)
				var->exported = local->was_exported;
		}
		free(local->key);
		free(local->old);
		free(local);
	}
}

static char	**args_after_name(char **argv)
{
	char	**out;
	int		n;
	int		i;

	n = 0;
	while (argv[n + 1])
		n++;
	out = ft_calloc(n + 1, sizeof(char *));
	i = 0;
	while (out && i < n)
	{
		out[i] = ft_strdup(argv[i + 1]);
		i++;
	}
	return (out);
}

int	call_function(t_shell *shell, t_func *func, char **argv)
{
	char	**saved;
	int		saved_n;
	int		status;

	if (shell->func_depth >= FUNC_MAX)
		return (ml_put_all(2, "minishell: ", argv[0],
				": maximum function nesting level exceeded\n", NULL), 1);
	saved = shell->params;
	saved_n = shell->nparams;
	shell->params = args_after_name(argv);
	shell->nparams = 0;
	while (shell->params && shell->params[shell->nparams])
		shell->nparams++;
	shell->func_depth++;
	status = execute_tree(func->body, shell);
	shell->ret = 0;
	pop_locals(shell, shell->func_depth);
	shell->func_depth--;
	free_str_array(shell->params);
	shell->params = saved;
	shell->nparams = saved_n;
	return (status);
}

static int	with_redirs(t_node *node, t_shell *shell,
				int (*fn)(t_node *, t_shell *))
{
	t_node	fake;
	t_node	*copy;
	int		backup[3];
	int		status;

	if (!node->data.ctl.redirs)
		return (fn(node, shell));
	ft_bzero(&fake, sizeof(fake));
	fake.type = NODE_CMD;
	fake.data.cmd.line = shell->lineno;
	fake.data.cmd.redirs = node->data.ctl.redirs;
	copy = clone_command(&fake);
	if (!copy)
		return (1);
	expand_command(copy, shell);
	save_fds(backup);
	if (!apply_redirections(copy->data.cmd.redirs))
	{
		restore_fds(backup);
		return (free_ast(copy), 1);
	}
	status = fn(node, shell);
	restore_fds(backup);
	free_ast(copy);
	return (status);
}

int	execute_ctl(t_node *node, t_shell *shell)
{
	if (node->type == NODE_FUNC)
		return (define_function(node, shell));
	if (node->type == NODE_IF)
		return (with_redirs(node, shell, execute_if));
	if (node->type == NODE_PAREN)
		return (with_redirs(node, shell, execute_subshell));
	if (node->type == NODE_WHILE)
		return (with_redirs(node, shell, execute_while));
	if (node->type == NODE_FOR)
		return (with_redirs(node, shell, execute_for));
	if (node->type == NODE_CASE)
		return (with_redirs(node, shell, execute_case));
	return (with_redirs(node, shell, execute_group));
}

void	func_remove(t_shell *shell, const char *name)
{
	t_func	**link;
	t_func	*func;

	link = &shell->funcs;
	while (*link && ft_strcmp((*link)->name, name) != 0)
		link = &(*link)->next;
	func = *link;
	if (!func)
		return ;
	*link = func->next;
	free_ast(func->body);
	free(func->name);
	free(func);
}
