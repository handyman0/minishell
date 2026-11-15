#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long	g_state = 1;

static unsigned int	rnd(unsigned int mod)
{
	g_state = g_state * 6364136223846793005UL + 1442695040888963407UL;
	return ((unsigned int)(g_state >> 33) % mod);
}

static const char	*g_words[] = {"echo", "cat", "ls", "true", "false", "pwd",
	"cd", "export A=1", "export B=\"x y\"", "unset A", "env", "type", "which",
	"alias z=\"echo Z\"", "unalias z", "jobs", "fg", "bg", "head -c 5",
	"wc -c", "tr a b", "$A", "${A}", "$B", "\"$B\"", "'$B'", "$?", "${?}",
	"$$", "~", "~/x", "*", "*.c", "?", "[a-m]*", "\"\"", "''", "a b", "-n",
	"--", "/", "/nonexistent", "/bin/echo", "./tests", "x\"y\"z", "'", "\"",
	"$", "${", "${A", "$(", "`", "\\", "#c", "{", "}", "=", "!", "%", NULL};

static const char	*g_ops[] = {"|", "||", "&&", ";", "&", "(", ")", "<", ">",
	">>", "<<", "2>", "2>>", ">&2", "2>&1", "<&", "|&", ";;", "&&&", "|||",
	"<>", "> >", NULL};

static int	count(const char **list)
{
	int	n;

	n = 0;
	while (list[n])
		n++;
	return (n);
}

static void	gen_line(void)
{
	int	tokens;
	int	i;

	tokens = 1 + rnd(12);
	i = 0;
	while (i++ < tokens)
	{
		if (rnd(4) == 0)
			fputs(g_ops[rnd(count(g_ops))], stdout);
		else
			fputs(g_words[rnd(count(g_words))], stdout);
		fputc(rnd(8) ? ' ' : '\t', stdout);
	}
	fputc('\n', stdout);
}

static void	gen_noise(void)
{
	int	len;
	int	c;

	len = 1 + rnd(60);
	while (len--)
	{
		c = 1 + rnd(255);
		if (c == '\n' || c == '\r')
			c = ' ';
		fputc(c, stdout);
	}
	fputc('\n', stdout);
}

static const char	*g_keys[] = {"\033[A", "\033[B", "\033[C", "\033[D",
	"\033[H", "\033[F", "\033[3~", "\033[1;5C", "\033[1;5D", "\033OA",
	"\033OH", "\033b", "\033f", "\033d", "\033\177", "\033", "\033[",
	"\033[99;99;", "\033[200~", "\033[201~", "\033[200~a b\nc\r\033[201~",
	"\t", "\t\t", "\177", "\177\177", "\001", "\002", "\005", "\006",
	"\013", "\025", "\027", "\f", "\003", "\032", "\r", "\r",
	"\303\251", "\346\227\245\346\234\254", "\360\237\230\200", "\377",
	"\200", "\303", "\342\202", " ", "  ", "echo ", "ls ", "cd ", "$HOME",
	"${", "$", "\"", "'", "|", "&&", ";", "(", ")", "<", ">", "*", "~/",
	"/", "./", "sub/", "ec", "a", "z", "0", NULL};

static void	gen_keys(void)
{
	fputs(g_keys[rnd(count(g_keys))], stdout);
	if (rnd(6) == 0)
		fputc(32 + rnd(95), stdout);
}

static void	mutate(char *line)
{
	size_t	len;
	size_t	pos;
	char	ch;
	int		kind;

	len = strlen(line);
	kind = rnd(4);
	if (len == 0)
		return ;
	pos = rnd(len);
	ch = "|&;()<>$\"'*~{} \\#"[rnd(17)];
	if (kind == 0)
		line[pos] = ch;
	else if (kind == 1)
		memmove(line + pos, line + pos + 1, len - pos);
	else if (kind == 2 && len < 500)
	{
		memmove(line + pos + 1, line + pos, len - pos + 1);
		line[pos] = ch;
	}
	else if (len > 2)
		line[pos] = line[rnd(len)];
}

static void	gen_mutated(FILE *cases)
{
	static char	lines[512][512];
	char		buf[1024];
	int			total;
	int			i;
	int			rounds;

	total = 0;
	while (total < 512 && fgets(buf, sizeof(buf), cases))
	{
		buf[strcspn(buf, "\n")] = '\0';
		strncpy(lines[total], buf, 511);
		total++;
	}
	if (total == 0)
		return ;
	strcpy(buf, lines[rnd(total)]);
	rounds = 1 + rnd(4);
	i = 0;
	while (i++ < rounds)
		mutate(buf);
	puts(buf);
}

int	main(int argc, char **argv)
{
	long	n;
	FILE	*cases;
	int		mode;

	if (argc < 4)
	{
		fprintf(stderr, "uso: fuzz gen|noise|mutate|keys seed n [casos]\n");
		return (2);
	}
	g_state = strtoul(argv[2], NULL, 10) * 2654435761UL + 12345;
	n = atol(argv[3]);
	mode = argv[1][0];
	cases = (mode == 'm' && argc > 4) ? fopen(argv[4], "r") : NULL;
	while (n-- > 0)
	{
		if (mode == 'g')
			gen_line();
		else if (mode == 'k')
			gen_keys();
		else if (mode == 'n')
			gen_noise();
		else if (cases)
		{
			rewind(cases);
			gen_mutated(cases);
		}
	}
	if (cases)
		fclose(cases);
	return (0);
}
