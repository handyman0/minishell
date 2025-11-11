/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 19:02:25 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/07 21:26:38 by lmelo-do         ###   ########.fr       */
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

static char *process_token_with_quotes(char *token, t_shell *shell)
{
	char	*result;
	int		i;
	int		j;
	int		in_squote;
	int		in_dquote;
	char	*var_value;

	if (!token)
		return (NULL);
	result = malloc(ft_strlen(token) * 2 + 1);
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	in_squote = 0;
	in_dquote = 0;
	while (token[i] && j < (int)(ft_strlen(token) * 2))
	{
		// Entrando ou saindo de aspas simples
		if (token[i] == '\'' && !in_dquote)
		{
			in_squote = !in_squote;
			i++;
		}
		// Entrando ou saindo de aspas duplas
		else if (token[i] == '"' && !in_squote)
		{
			in_dquote = !in_dquote;
			i++;
		}
		// Entrando de variaveis (não ocorre dentro de aspas simples)
		else if (token[i] == '$' && !in_squote && (ft_isalnum(token[i+1]) || token[i+1] == '?' || token[i+1] == '_'))
		{
			var_value = get_var_value(&token[i+1], shell);
			if (var_value)
			{
				int k = 0;
				while (var_value[k] && j < (int)(ft_strlen(token) * 2) - 1)
					result[j++] = var_value[k++];
				free(var_value);
				i += var_name_length(&token[i+1]) + 1;
			}
			else
				i += var_name_length(&token[i+1]) + 1;
		}
		// Caracteres normais
		else
		{
			if (j < (int)(ft_strlen(token) * 2) - 1)
				result[j++] = token[i++];
			else
				break ;
		}
	}
	result[j] = '\0';
	return (result);
}

// Função para expandir as variaveis de ambiente
char	*expand_variables(char *token, t_shell *shell)
{
	return (process_token_with_quotes(token, shell));
}

// Função para remover aspas
char	*remove_quotes(char *token)
{
	return (ft_strdup(token));
}

void	process_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	char	*processed;

	current = tokens;
	while (current)
	{
		if (current->type == TOKEN_WORD && current->value)
		{
			processed = process_token_with_quotes(current->value, shell);
			if (processed)
			{
				free(current->value);
				current->value = processed;
			}
		}
		current = current->next;
	}
}
