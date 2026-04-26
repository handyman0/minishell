/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:01:35 by lmelo-do          #+#    #+#             */
/*   Updated: 2026/04/25 21:29:06 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENV_H
# define ENV_H

# include "../libft/include/libft.h"
# include "../libft/include/ft_printf.h"
# include "utils.h"
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>

/**
 * @struct s_env
 * @brief Nó da lista encadeada de variáveis de ambiente.
 */
typedef struct s_env
{
	char			*key; /**< Nome da variável. */
	char			*value; /**< Valor da variável. */
	struct s_env	*next; /**< Próximo nó na lista. */
}	t_env;

/* ======================== */
/* Inicialização de limpeza */
/* ======================== */
/**
 * @brief Inicializa a lista de ambiente a partir de envp.
 * @param envp Array de strings no formato KEY=VALUE.
 * @return Lista encadeada de variáveis de ambiente.
 */
t_env	*env_init(char **envp);

/**
 * @brief Libera toda a lista de ambiente.
 * @param env Lista de variáveis de ambiente.
 */
void	env_free(t_env *env);

/* =================== */
/* Busca e modificação */
/* =================== */

/**
 * @brief Busca o valor de uma variável de ambiente.
 * @param env Lista de ambiente.
 * @param key Nome da variável.
 * @return Valor associado ou NULL se não existir.
 */
char	*env_get(t_env *env, const char *key);

/**
 * @brief Define ou atualiza uma variável de ambiente.
 * @param env Ponteiro para a lista de ambiente.
 * @param key Nome da variável.
 * @param value Valor da variável.
 */
void	env_set(t_env **env, const char *key, const char *value);

/**
 * @brief Remove uma variável do ambiente.
 * @param env Ponteiro para a lista de ambiente.
 * @param key Nome da variável.
 */
void	env_unset(t_env **env, const char *key);

/* =================== */
/* Utilitarios e debug */
/* =================== */

/**
 * @brief Imprime o ambiente no formato KEY=VALUE.
 * @param env Lista de variáveis de ambiente.
 */
void	env_print(t_env *env);

/**
 * @brief Retorna a quantidade de variáveis no ambiente.
 * @param env Lista de variáveis.
 * @return Número de variáveis.
 */
int		env_size(t_env *env);

/**
 * @brief Converte o ambiente para um array de strings usadas pelo execve.
 * @param env Lista de variáveis.
 * @return Array terminado em NULL ou NULL em erro.
 */
char	**env_to_array(t_env *env);

/* ================== */
/* Funções auxiliares */
/* ================== */

/**
 * @brief Cria um novo nó de variável de ambiente.
 * @param key Nome da variável.
 * @param value Valor da variável.
 * @return Nó alocado ou NULL em caso de erro.
 */
t_env	*env_new(char *key, char *value);

/**
 * @brief Anexa um nó ao final da lista de ambiente.
 * @param env Ponteiro para a lista de ambiente.
 * @param new Novo nó de variável.
 */
void	env_add_back(t_env **env, t_env *new);

/**
 * @brief Cria um ambiente mínimo quando envp não está disponível.
 * @return Lista de ambiente inicializada.
 */
t_env	*env_init_minimal(void);

#endif
