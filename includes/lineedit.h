#ifndef LINEEDIT_H
# define LINEEDIT_H

# include "minishell.h"
# include <stddef.h>

# define LE_PLAIN 0
# define LE_FANCY 1
# define HISTORY_MAX 1000

typedef enum e_keytype
{
	K_NONE,
	K_CHAR,
	K_ENTER,
	K_TAB,
	K_BACKSPACE,
	K_DELETE,
	K_LEFT,
	K_RIGHT,
	K_UP,
	K_DOWN,
	K_HOME,
	K_END,
	K_WORD_LEFT,
	K_WORD_RIGHT,
	K_KILL_EOL,
	K_KILL_BOL,
	K_KILL_WORD,
	K_DELETE_WORD,
	K_CLEAR,
	K_INTERRUPT,
	K_EOF,
	K_SEARCH,
	K_SEARCH_FWD,
	K_CANCEL,
	K_PASTE_START,
	K_PASTE_END,
	K_ERROR
}	t_keytype;

typedef struct s_key
{
	t_keytype	type;
	char		ch[4];
	int			n;
}	t_key;

typedef struct s_out
{
	char	*s;
	size_t	len;
	size_t	cap;
}	t_out;

void		out_put(t_out *o, const char *src, size_t n);

typedef struct s_edit
{
	char		*buf;
	size_t		len;
	size_t		cap;
	size_t		pos;
	const char	*prompt;
	int			prompt_w;
	int			prompt_rows;
	int			cur_row;
	int			fancy;
	int			ghost;
	int			hist_idx;
	char		*saved;
	char		*prefix;
	t_shell		*shell;
}	t_edit;

char		*le_readline(const char *prompt, t_shell *shell, int fancy);

int			term_raw_on(void);
void		term_raw_off(void);
int			term_cols(void);
int			input_pending(void);
void		read_key(t_key *key);

void		render(t_edit *e);
int			text_cols(const char *s, size_t len);
int			visible_width(const char *prompt, int *rows);

void		le_history_load(const char *path);
void		le_history_save(const char *path);
void		le_history_add(const char *line);
void		le_history_clear(void);
int			le_history_count(void);
const char	*le_history_get(int index);
const char	*history_suggestion(const char *buf, size_t len);

void		ml_sort_strings(char **items, int count);
int			complete_tab(t_edit *e);
char		**opts_candidates(char **words, int nw, const char *text, t_shell *shell);
void		le_set_buffer(t_edit *e, const char *text);
int			search_history(t_edit *e);
int			le_insert(t_edit *e, const char *src, size_t n);
void		le_erase(t_edit *e, size_t from, size_t to);
void		le_replace(t_edit *e, size_t from, size_t to, const char *text);

char		*highlight_build(const char *line, size_t len, t_shell *shell);
void		highlight_reset(void);

#endif
