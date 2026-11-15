#include "../includes/builtins.h"

static const char	*g_builtin_names[] = {"echo", "cd", "pwd", "export",
	"unset", "env", "exit", "jobs", "fg", "bg", NULL};

const char	**builtin_names(void)
{
	return (g_builtin_names);
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
