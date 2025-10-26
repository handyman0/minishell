/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 16:36:49 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:17:38 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t	real_len;
	char	*p;

	if (s == NULL)
		return (NULL);
	if (start >= ft_strlen(s))
		return (ft_strdup(""));
	real_len = ft_strlen(s) - start;
	if (len > real_len)
		len = real_len;
	p = ft_calloc(len + 1, sizeof(char));
	if (!p)
		return (NULL);
	ft_memcpy(p, s + start, len);
	return (p);
}
