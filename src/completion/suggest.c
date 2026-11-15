#include "../../includes/completion.h"
#include <readline/readline.h>
#include <readline/history.h>

static rl_voidfunc_t	*g_default_display = NULL;
static int				g_shown = 0;
static t_shell			*g_suggest_shell = NULL;

static int	is_main_prompt(void)
{
	return (rl_display_prompt
		&& ft_strncmp(rl_display_prompt, "minishell:", 10) == 0);
}

static char	*find_suggestion(void)
{
	HIST_ENTRY	**list;
	int			i;

	list = history_list();
	if (!list || rl_end == 0)
		return (NULL);
	i = history_length - 1;
	while (i >= 0)
	{
		if (ft_strlen(list[i]->line) > (size_t)rl_end
			&& ft_strncmp(list[i]->line, rl_line_buffer, rl_end) == 0)
			return (list[i]->line + rl_end);
		i--;
	}
	return (NULL);
}

static int	fit_length(char *suggestion)
{
	int	rows;
	int	cols;
	int	room;
	int	len;

	rl_get_screen_size(&rows, &cols);
	room = cols - (int)ft_strlen(rl_display_prompt) - rl_end - 1;
	len = (int)ft_strlen(suggestion);
	if (len > room)
		len = room;
	while (len > 0 && (suggestion[len] & 0xC0) == 0x80)
		len--;
	return (len);
}

static int	columns(char *str, int len)
{
	int	cols;
	int	i;

	cols = 0;
	i = 0;
	while (i < len)
	{
		if ((str[i] & 0xC0) != 0x80)
			cols++;
		i++;
	}
	return (cols);
}

static void	draw_ghost(char *suggestion)
{
	int		len;
	char	*back;
	char	*num;

	len = fit_length(suggestion);
	if (len <= 0)
		return ;
	write(STDOUT_FILENO, "\033[K\033[90m", 8);
	write(STDOUT_FILENO, suggestion, len);
	write(STDOUT_FILENO, "\033[0m\033[", 6);
	num = ft_itoa(columns(suggestion, len));
	back = ft_strjoin(num, "D");
	if (back)
		write(STDOUT_FILENO, back, ft_strlen(back));
	free(num);
	free(back);
	g_shown = 1;
}

static void	move_cursor(char dir, int cols)
{
	char	*num;
	char	*seq;

	if (cols <= 0)
		return ;
	num = ft_itoa(cols);
	seq = ft_strjoin("\033[", num);
	if (seq)
	{
		write(STDOUT_FILENO, seq, ft_strlen(seq));
		write(STDOUT_FILENO, &dir, 1);
	}
	free(num);
	free(seq);
}

static void	paint_line(void)
{
	char	*colored;
	int		rows;
	int		cols;

	rl_get_screen_size(&rows, &cols);
	if (rl_end == 0 || (int)ft_strlen(rl_display_prompt) + rl_end + 2 > cols)
		return ;
	colored = highlight_build(rl_line_buffer, rl_end, g_suggest_shell);
	if (!colored)
		return ;
	move_cursor('D', columns(rl_line_buffer, rl_point));
	write(STDOUT_FILENO, colored, ft_strlen(colored));
	move_cursor('D', columns(rl_line_buffer + rl_point, rl_end - rl_point));
	free(colored);
}

static void	clear_ghost(void)
{
	char	*num;
	char	*seq;

	g_shown = 0;
	num = ft_itoa(columns(rl_line_buffer + rl_point, rl_end - rl_point));
	if (rl_point < rl_end && num)
	{
		seq = ft_strjoin("\033[", num);
		write(STDOUT_FILENO, seq, ft_strlen(seq));
		write(STDOUT_FILENO, "C", 1);
		free(seq);
	}
	write(STDOUT_FILENO, "\033[K", 3);
	if (rl_point < rl_end && num)
	{
		seq = ft_strjoin("\033[", num);
		write(STDOUT_FILENO, seq, ft_strlen(seq));
		write(STDOUT_FILENO, "D", 1);
		free(seq);
	}
	free(num);
}

static void	display_hook(void)
{
	char	*suggestion;

	suggestion = NULL;
	if (is_main_prompt() && !rl_done && rl_point == rl_end)
		suggestion = find_suggestion();
	g_default_display();
	if (is_main_prompt() && !rl_done)
		paint_line();
	if (suggestion)
		draw_ghost(suggestion);
	else if (g_shown)
		clear_ghost();
}

static int	accept_or_forward(int count, int key)
{
	char	*suggestion;

	suggestion = NULL;
	if (rl_point == rl_end && is_main_prompt())
		suggestion = find_suggestion();
	if (suggestion)
		return (rl_insert_text(suggestion), 0);
	return (rl_forward_char(count, key));
}

static int	accept_or_end(int count, int key)
{
	char	*suggestion;

	suggestion = NULL;
	if (rl_point == rl_end && is_main_prompt())
		suggestion = find_suggestion();
	if (suggestion)
		return (rl_insert_text(suggestion), 0);
	return (rl_end_of_line(count, key));
}

static int	newline_clean(int count, int key)
{
	if (g_shown)
		write(STDOUT_FILENO, "\033[K", 3);
	g_shown = 0;
	return (rl_newline(count, key));
}

void	setup_suggestions(t_shell *shell)
{
	g_suggest_shell = shell;
	g_default_display = rl_redisplay_function;
	rl_redisplay_function = display_hook;
	rl_bind_keyseq("\\e[C", accept_or_forward);
	rl_bind_keyseq("\\eOC", accept_or_forward);
	rl_bind_keyseq("\\C-f", accept_or_forward);
	rl_bind_keyseq("\\e[F", accept_or_end);
	rl_bind_keyseq("\\eOF", accept_or_end);
	rl_bind_keyseq("\\C-e", accept_or_end);
	rl_bind_keyseq("\\C-m", newline_clean);
	rl_bind_keyseq("\\C-j", newline_clean);
}
