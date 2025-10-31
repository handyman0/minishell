/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 20:03:08 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/31 19:37:51 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

# include "../libft/include/libft.h"
# include "parser.h"

int		ft_isspace(char c);
void	print_tokens(t_token *tokens); // Para debug
void	print_tree(t_node *node, int depth); // Para debug
void	free_str_array(char **arr);

#endif
