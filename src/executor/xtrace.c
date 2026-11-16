#include "../../includes/executor.h"
#include "../../includes/output.h"

static int	plain_word(const char *w)
{
	if (!*w)
		return (0);
	while (*w)
	{
		if (!ft_isalnum(*w) && !ft_strchr("_./:=@%+,-", *w))
			return (0);
		w++;
	}
	return (1);
}

static char	*append(char *text, const char *word)
{
	char	*quoted;
	char	*tmp;
	char	*out;

	quoted = plain_word(word) ? ft_strdup(word) : quote_word(word);
	tmp = ft_strjoin(text, *text ? " " : "");
	out = ft_strjoin(tmp, quoted);
	free(tmp);
	free(quoted);
	free(text);
	return (out);
}

void	xtrace_print(t_shell *shell, char **argv, char **assigns)
{
	char	*ps4;
	char	*text;
	int		i;

	ps4 = expand_variables(env_get(shell->env, "PS4")
			? env_get(shell->env, "PS4") : "+ ", shell);
	text = ft_strdup("");
	i = 0;
	while (assigns && assigns[i])
		text = append(text, assigns[i++]);
	i = 0;
	while (argv && argv[i])
		text = append(text, argv[i++]);
	ml_put_all(2, ps4 ? ps4 : "+ ", text, "\n", NULL);
	free(ps4);
	free(text);
}
