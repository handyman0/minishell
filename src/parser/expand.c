/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 19:02:25 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 20:33:14 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parser.h"
#include "../../includes/minishell.h"
#include "../../includes/env.h"

// Função auxiliar para obter o comprimento do nome da variavel
static int	var_name_length(char *str)
{
	int	i = 0;

	if (str[0] == '?')
		return (1);
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	return (i);
}

// Função para obter o valor da variavel
static char	*get_var_value(char *var_name, t_shell *shell)
{
	int		len;
	char	*name;
	char	*value;

	len = var_name_length(var_name);
	if (len == 0)
		return (ft_strdup("$"));
	name = ft_substr(var_name, 0, len);
	if (!name)
		return (NULL);
	if (ft_strcmp(name, "?") == 0)
		value = ft_itoa(shell->exit_status);
	else
	{
		value = env_get(shell->env, name);
		if (value)
			value = ft_strdup(value);
	}
	free(name);
	return (value ? value : ft_strdup(""));
}

// Função para calcular o tamanho necessário após expansão
static size_t	calculate_required_size(char *token, t_shell *shell)
{
	size_t	size;
	int		i;
	char	*var_value;
	int		var_len;

	size = 0;
	i = 0;
	while (token[i])
	{
		if (token[i] == '$' && (ft_isalnum(token[i+1]) || token[i+1] == '?' || token[i+1] == '_'))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				size += ft_strlen(var_value);
				free(var_value);
			}
			var_len = var_name_length(&token[i+1]);
			i += var_len + 1;
		}
		else
		{
			size++;
			i++;
		}
	}
	return (size + 1); // +1 para o null terminator
}

// Função para expandir as variaveis de ambiente
char	*expand_variables(char *token, t_shell *shell)
{
	char	*result;
	size_t	size;
	int		i;
	int		j;
	char	*var_value;

	if (!token)
		return (NULL);

	size = calculate_required_size(token, shell);
	result = malloc(size);
	if (!result)
		return (NULL);

	i = 0;
	j = 0;
	while (token[i])
	{
		if (token[i] == '$' && (ft_isalnum(token[i+1]) || token[i+1] == '?' || token[i+1] == '_'))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				// Copiar caractere por caractere para evitar overflow
				int k = 0;
				while (var_value[k] && j < (int)size - 1)
					result[j++] = var_value[k++];
				free(var_value);
				i += var_name_length(&token[i+1]) + 1;
			}
			else
			{
				i += var_name_length(&token[i+1]) + 1;
			}
		}
		else
		{
			if (j < (int)size - 1)
				result[j++] = token[i++];
			else
				break;
		}
	}
	result[j] = '\0';
	return (result);
}

// Função para remover aspas
char	*remove_quotes(char *token)
{
	char	*result;
	int		i;
	int		j;
	int		in_dquote;
	int		in_squote;

	if (!token)
		return (NULL);

	i = 0;
	j = 0;
	in_dquote = 0;
	in_squote = 0;
	result = malloc(ft_strlen(token) + 1);
	if (!result)
		return (NULL);

	while (token[i])
	{
		if (token[i] == '"' && !in_squote)
		{
			in_dquote = !in_dquote;
			i++;
		}
		else if (token[i] == '\'' && !in_dquote)
		{
			in_squote = !in_squote;
			i++;
		}
		else
		{
			result[j++] = token[i++];
		}
	}
	result[j] = '\0';

	// Otimização: se não houve remoção, retorna o original
	if (j == (int)ft_strlen(token))
	{
		free(result);
		return (ft_strdup(token));
	}

	return (result);
}

void	process_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	char	*expanded;
	char	*clean;

	current = tokens;
	while (current)
	{
		if (current->type == TOKEN_WORD && current->value)
		{
			expanded = expand_variables(current->value, shell);
			if (expanded)
			{
				clean = remove_quotes(expanded);
				free(expanded);
				free(current->value);
				current->value = clean;
			}
		}
		current = current->next;
	}
}
