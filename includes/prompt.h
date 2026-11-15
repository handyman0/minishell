#ifndef PROMPT_H
# define PROMPT_H

# include "minishell.h"

# define DEFAULT_PS1 "minishell:\\w> "

/**
 * @brief Monta o prompt a partir da variavel PS1 (ou do padrao).
 * @param shell Estado global do shell.
 * @return String alocada, nunca NULL em condicoes normais.
 */
char	*build_prompt(t_shell *shell);

#endif
