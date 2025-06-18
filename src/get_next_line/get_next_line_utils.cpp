/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/27 19:33:00 by pipolint          #+#    #+#             */
/*   Updated: 2023/12/21 15:01:49 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.hpp"

char	*ft_newlinejoin(char *result, char *buffer)
{
	char	*newStr;
	size_t	i;
	int		j;

	i = 0;
	j = -1;
	while (buffer[i] && buffer[i] != '\n')
		i++;
	if (buffer[i] && buffer[i] == '\n')
		i++;
	newStr = new char[ft_strlen(result) + i + 1];
	if (!newStr)
		return (NULL);
	i = 0;
	while (result[++j])
		newStr[j] = result[j];
	while (buffer[i] && buffer[i] != '\n')
		newStr[j++] = buffer[i++];
	if (buffer[i] && buffer[i] == '\n')
		newStr[j++] = buffer[i++];
	newStr[j] = '\0';
	return (newStr);
}
