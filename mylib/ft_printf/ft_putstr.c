/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putstr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 05:28:14 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/08/07 19:48:11 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putstr(const char *s)
{
	int	count;

	if (!s)
		return (ft_putstr("(null)"));
	count = 0;
	while (*s)
		count += ft_putchar(*s++);
	return (count);
}
