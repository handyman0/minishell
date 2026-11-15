#ifndef COMPLETION_H
# define COMPLETION_H

# include "minishell.h"

void	setup_completion(t_shell *shell);
void	setup_suggestions(t_shell *shell);
char	*highlight_build(const char *line, size_t len, t_shell *shell);

#endif
