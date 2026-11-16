/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 21:14:11 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 16:34:09 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

size_t	gnl_strlen(const char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

char	*gnl_strjoin(char const *s1, char const *s2)
{
	char	*result;
	int		i;
	int		j;

	i = -1;
	j = -1;
	if (!s1 && !s2)
		return (NULL);
	if (!s1)
		return (gnl_strdup(s2));
	if (!s2)
		return (gnl_strdup(s1));
	result = malloc((gnl_strlen(s1) + gnl_strlen(s2) + 1) * sizeof(char));
	if (!result)
		return (NULL);
	while (s1[++i])
		result[i] = s1[i];
	while (s2[++j])
		result[i + j] = s2[j];
	result[i + j] = '\0';
	return (result);
}

void	*gnl_calloc(size_t n, size_t size)
{
	size_t			total;
	void			*ptr;
	size_t			i;
	unsigned char	*p;

	if (n == 0 || size == 0)
		return (malloc(1));
	if (size != 0 && n > (size_t)-1 / size)
		return (NULL);
	total = n * size;
	ptr = malloc(n * size);
	if (!ptr)
		return (NULL);
	p = (unsigned char *)ptr;
	i = 0;
	while (i < total)
		p[i++] = 0;
	return (ptr);
}

void	*gnl_free(void *ptr, void *ptr2)
{
	free(ptr);
	free(ptr2);
	return (NULL);
}

char	*gnl_strdup(const char *s)
{
	char	*dup;
	size_t	len;
	size_t	i;

	len = gnl_strlen(s);
	dup = malloc((len + 1) * sizeof(char));
	if (!dup)
		return (NULL);
	i = 0;
	while (s[i])
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
}
