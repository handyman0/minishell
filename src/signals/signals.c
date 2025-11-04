/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 18:00:11 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:11:19 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/signals.h"
#include <signal.h>

void	setup_signals(void)
{
	// Configuração basica de sinais
	signal(SIGINT, SIG_IGN); // Ignora o Ctrl+c por enquanto
	signal(SIGQUIT, SIG_IGN); // Ignora o Ctrl+\ por enquanto
}
