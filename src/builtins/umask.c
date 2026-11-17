#include "../../includes/builtins.h"
#include "../../includes/executor.h"
#include "../../includes/output.h"
#include <sys/stat.h>

static void	put_octal(char *buf, unsigned int mask)
{
	buf[0] = '0';
	buf[1] = '0' + ((mask >> 6) & 7);
	buf[2] = '0' + ((mask >> 3) & 7);
	buf[3] = '0' + (mask & 7);
	buf[4] = '\0';
}

static void	put_class(char *out, unsigned int allowed)
{
	out[0] = (allowed & 4) ? 'r' : '\0';
	out[1] = (allowed & 2) ? 'w' : '\0';
	out[2] = (allowed & 1) ? 'x' : '\0';
}

static void	print_symbolic(unsigned int mask)
{
	char	text[40];
	char	cls[4];
	int		i;
	int		n;
	int		j;

	n = 0;
	i = 0;
	while (i < 3)
	{
		text[n++] = "ugo"[i];
		text[n++] = '=';
		put_class(cls, (~mask >> (6 - 3 * i)) & 7);
		j = 0;
		while (j < 3)
			if (cls[j++])
				text[n++] = cls[j - 1];
		if (++i < 3)
			text[n++] = ',';
	}
	text[n] = '\0';
	ml_put_all(STDOUT_FILENO, text, "\n", NULL);
}

static int	parse_octal(const char *s, unsigned int *mask)
{
	unsigned int	value;

	value = 0;
	if (!*s)
		return (0);
	while (*s)
	{
		if (*s < '0' || *s > '7')
			return (0);
		value = value * 8 + (*s++ - '0');
	}
	if (value > 0777)
		return (0);
	*mask = value;
	return (1);
}

static unsigned int	perm_rwx(const char *perms, unsigned int allowed)
{
	unsigned int	rwx;
	int				i;

	if (perms[0] && ft_strchr("ugo", perms[0]))
		return ((allowed >> (6 - 3 * (int)(ft_strchr("ugo", perms[0])
						- "ugo"))) & 7);
	rwx = 0;
	i = 0;
	while (perms[i] && ft_strchr("rwxXst", perms[i]))
		rwx |= (perms[i] == 'r') * 4 | (perms[i] == 'w') * 2
			| (perms[i] == 'x' || perms[i] == 'X') * 1, i++;
	return (rwx);
}

static unsigned int	class_bits(const char *who, unsigned int rwx)
{
	unsigned int	bits;

	bits = 0;
	while (*who)
	{
		bits |= (*who == 'u' || *who == 'a') * (rwx << 6);
		bits |= (*who == 'g' || *who == 'a') * (rwx << 3);
		bits |= (*who == 'o' || *who == 'a') * rwx;
		who++;
	}
	return (bits);
}

static int	apply_clause(const char *c, unsigned int *allowed)
{
	char			who[4];
	unsigned int	cover;
	unsigned int	bits;
	int				n;
	char			op;

	n = 0;
	while (*c && ft_strchr("ugoa", *c) && n < 3)
		who[n++] = *c++;
	who[n] = '\0';
	if (n == 0)
		ft_strlcpy(who, "a", sizeof(who));
	cover = class_bits(who, 7);
	while (*c == '+' || *c == '-' || *c == '=')
	{
		op = *c++;
		bits = class_bits(who, perm_rwx(c, *allowed));
		if (*c && ft_strchr("ugo", *c))
			c++;
		else
			while (*c && ft_strchr("rwxXst", *c))
				c++;
		if (op == '=')
			*allowed = (*allowed & ~cover) | bits;
		else
			*allowed = op == '+' ? (*allowed | bits) : (*allowed & ~bits);
	}
	return (*c == '\0' || *c == ',');
}

static int	parse_symbolic(const char *s, unsigned int *mask)
{
	unsigned int	allowed;
	const char		*comma;

	allowed = ~(*mask) & 0777;
	while (1)
	{
		if (!apply_clause(s, &allowed))
			return (0);
		comma = ft_strchr(s, ',');
		if (!comma)
			break ;
		s = comma + 1;
	}
	*mask = ~allowed & 0777;
	return (1);
}

int	builtin_umask(char **argv, t_shell *shell)
{
	unsigned int	mask;
	char			text[8];
	int				symbolic;
	int				i;

	(void)shell;
	mask = umask(0);
	umask(mask);
	symbolic = argv[1] && ft_strcmp(argv[1], "-S") == 0;
	i = 1 + symbolic;
	if (!argv[i])
	{
		put_octal(text, mask);
		if (symbolic)
			print_symbolic(mask);
		else
			ml_put_all(STDOUT_FILENO, text, "\n", NULL);
		return (0);
	}
	if (!parse_octal(argv[i], &mask) && !parse_symbolic(argv[i], &mask))
		return (ml_put_all(2, "minishell: umask: ", argv[i],
				": invalid mask\n", NULL), 1);
	umask(mask);
	return (0);
}
