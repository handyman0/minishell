/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putptr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 05:28:14 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/08/11 22:18:18 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putptr(void *ptr)
{
	if (ptr == NULL)
		return (ft_putstr("(nil)"));
	return (ft_putstr("0x") + ft_puthex((unsigned long long)ptr, 0));
}
