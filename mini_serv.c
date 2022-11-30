/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: artmende <artmende@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/30 16:29:10 by artmende          #+#    #+#             */
/*   Updated: 2022/11/30 17:58:28 by artmende         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// clients are struct that have an ID and a socket
// var in the main function to keep track of client id
// main loop, select from accepting, select for reading, select for writing

// concept : a message. as soon as a message is read, we browse all socket selected for writing and we send to all of them that doesnt match the socket that we listened from

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

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


int main(int argc, char **argv)
{

	if (argc != 2)
	{
		write(1, "wrong number of arguments\n", 26);
		exit(1);
	}

	int sockfd, connfd;
	socklen_t	len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
	{
		write(1, "socket creation failed...\n", 26);
		exit(0); 
	}
	else
		write(1, "Socket successfully created..\n", 30);

	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		write(1, "socket bind failed...\n", 22);
		exit(0); 
	}
	else
		write(1, "Socket successfully binded..\n", 29);

	if (listen(sockfd, 10) != 0)
	{
		write(1, "cannot listen\n", 14);
		exit(0); 
	}

	while (1)
	{
		// save a copy of read set and write set
		// select()
		// check if listening socket has been selected
		// if yes, accept new connection and load message to be sent to everyone
	}



	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr *)&cli, &len);

	if (connfd < 0)
	{
		write(1, "server acccept failed...\n", 25);
		exit(0);
	}
	else
	{
		write(1, "server acccept the client...\n", 29);
		write(connfd, "youplaboum\n", 11);
	}
}
