/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 14:36:06 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/09/12 04:15:32 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static int	ft_numlen(int n)
{
	int	len;

	if (n == 0)
		return (1);
	len = 0;
	while (n != 0)
	{
		n /= 10;
		len++;
	}
	return (len);
}

char	*ft_itoa(int n)
{
	char	*str;
	int		len;
	int		sign;

	sign = 1;
	if (n < 0)
		sign = -1;
	len = ft_numlen(n) + (sign == -1);
	str = (char *)malloc(len + 1);
	if (!str)
		return (NULL);
	str[len] = '\0';
	while (len--)
	{
		if (sign == -1)
			str[len] = -(n % 10) + '0';
		else
			str[len] = (n % 10) + '0';
		n /= 10;
	}
	if (sign == -1)
		str[0] = '-';
	return (str);
}
