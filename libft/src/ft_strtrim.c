/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:31:19 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:17:35 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	int	start;
	int	end;
	int	len;

	start = 0;
	while (s1[start] && ft_strchr((char *)set, s1[start]) != NULL)
		start++;
	end = ft_strlen(s1) - 1;
	while (end >= start && ft_strchr((char *)set, s1[end]) != NULL)
		end--;
	len = end - start + 1;
	return (ft_substr(s1, start, len));
}
