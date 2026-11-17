#include "../../includes/parser.h"
#include "../../includes/minishell.h"
#include <fcntl.h>
#include <unistd.h>

int	tilde_name_len(const char *s, int assign)
{
	int	n;

	n = 0;
	while (s[n] && s[n] != '/' && !(assign && s[n] == ':'))
	{
		if (ft_strchr("\\'\"$`", s[n]))
			return (-1);
		n++;
	}
	return (n);
}

static char	*home_field(char *colon)
{
	char	*end;
	int		k;

	k = 0;
	while (colon && k++ < 4)
		colon = ft_strchr(colon + 1, ':');
	if (!colon)
		return (NULL);
	end = colon + 1;
	while (*end && *end != ':' && *end != '\n')
		end++;
	return (ft_substr(colon + 1, 0, end - colon - 1));
}

static char	*passwd_home(const char *name, int n)
{
	char	buf[65536];
	char	*line;
	ssize_t	len;
	int		fd;

	fd = open("/etc/passwd", O_RDONLY);
	if (fd < 0)
		return (NULL);
	len = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (len <= 0)
		return (NULL);
	buf[len] = '\0';
	line = buf;
	while (*line)
	{
		if (ft_strncmp(line, name, n) == 0 && line[n] == ':')
			return (home_field(line + n));
		line = ft_strchr(line, '\n');
		if (!line)
			break ;
		line++;
	}
	return (NULL);
}

char	*tilde_lookup(const char *name, int n, t_shell *shell)
{
	char	*var;

	var = NULL;
	if (n == 0)
		var = env_get(shell->env, "HOME");
	else if (n == 1 && name[0] == '+')
		var = env_get(shell->env, "PWD");
	else if (n == 1 && name[0] == '-')
		var = env_get(shell->env, "OLDPWD");
	else
		return (passwd_home(name, n));
	return (var ? ft_strdup(var) : NULL);
}
