#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"

int	builtin_hash(char **argv, t_shell *shell)
{
	char	*path;
	int		status;
	int		i;

	i = 1;
	status = 0;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "--") == 0 && ++i)
			break ;
		if (ft_strcmp(argv[i], "-r") != 0)
			return (ml_put_all(2, "minishell: hash: ", argv[i],
					": invalid option\n", NULL), 2);
		i++;
	}
	while (argv[i])
	{
		path = find_path(argv[i], shell);
		if (!path)
		{
			ml_put_all(2, "minishell: hash: ", argv[i], ": not found\n", NULL);
			status = 1;
		}
		free(path);
		i++;
	}
	return (status);
}
