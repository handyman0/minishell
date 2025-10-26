/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 16:07:17 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:16:38 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

void	*ft_memset(void *mem, int byte, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
		*(char *)(mem + i++) = byte;
	return (mem);
}
