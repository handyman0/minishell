#ifndef OUTPUT_H
# define OUTPUT_H


/**
 * @brief Escreve "minishell: a: b: <erro de errno>" em stderr.
 * @param a Contexto opcional (comando).
 * @param b Contexto opcional (argumento).
 */
void	put_errno(const char *a, const char *b);

#endif
