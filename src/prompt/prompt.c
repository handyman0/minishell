#include "../../includes/prompt.h"
#include <string.h>

static char	*cwd_text(t_shell *shell, int base_only)
{
	char	cwd[4096];
	char	*home;
	char	*slash;
	size_t	hlen;

	if (!getcwd(cwd, sizeof(cwd)))
		return (ft_strdup("?"));
	home = env_get(shell->env, "HOME");
	hlen = home ? ft_strlen(home) : 0;
	if (hlen > 0 && ft_strncmp(cwd, home, hlen) == 0
		&& (cwd[hlen] == '/' || cwd[hlen] == '\0'))
	{
		if (base_only && cwd[hlen] == '\0')
			return (ft_strdup("~"));
		if (!base_only)
			return (ft_strjoin("~", cwd + hlen));
	}
	slash = ft_strrchr(cwd, '/');
	if (base_only && slash && slash[1])
		return (ft_strdup(slash + 1));
	return (ft_strdup(cwd));
}

static char	*host_text(void)
{
	char	host[256];
	char	*dot;

	if (gethostname(host, sizeof(host)) != 0)
		return (ft_strdup("host"));
	host[sizeof(host) - 1] = '\0';
	dot = ft_strchr(host, '.');
	if (dot)
		*dot = '\0';
	return (ft_strdup(host));
}

static char	*user_text(t_shell *shell)
{
	char	*user;

	user = env_get(shell->env, "USER");
	if (!user)
		user = env_get(shell->env, "LOGNAME");
	return (ft_strdup(user ? user : "user"));
}

static char	*escape_text(char c, t_shell *shell)
{
	if (c == 'u')
		return (user_text(shell));
	if (c == 'h')
		return (host_text());
	if (c == 'w')
		return (cwd_text(shell, 0));
	if (c == 'W')
		return (cwd_text(shell, 1));
	if (c == '$')
		return (ft_strdup(geteuid() == 0 ? "#" : "$"));
	if (c == 'n')
		return (ft_strdup("\n"));
	if (c == 'e')
		return (ft_strdup("\033"));
	if (c == 's')
		return (ft_strdup("minishell"));
	if (c == '?')
		return (ft_itoa(shell->exit_status));
	if (c == '\\')
		return (ft_strdup("\\"));
	if (c == '[' || c == ']')
		return (ft_strdup(""));
	return (NULL);
}

static void	put_escape(t_out *out, const char *ps1, size_t *i, t_shell *shell)
{
	char	*text;

	text = escape_text(ps1[*i + 1], shell);
	if (text)
	{
		out_put(out, text, ft_strlen(text));
		free(text);
		*i += 2;
		return ;
	}
	out_put(out, "\\", 1);
	*i += 1;
}

char	*build_prompt(t_shell *shell)
{
	t_out		out;
	const char	*ps1;
	size_t		i;

	out.s = NULL;
	out.len = 0;
	out.cap = 0;
	ps1 = env_get(shell->env, "PS1");
	if (!ps1)
		ps1 = DEFAULT_PS1;
	i = 0;
	while (ps1[i])
	{
		if (ps1[i] == '\\' && ps1[i + 1])
			put_escape(&out, ps1, &i, shell);
		else
			out_put(&out, ps1 + i++, 1);
	}
	out_put(&out, "", 1);
	return (out.s);
}
