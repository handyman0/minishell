#include "../../includes/executor.h"

struct s_saved
{
	char	*key;
	char	*value;
	int		existed;
	int		exported;
};

static t_env	*find_var(t_env *env, const char *key)
{
	while (env && ft_strcmp(env->key, key) != 0)
		env = env->next;
	return (env);
}

static void	split_assign(const char *assign, char **key, char **value)
{
	char	*equal;

	equal = ft_strchr(assign, '=');
	*key = ft_substr(assign, 0, equal - assign);
	*value = equal + 1;
}

void	assign_shell(t_shell *shell, char **assigns)
{
	char	*key;
	char	*value;
	int		i;

	i = 0;
	while (assigns && assigns[i])
	{
		split_assign(assigns[i++], &key, &value);
		if (key)
			env_assign(&shell->env, key, value);
		free(key);
	}
}

t_saved	*overlay_apply(t_shell *shell, char **assigns, int *count)
{
	t_saved	*saved;
	t_env	*var;
	char	*value;
	int		i;

	*count = 0;
	while (assigns[*count])
		(*count)++;
	saved = ft_calloc(*count + 1, sizeof(t_saved));
	i = -1;
	while (saved && ++i < *count)
	{
		split_assign(assigns[i], &saved[i].key, &value);
		var = find_var(shell->env, saved[i].key);
		saved[i].existed = var != NULL;
		saved[i].exported = var && var->exported;
		saved[i].value = var ? ft_strdup(var->value) : NULL;
		env_set(&shell->env, saved[i].key, value);
		env_export(shell->env, saved[i].key);
	}
	return (saved);
}

void	overlay_restore(t_shell *shell, t_saved *saved, int count)
{
	t_env	*var;
	int		i;

	i = count;
	while (saved && i-- > 0)
	{
		if (!saved[i].existed)
			env_unset(&shell->env, saved[i].key);
		else
		{
			env_set(&shell->env, saved[i].key, saved[i].value);
			var = find_var(shell->env, saved[i].key);
			if (var)
				var->exported = saved[i].exported;
		}
		free(saved[i].key);
		free(saved[i].value);
	}
	free(saved);
}
