#include "../../includes/parser.h"
#include "../../includes/minishell.h"
#include <time.h>
#include <unistd.h>

static unsigned int	next_random(t_shell *shell)
{
	static unsigned int	state;

	if (!state)
		state = (unsigned int)(shell->pid * 2654435761u) ^ (unsigned int)time(NULL);
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return (state);
}

static char	*host_name(void)
{
	char	buf[256];

	if (gethostname(buf, sizeof(buf) - 1) != 0)
		return (NULL);
	buf[sizeof(buf) - 1] = '\0';
	return (ft_strdup(buf));
}

char	*dynamic_value(const char *name, t_shell *shell)
{
	if (!ft_strchr("RSPUEH", name[0]))
		return (NULL);
	if (ft_strcmp(name, "RANDOM") == 0)
		return (ft_itoa((int)(next_random(shell) & 32767)));
	if (ft_strcmp(name, "SECONDS") == 0)
		return (ft_itoa((int)(time(NULL) - shell->start_time)));
	if (ft_strcmp(name, "PPID") == 0)
		return (ft_itoa(shell->ppid));
	if (ft_strcmp(name, "UID") == 0)
		return (ft_itoa((int)getuid()));
	if (ft_strcmp(name, "EUID") == 0)
		return (ft_itoa((int)geteuid()));
	if (ft_strcmp(name, "HOSTNAME") == 0 && !env_get(shell->env, name))
		return (host_name());
	return (NULL);
}
