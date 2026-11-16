#ifndef CTL_H
# define CTL_H

# include "minishell.h"

struct s_redir;

# define OPT_E 1
# define OPT_U 2
# define OPT_X 4
# define OPT_F 8
# define OPT_C 16
# define OPT_N 32
# define OPT_V 64
# define OPT_A 128
# define OPT_M 256

struct s_func
{
	char			*name;
	t_node			*body;
	struct s_func	*next;
};
struct s_bg
{
	pid_t			pid;
	int				id;
	int				status;
	int				done;
	struct s_bg		*next;
};


struct s_local
{
	char			*key;
	char			*old;
	int				had;
	int				was_exported;
	int				depth;
	struct s_local	*next;
};

int		execute_ctl(t_node *node, t_shell *shell);
int		execute_subshell(t_node *node, t_shell *shell);
t_func	*find_func(t_shell *shell, const char *name);
int		call_function(t_shell *shell, t_func *func, char **argv);
void	funcs_free(t_shell *shell);
void	func_remove(t_shell *shell, const char *name);
char	**expand_words(char **raw, t_shell *shell);
int		apply_redirections(struct s_redir *redirs);
void	save_fds(int backup[3]);
void	restore_fds(int backup[3]);
int		glob_match(const char *pattern, const char *name);
int		builtin_break(char **argv, t_shell *shell);
int		builtin_continue(char **argv, t_shell *shell);
int		builtin_return(char **argv, t_shell *shell);
int		builtin_local(char **argv, t_shell *shell);
int		builtin_shift(char **argv, t_shell *shell);
int		builtin_set(char **argv, t_shell *shell);
int		builtin_test(char **argv, t_shell *shell);
int		builtin_read(char **argv, t_shell *shell);
int		builtin_umask(char **argv, t_shell *shell);
int		builtin_times(char **argv, t_shell *shell);
int		run_exec_builtin(t_node *node, t_shell *shell, int backup[3]);
void	exec_script_fallback(char *path, char **argv, char **env);
int		builtin_readonly(char **argv, t_shell *shell);
int		builtin_trap(char **argv, t_shell *shell);
int		builtin_wait(char **argv, t_shell *shell);
int		builtin_kill(char **argv, t_shell *shell);
int		builtin_getopts(char **argv, t_shell *shell);
char	*option_string(t_shell *shell);
int		signal_number(const char *s);
const char	*signal_name(int num);
pid_t	job_spec_pid(char *arg, t_shell *shell);
char	*quote_word(const char *word);
void	trap_run_pending(t_shell *shell);
void	trap_run_exit(t_shell *shell);
void	trap_subshell_reset(t_shell *shell);
void	traps_free(t_shell *shell);
void	bg_add(t_shell *shell, pid_t pid);
void	bg_mark(t_shell *shell, pid_t pid, int status);
void	bg_reap(t_shell *shell);
void	bgs_free(t_shell *shell);
int		builtin_command(char **argv, t_shell *shell);
void	push_local(t_shell *shell, const char *key);
int		builtin_true(char **argv, t_shell *shell);
int		builtin_false(char **argv, t_shell *shell);
int		builtin_eval(char **argv, t_shell *shell);

#endif
