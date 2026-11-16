#include "../../includes/builtins.h"
#include "../includes/output.h"

int	builtin_pwd(char **argv, t_shell *shell)
{
	char	*cwd;
	int		physical;
	int		i;

	physical = 0;
	i = 1;
	while (argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (ft_strcmp(argv[i], "-P") == 0)
			physical = 1;
		else if (ft_strcmp(argv[i], "-L") == 0)
			physical = 0;
		else if (ft_strcmp(argv[i], "--") == 0)
			break ;
		else
			return (ml_put_all(2, "minishell: pwd: invalid option\n", NULL), 2);
		i++;
	}
	cwd = physical ? getcwd(NULL, 0) : logical_pwd(shell);
	if (!cwd)
		return (put_errno("pwd", NULL), 1);
	ml_put_all(STDOUT_FILENO, cwd, "\n", NULL);
	free(cwd);
	return (0);
}
