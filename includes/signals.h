/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:03:04 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/11/04 18:11:42 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

# include <signal.h>

extern volatile sig_atomic_t	g_signal;
extern volatile sig_atomic_t	g_trap_pending[65];

void	trap_handler(int sig);
void	trap_reapply(void);

void	setup_signals(void);
void	setup_signals_noninteractive(void);
void	signals_ignore(void);
void	signals_child(void);

#endif
