#include "../../includes/lineedit.h"
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>

static struct termios	g_saved;
static int				g_raw = 0;

int	term_raw_on(void)
{
	struct termios	raw;

	if (tcgetattr(STDIN_FILENO, &g_saved) == -1)
		return (0);
	raw = g_saved;
	raw.c_iflag &= ~(IXON | ICRNL | INLCR | IGNCR | ISTRIP | BRKINT);
	raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &raw) == -1)
		return (0);
	g_raw = 1;
	return (1);
}

void	term_raw_off(void)
{
	if (g_raw)
		tcsetattr(STDIN_FILENO, TCSADRAIN, &g_saved);
	g_raw = 0;
}

int	term_cols(void)
{
	struct winsize	ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return (ws.ws_col);
	return (80);
}

int	input_pending(void)
{
	struct pollfd	p;

	p.fd = STDIN_FILENO;
	p.events = POLLIN;
	p.revents = 0;
	return (poll(&p, 1, 0) > 0 && (p.revents & POLLIN));
}

static int	read_byte(int timeout_ms)
{
	struct pollfd	p;
	unsigned char	c;
	ssize_t			n;

	p.fd = STDIN_FILENO;
	p.events = POLLIN;
	if (timeout_ms >= 0 && poll(&p, 1, timeout_ms) <= 0)
		return (-2);
	n = read(STDIN_FILENO, &c, 1);
	while (n < 0 && errno == EINTR)
		n = read(STDIN_FILENO, &c, 1);
	if (n <= 0)
		return (-1);
	return (c);
}

static t_keytype	tilde_key(int param)
{
	if (param == 1 || param == 7)
		return (K_HOME);
	if (param == 4 || param == 8)
		return (K_END);
	if (param == 3)
		return (K_DELETE);
	if (param == 200)
		return (K_PASTE_START);
	if (param == 201)
		return (K_PASTE_END);
	return (K_NONE);
}

static void	csi_key(t_key *key)
{
	int	param;
	int	modified;
	int	c;

	param = 0;
	modified = 0;
	c = read_byte(50);
	while (c == ';' || (c >= '0' && c <= '9'))
	{
		if (c == ';')
			modified = 1;
		else if (!modified)
			param = param * 10 + (c - '0');
		c = read_byte(50);
	}
	key->type = K_NONE;
	if (c == 'A' || c == 'B' || c == 'H' || c == 'F')
		key->type = (c == 'A') * K_UP + (c == 'B') * K_DOWN
			+ (c == 'H') * K_HOME + (c == 'F') * K_END;
	else if (c == 'C')
		key->type = modified ? K_WORD_RIGHT : K_RIGHT;
	else if (c == 'D')
		key->type = modified ? K_WORD_LEFT : K_LEFT;
	else if (c == '~')
		key->type = tilde_key(param);
}

static void	escape_key(t_key *key)
{
	int	c;

	c = read_byte(50);
	key->type = K_NONE;
	if (c == '[')
		return (csi_key(key));
	if (c == 'O')
	{
		c = read_byte(50);
		key->type = (c == 'A') * K_UP + (c == 'B') * K_DOWN
			+ (c == 'C') * K_RIGHT + (c == 'D') * K_LEFT
			+ (c == 'H') * K_HOME + (c == 'F') * K_END;
	}
	else if (c == 'b')
		key->type = K_WORD_LEFT;
	else if (c == 'f')
		key->type = K_WORD_RIGHT;
	else if (c == 'd')
		key->type = K_DELETE_WORD;
	else if (c == 127 || c == 8)
		key->type = K_KILL_WORD;
}

static void	utf8_key(t_key *key, int lead)
{
	int	extra;
	int	c;

	extra = 0;
	if (lead >= 0xF0)
		extra = 3;
	else if (lead >= 0xE0)
		extra = 2;
	else if (lead >= 0xC0)
		extra = 1;
	key->type = K_CHAR;
	key->ch[0] = lead;
	key->n = 1;
	while (extra-- > 0)
	{
		c = read_byte(50);
		if (c < 0)
			break ;
		key->ch[key->n++] = c;
	}
}

void	read_key(t_key *key)
{
	int	c;

	key->n = 0;
	key->type = K_NONE;
	c = read_byte(-1);
	if (c < 0)
		key->type = K_ERROR;
	else if (c == 27)
		escape_key(key);
	else if (c == '\r' || c == '\n')
		key->type = K_ENTER;
	else if (c == '\t')
		key->type = K_TAB;
	else if (c == 127 || c == 8)
		key->type = K_BACKSPACE;
	else if (c == 1)
		key->type = K_HOME;
	else if (c == 5)
		key->type = K_END;
	else if (c == 2)
		key->type = K_LEFT;
	else if (c == 6)
		key->type = K_RIGHT;
	else if (c == 16)
		key->type = K_UP;
	else if (c == 14)
		key->type = K_DOWN;
	else if (c == 11)
		key->type = K_KILL_EOL;
	else if (c == 21)
		key->type = K_KILL_BOL;
	else if (c == 23)
		key->type = K_KILL_WORD;
	else if (c == 12)
		key->type = K_CLEAR;
	else if (c == 3)
		key->type = K_INTERRUPT;
	else if (c == 4)
		key->type = K_EOF;
	else if (c >= 32)
		utf8_key(key, c);
}
