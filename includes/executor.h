/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:02:41 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:08:41 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include <unistd.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/wait.h>
# include "../mylib/include/mylib.h"
# include "minishell.h"

/**
 * @brief Libera um array de strings terminado em NULL.
 * @param arr Array alocado.
 */
void	free_str_array(char **arr);

/**
 * @brief Le o corpo de um heredoc para um arquivo temporario anonimo.
 * @param raw Limitador como digitado, com aspas se houver.
 */
int	heredoc_read(const char *raw, t_shell *shell);
int	prepare_heredocs(t_node *node, t_shell *shell);

/**
 * @brief Procura o caminho executável para um comando interno.
 * @param cmd Nome do comando.
 * @param shell Estado global do shell.
 * @return Caminho completo ou NULL se não encontrado.
 */
char	*find_path(char *cmd, t_shell *shell);

/**
 * @brief Procura um executavel valido, no PATH ou por caminho direto.
 * @return Caminho alocado ou NULL se nao houver executavel.
 */
char	*find_executable(const char *name, t_shell *shell);

typedef struct s_saved	t_saved;


char	*run_substitution(t_shell *shell, char *cmd);
t_node	*clone_command(t_node *node);
int		assign_shell(t_shell *shell, char **assigns);
int		assigns_ok(t_shell *shell, char **assigns);
t_saved	*overlay_apply(t_shell *shell, char **assigns, int *count);
void	overlay_restore(t_shell *shell, t_saved *saved, int count);

/**
 * @brief Converte a lista de ambiente em array de strings para execve.
 * @param env Lista encadeada de variáveis de ambiente.
 * @return Array terminado em NULL ou NULL em erro.
 */
char	**env_to_array(t_env *env);

/**
 * @brief Executa a árvore de comandos construída pelo parser.
 * @param node Nó AST raiz.
 * @param shell Estado global do shell.
 * @return Código de saída do comando.
 */
int		execute_tree(t_node *node, t_shell *shell);

void	expand_command(t_node *node, t_shell *shell);

#endif
