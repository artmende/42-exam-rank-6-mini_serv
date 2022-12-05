/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: artmende <artmende@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/01 15:09:56 by artmende          #+#    #+#             */
/*   Updated: 2022/12/05 15:45:21 by artmende         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <stdio.h>

int	main()
{
	fd_set	read_set, write_set, save_set;
	struct sockaddr_in servaddr, client_addr;

	int	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		write(1, "socket creation failed...\n", 26);
		exit(0); 
	}
	else
		write(1, "Socket successfully created..\n", 30);

//fcntl(sockfd, F_SETFL, O_NONBLOCK); /////////////////////////////////////

	bzero(&servaddr, sizeof(servaddr));
	bzero(&client_addr, sizeof(struct sockaddr_in));
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(9000); 

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

	FD_ZERO(&save_set);
	FD_SET(sockfd, &save_set);

	int	client_sock = -1;
	socklen_t	len_addr_client;

	while (1)
	{
		read_set = save_set;
		write_set = save_set;
		if (-1 == select(FD_SETSIZE, &read_set, &write_set, NULL, NULL))
			{write(1, "error select\n", 13); exit(1);}
		if (FD_ISSET(sockfd, &read_set))
		{
			client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &len_addr_client);
			if (client_sock == -1)
				{write(1, "error accept\n", 13); exit(1);}
			printf("new fd is : %d\n", client_sock);
			//fcntl(client_sock, F_SETFL, O_NONBLOCK); //////////////////////////////
			FD_SET(client_sock, &save_set);
			continue;
		}
		if (FD_ISSET(5645646, &read_set)) // no error when checking random fd
		{
			printf("yop je suis la");
		}
		if (client_sock > 0 && FD_ISSET(client_sock, &read_set))
		{
			write(1, "something to read\n", 18);
			char	*buf = calloc(1, sizeof(char) * 1000);
			int	read_ret = read(client_sock, buf, 1000);
			if (read_ret > 0)
				write(1, buf, read_ret);
			else if (read_ret == 0)
			{
				write(1, "Client has left\n", 16);
				close(client_sock);
				FD_CLR(client_sock, &save_set);
				client_sock = -1;
				bzero(&client_addr, sizeof(struct sockaddr_in));
			}
			else
				write(1, "error with read\n", 16);
			free(buf);
		}
	}

	return 0;
}
