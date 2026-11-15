#include "../../includes/lineedit.h"

#define SEARCH_MAX 255

typedef struct s_search
{
	char	query[SEARCH_MAX + 1];
	size_t	len;
	int		cur;
	int		failed;
}	t_search;

static int	find_entry(const char *query, int from, int dir, const char *skip)
{
	const char	*entry;
	int			count;

	count = le_history_count();
	while (from >= 0 && from < count)
	{
		entry = le_history_get(from);
		if (ft_strnstr(entry, query, ft_strlen(entry))
			&& (!skip || ft_strcmp(entry, skip) != 0))
			return (from);
		from += dir;
	}
	return (-1);
}

static void	show_match(t_edit *e, t_search *s)
{
	const char	*entry;
	const char	*hit;

	entry = le_history_get(s->cur);
	if (!entry)
		return ;
	le_set_buffer(e, entry);
	hit = ft_strnstr(entry, s->query, ft_strlen(entry));
	e->pos = hit ? (size_t)(hit - entry) : e->len;
}

static void	search_step(t_edit *e, t_search *s, int from, int dir)
{
	const char	*skip;
	int			found;

	skip = NULL;
	if (s->cur >= 0 && s->cur < le_history_count()
		&& (from == s->cur - 1 || from == s->cur + 1))
		skip = le_history_get(s->cur);
	found = find_entry(s->query, from, dir, skip);
	s->failed = (found == -1);
	if (found != -1)
	{
		s->cur = found;
		show_match(e, s);
	}
}

static void	set_search_prompt(t_edit *e, t_search *s, char **prompt)
{
	char	*head;
	char	*body;

	free(*prompt);
	head = ft_strjoin(s->failed ? "(failed reverse-i-search)`"
			: "(reverse-i-search)`", s->query);
	body = ft_strjoin(head, "': ");
	free(head);
	*prompt = body;
	e->prompt = body;
	e->prompt_w = visible_width(body, &e->prompt_rows);
	e->prompt_rows = 0;
}

static void	erase_query(t_search *s)
{
	if (s->len == 0)
		return ;
	s->len--;
	while (s->len > 0 && (s->query[s->len] & 0xC0) == 0x80)
		s->len--;
	s->query[s->len] = '\0';
}

static int	search_key(t_edit *e, t_search *s, t_key *key)
{
	if (key->type == K_CHAR && s->len + key->n <= SEARCH_MAX)
	{
		ft_memcpy(s->query + s->len, key->ch, key->n);
		s->len += key->n;
		s->query[s->len] = '\0';
		search_step(e, s, s->cur >= le_history_count()
			? le_history_count() - 1 : s->cur, -1);
	}
	else if (key->type == K_BACKSPACE)
	{
		erase_query(s);
		s->cur = le_history_count();
		if (s->len > 0)
			search_step(e, s, le_history_count() - 1, -1);
	}
	else if (key->type == K_SEARCH && s->len > 0)
		search_step(e, s, s->cur - 1, -1);
	else if (key->type == K_SEARCH_FWD && s->len > 0)
		search_step(e, s, s->cur + 1, 1);
	else if (key->type == K_ENTER)
		return (2);
	else if (key->type == K_INTERRUPT || key->type == K_CANCEL
		|| key->type == K_EOF || key->type == K_ERROR)
		return (-1);
	else if (key->type != K_SEARCH && key->type != K_SEARCH_FWD
		&& key->type != K_CHAR)
		return (1);
	return (0);
}

static void	search_init(t_search *s)
{
	s->query[0] = '\0';
	s->len = 0;
	s->cur = le_history_count();
	s->failed = 0;
}

int	search_history(t_edit *e)
{
	t_search	s;
	t_key		key;
	char		*saved;
	char		*prompt;
	const char	*orig_prompt;
	int			result;
	size_t		saved_pos;

	saved = ft_substr(e->buf, 0, e->len);
	saved_pos = e->pos;
	orig_prompt = e->prompt;
	prompt = NULL;
	search_init(&s);
	e->ghost = 0;
	result = 0;
	while (result == 0)
	{
		set_search_prompt(e, &s, &prompt);
		render(e);
		read_key(&key);
		result = search_key(e, &s, &key);
	}
	if (result == -1 && saved)
		le_set_buffer(e, saved);
	if (result == -1)
		e->pos = saved_pos;
	else
		e->pos = e->len;
	e->prompt = orig_prompt;
	e->prompt_w = visible_width(orig_prompt, &e->prompt_rows);
	e->ghost = 1;
	e->hist_idx = le_history_count();
	return (free(saved), free(prompt), result);
}
