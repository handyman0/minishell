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
# include <stdio.h>
# include "../libft/include/libft.h"
# include "minishell.h"

/**
 * @brief Exibe uma mensagem de erro e encerra o programa.
 * @param msg Mensagem a ser exibida.
 */
void	error_exit(const char *msg);

/**
 * @brief Libera um array de strings terminado em NULL.
 * @param arr Array alocado.
 */
void	free_str_array(char **arr);

/**
 * @brief Executa um heredoc e redireciona stdin para o pipe interno.
 * @param limiter Texto que encerra o heredoc.
 */
void	exec_heredoc(const char *limiter);

/**
 * @brief Abre um arquivo com modo de leitura ou escrita.
 * @param path Caminho do arquivo.
 * @param mode 0 = append, 1 = truncate, outro = read.
 * @return File descriptor ou -1 em erro.
 */
int	open_file(char *path, int mode);

/**
 * @brief Redireciona stdin para um arquivo de entrada.
 * @param file Caminho do arquivo.
 * @return 1 em sucesso, 0 em erro.
 */
int	handle_redirection_in(const char *file);

/**
 * @brief Redireciona stdout para um arquivo de saída.
 * @param file Caminho do arquivo.
 * @param append Se 1, faz append; caso contrário, trunca o arquivo.
 * @return 1 em sucesso, 0 em erro.
 */
int	handle_redirection_out(const char *file, int append);

/**
 * @brief Procura o caminho executável para um comando interno.
 * @param cmd Nome do comando.
 * @param shell Estado global do shell.
 * @return Caminho completo ou NULL se não encontrado.
 */
char	*find_path(char *cmd, t_shell *shell);

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

#endif
