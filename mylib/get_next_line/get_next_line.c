/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmelo-do <lmelo-do@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 12:27:17 by lmelo-do          #+#    #+#             */
/*   Updated: 2025/10/24 16:38:07 by lmelo-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

static char	*gnl_read(int fd, char *stash)
{
	char	*buffer;
	int		bytes_read;
	char	*temp;

	if (!stash)
		stash = gnl_calloc(sizeof(char), 1);
	buffer = gnl_calloc(sizeof(char), BUFFER_SIZE + 1);
	if (!stash || !buffer)
		return (gnl_free(stash, buffer));
	bytes_read = 1;
	while (!ft_strchr(stash, '\n') && bytes_read > 0)
	{
		bytes_read = read(fd, buffer, BUFFER_SIZE);
		if (bytes_read == -1)
			return (gnl_free(stash, buffer));
		buffer[bytes_read] = '\0';
		temp = gnl_strjoin(stash, buffer);
		if (!temp)
			return (gnl_free(temp, buffer));
		free(stash);
		stash = temp;
	}
	free(buffer);
	return (stash);
}

static char	*gnl_extract_line(char *stash)
{
	int		i;
	int		j;
	char	*line;

	i = 0;
	if (!stash || !*stash)
		return (NULL);
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	line = malloc((i + 1) * sizeof(char));
	if (!line)
		return (NULL);
	j = 0;
	while (j < i)
	{
		line[j] = stash[j];
		j++;
	}
	line[j] = '\0';
	return (line);
}

static char	*gnl_trim_stash(char *stash)
{
	int		i;
	int		j;
	char	*new_stash;

	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	if (!stash[i])
		return (gnl_free(stash, NULL));
	new_stash = malloc((gnl_strlen(stash) - i + 1) * sizeof(char));
	if (!new_stash)
		return (gnl_free(stash, NULL));
	j = 0;
	while (stash[i])
		new_stash[j++] = stash[i++];
	new_stash[j] = '\0';
	free(stash);
	return (new_stash);
}

char	*get_next_line(int fd)
{
	static char	*stash[MAX_FD];
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0 || fd >= MAX_FD)
		return (NULL);
	stash[fd] = gnl_read(fd, stash[fd]);
	if (!stash[fd])
		return (NULL);
	line = gnl_extract_line(stash[fd]);
	stash[fd] = gnl_trim_stash(stash[fd]);
	return (line);
}
