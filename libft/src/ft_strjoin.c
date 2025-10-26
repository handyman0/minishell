/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 17:30:10 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:17:08 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*newstring;
	size_t	total_len;

	total_len = ft_strlen(s1) + ft_strlen(s2);
	newstring = ft_calloc((total_len + 1), sizeof(char));
	if (!newstring)
		return (NULL);
	ft_strlcpy(newstring, s1, total_len + 1);
	ft_strlcat(newstring, s2, total_len + 1);
	return (newstring);
}
