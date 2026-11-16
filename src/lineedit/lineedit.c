#include "../../includes/lineedit.h"
#include "../../includes/signals.h"

int	le_insert(t_edit *e, const char *src, size_t n)
{
	char	*bigger;
	size_t	cap;

	if (e->len + n + 1 > e->cap)
	{
		cap = e->cap * 2;
		while (e->len + n + 1 > cap)
			cap *= 2;
		bigger = malloc(cap);
		if (!bigger)
			return (0);
		ft_memcpy(bigger, e->buf, e->len + 1);
		free(e->buf);
		e->buf = bigger;
		e->cap = cap;
	}
	ft_memmove(e->buf + e->pos + n, e->buf + e->pos, e->len - e->pos + 1);
	ft_memcpy(e->buf + e->pos, src, n);
	e->len += n;
	e->pos += n;
	return (1);
}

void	le_erase(t_edit *e, size_t from, size_t to)
{
	if (from >= to)
		return ;
	ft_memmove(e->buf + from, e->buf + to, e->len - to + 1);
	e->len -= to - from;
	if (e->pos > to)
		e->pos -= to - from;
	else if (e->pos > from)
		e->pos = from;
}

static size_t	prev_pos(t_edit *e, size_t pos)
{
	if (pos == 0)
		return (0);
	pos--;
	while (pos > 0 && (e->buf[pos] & 0xC0) == 0x80)
		pos--;
	return (pos);
}

static size_t	next_pos(t_edit *e, size_t pos)
{
	if (pos >= e->len)
		return (e->len);
	pos++;
	while (pos < e->len && (e->buf[pos] & 0xC0) == 0x80)
		pos++;
	return (pos);
}

static size_t	word_left(t_edit *e)
{
	size_t	p;

	p = e->pos;
	while (p > 0 && e->buf[p - 1] == ' ')
		p--;
	while (p > 0 && e->buf[p - 1] != ' ')
		p--;
	return (p);
}

static size_t	word_right(t_edit *e)
{
	size_t	p;

	p = e->pos;
	while (p < e->len && e->buf[p] == ' ')
		p++;
	while (p < e->len && e->buf[p] != ' ')
		p++;
	return (p);
}

void	le_set_buffer(t_edit *e, const char *text)
{
	e->len = 0;
	e->pos = 0;
	e->buf[0] = '\0';
	le_insert(e, text, ft_strlen(text));
}

static void	history_move(t_edit *e, int dir)
{
	int	count;
	int	i;

	count = le_history_count();
	if (!e->fancy || count == 0)
		return ;
	if (e->hist_idx == count && dir < 0)
	{
		free(e->saved);
		free(e->prefix);
		e->saved = ft_substr(e->buf, 0, e->len);
		e->prefix = e->len ? ft_substr(e->buf, 0, e->len) : NULL;
	}
	i = e->hist_idx + dir;
	while (i >= 0 && i < count && e->prefix
		&& ft_strncmp(le_history_get(i), e->prefix,
			ft_strlen(e->prefix)) != 0)
		i += dir;
	if (i < 0 || (e->hist_idx == count && dir > 0))
		return ;
	e->hist_idx = i >= count ? count : i;
	le_set_buffer(e, i >= count ? (e->saved ? e->saved : "")
		: le_history_get(i));
}

static int	accept_ghost(t_edit *e, int whole)
{
	const char	*ghost;
	size_t		n;

	ghost = NULL;
	if (e->fancy && e->pos == e->len)
		ghost = history_suggestion(e->buf, e->len);
	if (!ghost)
		return (0);
	n = ft_strlen(ghost);
	if (!whole)
	{
		n = 0;
		while (ghost[n] == ' ')
			n++;
		while (ghost[n] && ghost[n] != ' ')
			n++;
	}
	le_insert(e, ghost, n);
	return (1);
}

static void	move_key(t_edit *e, t_key *key)
{
	if (key->type == K_LEFT)
		e->pos = prev_pos(e, e->pos);
	else if (key->type == K_RIGHT && !accept_ghost(e, 1))
		e->pos = next_pos(e, e->pos);
	else if (key->type == K_HOME)
		e->pos = 0;
	else if (key->type == K_END && !accept_ghost(e, 1))
		e->pos = e->len;
	else if (key->type == K_WORD_LEFT)
		e->pos = word_left(e);
	else if (key->type == K_WORD_RIGHT && !accept_ghost(e, 0))
		e->pos = word_right(e);
}

static void	edit_key(t_edit *e, t_key *key)
{
	if (key->type == K_BACKSPACE)
		le_erase(e, prev_pos(e, e->pos), e->pos);
	else if (key->type == K_DELETE)
		le_erase(e, e->pos, next_pos(e, e->pos));
	else if (key->type == K_KILL_EOL)
		le_erase(e, e->pos, e->len);
	else if (key->type == K_KILL_BOL)
		le_erase(e, 0, e->pos);
	else if (key->type == K_KILL_WORD)
		le_erase(e, word_left(e), e->pos);
	else if (key->type == K_DELETE_WORD)
		le_erase(e, e->pos, word_right(e));
	else if (key->type == K_CHAR)
	{
		le_insert(e, key->ch, key->n);
	}
}

static char	*finish(t_edit *e, int interrupted)
{
	e->ghost = 0;
	e->pos = e->len;
	render(e);
	if (interrupted)
	{
		write(STDOUT_FILENO, "^C", 2);
		g_signal = SIGINT;
		e->len = 0;
	}
	write(STDOUT_FILENO, "\r\n", 2);
	return (ft_substr(e->buf, 0, e->len));
}

static int	g_paste = 0;

static int	handle_key(t_edit *e, t_key *key, char **line)
{
	if (key->type == K_ENTER)
		return (*line = finish(e, 0), 1);
	if (g_paste && key->type == K_TAB)
	{
		key->type = K_CHAR;
		key->ch[0] = '\t';
		key->n = 1;
	}
	if (key->type == K_INTERRUPT)
		return (g_paste = 0, *line = finish(e, 1), 1);
	if (key->type == K_ERROR || (key->type == K_EOF && e->len == 0))
		return (*line = NULL, 1);
	if (key->type == K_SEARCH && e->fancy)
	{
		if (search_history(e) == 2)
			return (*line = finish(e, 0), 1);
		return (0);
	}
	if (key->type == K_PASTE_START || key->type == K_PASTE_END)
		g_paste = (key->type == K_PASTE_START);
	if (key->type == K_UP || key->type == K_DOWN)
		history_move(e, key->type == K_UP ? -1 : 1);
	else
	{
		e->hist_idx = le_history_count();
		if (key->type == K_EOF)
			key->type = K_DELETE;
		if (key->type == K_TAB && e->fancy)
			complete_tab(e);
		else if (key->type == K_CLEAR)
		{
			write(STDOUT_FILENO, "\033[H\033[2J", 7);
			e->cur_row = 0;
		}
		move_key(e, key);
		edit_key(e, key);
	}
	return (0);
}

static void	edit_init(t_edit *e, const char *prompt, t_shell *shell, int fancy)
{
	e->cap = 256;
	e->buf = malloc(e->cap);
	if (e->buf)
		e->buf[0] = '\0';
	e->len = 0;
	e->pos = 0;
	e->prompt = prompt;
	e->prompt_w = visible_width(prompt, &e->prompt_rows);
	e->cur_row = 0;
	e->fancy = fancy;
	e->ghost = 1;
	e->hist_idx = le_history_count();
	e->saved = NULL;
	e->prefix = NULL;
	e->shell = shell;
}

char	*le_readline(const char *prompt, t_shell *shell, int fancy)
{
	t_edit	e;
	t_key	key;
	char	*line;
	int		done;

	if (!term_raw_on())
		return (ml_reader_line(ml_stdin_reader()));
	edit_init(&e, prompt, shell, fancy);
	if (!e.buf)
		return (term_raw_off(), NULL);
	if (fancy)
		write(STDOUT_FILENO, "\033[?2004h", 8);
	highlight_reset();
	render(&e);
	done = 0;
	while (!done)
	{
		read_key(&key);
		done = handle_key(&e, &key, &line);
		if (!done && !input_pending())
			render(&e);
	}
	if (fancy)
		write(STDOUT_FILENO, "\033[?2004l", 8);
	term_raw_off();
	free(e.buf);
	free(e.saved);
	free(e.prefix);
	return (line);
}

void	le_replace(t_edit *e, size_t from, size_t to, const char *text)
{
	e->pos = to;
	le_erase(e, from, to);
	le_insert(e, text, ft_strlen(text));
}
