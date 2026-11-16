/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 05:28:14 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/08/07 20:22:31 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putnbr(long long n)
{
	unsigned long long	nb;
	int					count;

	count = 0;
	if (n < 0)
	{
		count += ft_putchar('-');
		nb = (unsigned long long)(-n);
	}
	else
		nb = (unsigned long long)n;
	if (nb >= 10)
		count += ft_putnbr(nb / 10);
	count += ft_putchar((nb % 10) + '0');
	return (count);
}
