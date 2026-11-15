#include "../../includes/completion.h"
#include <dirent.h>
#include <readline/readline.h>

static t_shell	*g_completion_shell = NULL;
static t_list	*g_matches = NULL;
static t_list	*g_cursor = NULL;

static const char	*g_builtins[] = {"echo", "cd", "pwd", "export", "unset",
	"env", "exit", NULL};

static int	has_match(const char *name)
{
	t_list	*cur;

	cur = g_matches;
	while (cur)
	{
		if (ft_strcmp(cur->content, name) == 0)
			return (1);
		cur = cur->next;
	}
	return (0);
}

static void	add_match(const char *name)
{
	if (!has_match(name))
		ft_lstadd_back(&g_matches, ft_lstnew(ft_strdup(name)));
}

static void	scan_dir(const char *dir, const char *text)
{
	DIR				*d;
	struct dirent	*entry;
	char			*full;
	char			*prefix;

	d = opendir(dir);
	if (!d)
		return ;
	prefix = ft_strjoin(dir, "/");
	entry = readdir(d);
	while (entry)
	{
		if (ft_strncmp(entry->d_name, text, ft_strlen(text)) == 0
			&& (entry->d_name[0] != '.' || text[0] == '.'))
		{
			full = ft_strjoin(prefix, entry->d_name);
			if (full && access(full, X_OK) == 0)
				add_match(entry->d_name);
			free(full);
		}
		entry = readdir(d);
	}
	free(prefix);
	closedir(d);
}

static void	build_matches(const char *text)
{
	char	**dirs;
	char	*path;
	int		i;

	i = 0;
	while (g_builtins[i])
	{
		if (ft_strncmp(g_builtins[i], text, ft_strlen(text)) == 0)
			add_match(g_builtins[i]);
		i++;
	}
	path = env_get(g_completion_shell->env, "PATH");
	dirs = ft_split(path ? path : "", ':');
	i = 0;
	while (dirs && dirs[i])
		scan_dir(dirs[i++], text);
	free_str_array(dirs);
	g_cursor = g_matches;
}

static char	*next_match(void)
{
	char	*result;

	if (!g_cursor)
	{
		ft_lstclear(&g_matches, free);
		return (NULL);
	}
	result = ft_strdup(g_cursor->content);
	g_cursor = g_cursor->next;
	return (result);
}

static char	*command_generator(const char *text, int state)
{
	if (state == 0)
	{
		ft_lstclear(&g_matches, free);
		build_matches(text);
	}
	return (next_match());
}

static void	build_var_matches(const char *text)
{
	t_env	*env;

	env = g_completion_shell->env;
	while (env)
	{
		if (ft_strncmp(env->key, text, ft_strlen(text)) == 0)
			add_match(env->key);
		env = env->next;
	}
	g_cursor = g_matches;
}

static char	*variable_generator(const char *text, int state)
{
	if (state == 0)
	{
		ft_lstclear(&g_matches, free);
		build_var_matches(text);
	}
	return (next_match());
}

static int	is_command_position(int start)
{
	int	i;

	i = start - 1;
	while (i >= 0 && rl_line_buffer[i] == ' ')
		i--;
	return (i < 0 || ft_strchr("|&;(", rl_line_buffer[i]) != NULL);
}

static char	**complete_hook(const char *text, int start, int end)
{
	(void)end;
	if (start > 0 && rl_line_buffer[start - 1] == '$')
	{
		rl_attempted_completion_over = 1;
		return (rl_completion_matches(text, variable_generator));
	}
	if (!is_command_position(start) || ft_strchr(text, '/'))
		return (NULL);
	rl_attempted_completion_over = 1;
	return (rl_completion_matches(text, command_generator));
}

void	setup_completion(t_shell *shell)
{
	g_completion_shell = shell;
	rl_attempted_completion_function = complete_hook;
}
