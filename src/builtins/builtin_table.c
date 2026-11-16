#include "../includes/builtins.h"
#include "../includes/executor.h"

typedef struct s_ext
{
	const char	*name;
	int			(*fn)(char **argv, t_shell *shell);
}	t_ext;

static const t_ext	g_ext[] = {{"true", builtin_true}, {"false", builtin_false},
{":", builtin_true}, {"break", builtin_break}, {"continue", builtin_continue},
{"return", builtin_return}, {"local", builtin_local}, {"shift", builtin_shift},
{"set", builtin_set}, {"eval", builtin_eval}, {"test", builtin_test},
{"[", builtin_test}, {"read", builtin_read}, {"umask", builtin_umask},
{"command", builtin_command}, {"times", builtin_times}, {"readonly", builtin_readonly}, {".", builtin_source}, {NULL, NULL}};

static const char	*g_builtin_names[] = {"echo", "cd", "pwd", "export",
	"unset", "env", "exit", "jobs", "fg", "bg", "alias", "unalias", "source",
	"type", "which", "true", "false", "break", "continue", "return", "local",
	"shift", "set", "eval", "test", "[", "read", ".", ":", "umask", "command", "times", "exec", "readonly", NULL};

const char	**builtin_names(void)
{
	return (g_builtin_names);
}

int	(*find_ext_builtin(const char *name))(char **, t_shell *)
{
	int	i;

	i = 0;
	while (name && g_ext[i].name)
	{
		if (ft_strcmp(name, g_ext[i].name) == 0)
			return (g_ext[i].fn);
		i++;
	}
	return (NULL);
}

int	is_builtin(const char *name)
{
	int	i;

	i = 0;
	while (name && g_builtin_names[i])
	{
		if (ft_strcmp(name, g_builtin_names[i]) == 0)
			return (1);
		i++;
	}
	return (0);
}

int	is_special_builtin(const char *name)
{
	static const char	*special[] = {"break", ":", ".", "continue", "eval",
		"exec", "exit", "export", "readonly", "return", "set", "shift",
		"times", "trap", "unset", NULL};
	int					i;

	i = 0;
	while (name && special[i])
	{
		if (ft_strcmp(name, special[i++]) == 0)
			return (1);
	}
	return (0);
}
