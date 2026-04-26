/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 16:30:09 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:08:41 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTINS_H
#define BUILTINS_H

# include "../libft/include/libft.h"
# include "minishell.h"

/**
 * @brief Executa o builtin pwd imprimindo o diretório atual.
 * @return 0 em sucesso ou 1 em erro.
 */
int builtin_pwd(void);

/**
 * @brief Executa o builtin echo.
 * @param argv Argumentos do comando.
 * @return Sempre 0.
 */
int	builtin_echo(char **argv);

/**
 * @brief Executa o builtin cd.
 * @param argv Argumentos do comando.
 * @param shell Estado global do shell.
 * @return Código de saída do builtin.
 */
int	builtin_cd(char **argv, t_shell *shell);

/**
 * @brief Executa o builtin export.
 * @param argv Argumentos do comando.
 * @param shell Estado global do shell.
 * @return Código de saída do builtin.
 */
int	builtin_export(char **argv, t_shell *shell);

/**
 * @brief Executa o builtin unset.
 * @param argv Argumentos do comando.
 * @param shell Estado global do shell.
 * @return Código de saída do builtin.
 */
int	builtin_unset(char **argv, t_shell *shell);

/**
 * @brief Executa o builtin env imprimindo o ambiente.
 * @param env Lista de variáveis de ambiente.
 * @return Sempre 0.
 */
int	builtin_env(t_env *env);

/**
 * @brief Executa o builtin exit.
 * @param argv Argumentos do comando.
 * @param shell Estado global do shell.
 * @return Código de saída do processo shell.
 */
int	builtin_exit(char **argv, t_shell *shell);

#endif
