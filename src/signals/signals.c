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
#include <unistd.h>

volatile sig_atomic_t	g_signal = 0;
volatile sig_atomic_t	g_trap_pending[65];

void	trap_handler(int sig)
{
	if (sig > 0 && sig < 65)
		g_trap_pending[sig] = 1;
}

void	setup_signals(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	trap_reapply();
}

void	setup_signals_noninteractive(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	trap_reapply();
}

void	signals_ignore(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	trap_reapply();
}

void	signals_child(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
}
