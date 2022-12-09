/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   block-nonblock.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: artmende <artmende@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/09 15:12:06 by artmende          #+#    #+#             */
/*   Updated: 2022/12/09 16:21:34 by artmende         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int	main()
{

	fcntl(0, F_SETFL, O_NONBLOCK);
	char	buf[10];
	char	*final = NULL;

	int	read_return;
	while (0 < (read_return = read(0, buf, 9)))
	{
		buf[read_return] = 0;
		final = str_join(final, buf);
	}

	printf("data received is :\n%s\n", final);

	//printf("read_return is : %d\nstr received is : %s\n", read_return, buf);

	return 0;
}
