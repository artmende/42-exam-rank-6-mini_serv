/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: artmende <artmende@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/30 16:29:10 by artmende          #+#    #+#             */
/*   Updated: 2022/12/06 18:10:16 by artmende         ###   ########.fr       */
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
#include <sys/select.h>
#include <stdio.h>

#include <signal.h>


typedef struct s_client
{
	int					id;
	int					sock;
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	struct s_client		*next;
}	t_client;

typedef struct s_server
{
	int					listening_socket;
	struct sockaddr_in	addr;
	int					next_client_id;
	t_client			*client_list;
	fd_set				save_set;
}	t_server;

int	fatal_error()
{
	char	*msg = "Fatal error\n";
	write(2, msg, strlen(msg));
	exit(1);
	return (1);
}

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



void	setup_server(t_server *server, int port)
{
	if (server == NULL)
		fatal_error();
	server->next_client_id = 0;
	server->client_list = NULL;
	FD_ZERO(&server->save_set);
	// socket create and verification
	if (-1 == (server->listening_socket = socket(AF_INET, SOCK_STREAM, 0)))
		fatal_error();
	write(1, "Socket successfully created..\n", 30);
	bzero(&server->addr, sizeof(struct sockaddr_in));
	// assign IP, PORT
	server->addr.sin_family = AF_INET;
	server->addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	server->addr.sin_port = htons(port);
	// Binding newly created socket to given IP and verification 
	if (0 != (bind(server->listening_socket, (const struct sockaddr *)&server->addr, sizeof(struct sockaddr_in))))
		fatal_error();
	write(1, "Socket successfully binded..\n", 29);
	if (0 != listen(server->listening_socket, 10))
		fatal_error();
	write(1, "listen is successful\n", 21);
	FD_SET(server->listening_socket, &server->save_set);
}

void	add_client_and_announce(t_server *server, fd_set write_set)
{
	t_client	*new_client = calloc(1, sizeof(t_client));
	if (new_client == NULL)
		fatal_error();
	new_client->id = server->next_client_id;
	server->next_client_id = server->next_client_id + 1;
	new_client->addr_len = sizeof(struct sockaddr_in);
	new_client->sock = accept(server->listening_socket, (struct sockaddr *)&new_client->addr, &new_client->addr_len);
	if (new_client->sock < 0)
		fatal_error();
	FD_SET(new_client->sock, &server->save_set);

	char buf[42];
	sprintf(buf, "server: client %d just arrived\n", new_client->id);
	// add_front
	new_client->next = server->client_list;
	server->client_list = new_client;
	t_client *browse = new_client->next;
	while (browse)
	{
		if (FD_ISSET(browse->sock, &write_set))
			write(browse->sock, buf, strlen(buf));
		browse = browse->next;
	}
}

void	dispatch_message_from_client(t_client **client_browser, t_server *server, int *skip_increment, fd_set write_set) // need to access the save_set too to remove clients
{
	char	buf[10000];
	int		read_return = read((*client_browser)->sock, buf, 9999);
	if (read_return == 0)
	{
		// remove client and skip increment
		t_client	*to_delete = *client_browser;
		*client_browser = to_delete->next;
		*skip_increment = 1;
		if (server->client_list == to_delete) // deleting first node of the list
			server->client_list = to_delete->next;
		else
		{
			t_client	*parent_of_to_delete = server->client_list;
			while (parent_of_to_delete && parent_of_to_delete->next != to_delete)
				parent_of_to_delete = parent_of_to_delete->next;
			parent_of_to_delete->next = to_delete->next;
		}
		sprintf(buf, "server: client %d just left\n", to_delete->id);
		t_client	*to_write = server->client_list;
		while (to_write)
		{
			if (FD_ISSET(to_write->sock, &write_set))
				write(to_write->sock, buf, strlen(buf));
			to_write = to_write->next;
		}
		close(to_delete->sock);
		FD_CLR(to_delete->sock, &server->save_set);
		free(to_delete);
	}
	else
	{
		buf[read_return] = 0;
		char	buf2[11000];
		int length = sprintf(buf2, "client %d: %s", (*client_browser)->id, buf);
		t_client	*to_write = server->client_list;
		while (to_write)
		{
			if (to_write->id != (*client_browser)->id && FD_ISSET(to_write->sock, &write_set))
				write(to_write->sock, buf2, length);
			to_write = to_write->next;
		}
	}
}

void	handler(int	i)
{
	(void)i;
	printf("inside handler\n");
	system("leaks a.out");
	exit(1);
}

int main(int argc, char **argv)
{

	signal(SIGUSR1, handler);

	if (argc != 2)
	{
		write(1, "wrong number of arguments\n", 26);
		exit(1);
	}

	t_server	mini_serv;

	setup_server(&mini_serv, atoi(argv[1]));

	while (1)
	{
		fd_set	read_set = mini_serv.save_set;
		fd_set	write_set = mini_serv.save_set; // no need to remove the listening socket, because we will check the client list only
		if (select(FD_SETSIZE, &read_set, &write_set, NULL, NULL) == -1)
			write(1, "select\n", 7) && fatal_error();

		if (FD_ISSET(mini_serv.listening_socket, &read_set))
			add_client_and_announce(&mini_serv, write_set); // accept the new client, add it to save_set, annouce to all others and then add to the list

		t_client	*client_browser = mini_serv.client_list;
		while (client_browser)
		{
			int	skip_increment = 0;
			if (FD_ISSET(client_browser->sock, &read_set))
				dispatch_message_from_client(&client_browser, &mini_serv, &skip_increment, write_set); // read the message and send it to all other clients from the list. If message has length 0, we remove the client from the list and notify all others that he left
			if (skip_increment == 0) // if a client was removed, we should not increment, the browser will already point to the next client (or to NULL)
				client_browser = client_browser->next;
		}
	}
	return (0);
}
