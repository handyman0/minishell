/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 16:07:54 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:17:27 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

char	*ft_strnstr(const char *big, const char *little, size_t len)
{
	size_t	i;
	size_t	l_len;

	l_len = ft_strlen(little);
	if (!l_len)
		return ((char *)big);
	i = 0;
	while (big[i] && i + l_len <= len)
	{
		if (ft_strncmp(big + i, little, l_len) == 0)
			return ((char *)big + i);
		i++;
	}
	return (NULL);
}
