#include "../../includes/lineedit.h"


void	out_put(t_out *o, const char *src, size_t n)
{
	char	*bigger;
	size_t	cap;

	if (o->len + n + 1 > o->cap)
	{
		cap = o->cap ? o->cap : 256;
		while (o->len + n + 1 > cap)
			cap *= 2;
		bigger = malloc(cap);
		if (!bigger)
			return ;
		ft_memcpy(bigger, o->s, o->len);
		free(o->s);
		o->s = bigger;
		o->cap = cap;
	}
	ft_memcpy(o->s + o->len, src, n);
	o->len += n;
}

static void	out_csi(t_out *o, int n, char final)
{
	char	*num;

	if (n <= 0)
		return ;
	num = ft_itoa(n);
	out_put(o, "\033[", 2);
	out_put(o, num, ft_strlen(num));
	out_put(o, &final, 1);
	free(num);
}

int	text_cols(const char *s, size_t len)
{
	int		n;
	size_t	i;

	n = 0;
	i = 0;
	while (i < len)
	{
		if ((s[i] & 0xC0) != 0x80)
			n++;
		i++;
	}
	return (n);
}

int	visible_width(const char *prompt, int *rows)
{
	int	w;
	int	i;

	w = 0;
	*rows = 0;
	i = 0;
	while (prompt[i])
	{
		if (prompt[i] == '\033' && prompt[i + 1] == '[')
		{
			i += 2;
			while (prompt[i] && (prompt[i] < 0x40 || prompt[i] > 0x7e))
				i++;
		}
		else if (prompt[i] == '\n')
		{
			w = 0;
			(*rows)++;
		}
		else if (prompt[i] != 1 && prompt[i] != 2
			&& (prompt[i] & 0xC0) != 0x80)
			w++;
		if (prompt[i])
			i++;
	}
	return (w);
}

static void	out_prompt(t_out *o, const char *prompt)
{
	size_t	i;

	i = 0;
	while (prompt[i])
	{
		if (prompt[i] == '\n')
			out_put(o, "\r\n", 2);
		else if (prompt[i] != 1 && prompt[i] != 2)
			out_put(o, prompt + i, 1);
		i++;
	}
}

static int	out_ghost(t_out *o, t_edit *e, int cols, int text_w)
{
	const char	*ghost;
	int			room;
	size_t		bytes;
	int			shown;

	ghost = NULL;
	if (e->fancy && e->ghost && e->pos == e->len)
		ghost = history_suggestion(e->buf, e->len);
	room = cols - ((e->prompt_w + text_w) % cols) - 1;
	if (!ghost || room <= 0)
		return (0);
	bytes = 0;
	shown = 0;
	while (ghost[bytes] && (shown < room || (ghost[bytes] & 0xC0) == 0x80))
	{
		shown += (ghost[bytes] & 0xC0) != 0x80;
		bytes++;
	}
	out_put(o, "\033[90m", 5);
	out_put(o, ghost, bytes);
	out_put(o, "\033[0m", 4);
	return (text_cols(ghost, bytes));
}

static void	out_cursor(t_out *o, t_edit *e, int cols, int total)
{
	int	before;
	int	end_row;

	end_row = total / cols;
	if (total > 0 && total % cols == 0)
		out_put(o, " \r", 2);
	before = e->prompt_w + text_cols(e->buf, e->pos);
	out_csi(o, end_row - before / cols, 'A');
	out_put(o, "\r", 1);
	out_csi(o, before % cols, 'C');
	e->cur_row = e->prompt_rows + before / cols;
}

void	render(t_edit *e)
{
	t_out	o;
	char	*colored;
	int		cols;
	int		text_w;
	int		ghost_w;

	o.s = NULL;
	o.len = 0;
	o.cap = 0;
	cols = term_cols();
	text_w = text_cols(e->buf, e->len);
	out_csi(&o, e->cur_row, 'A');
	out_put(&o, "\r\033[J", 4);
	out_prompt(&o, e->prompt);
	colored = NULL;
	if (e->fancy && e->len > 0)
		colored = highlight_build(e->buf, e->len, e->shell);
	if (colored)
		out_put(&o, colored, ft_strlen(colored));
	else
		out_put(&o, e->buf, e->len);
	free(colored);
	ghost_w = out_ghost(&o, e, cols, text_w);
	out_cursor(&o, e, cols, e->prompt_w + text_w + ghost_w);
	if (o.s)
		write(STDOUT_FILENO, o.s, o.len);
	free(o.s);
}
