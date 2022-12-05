/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: artmende <artmende@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/30 16:29:10 by artmende          #+#    #+#             */
/*   Updated: 2022/12/05 17:54:14 by artmende         ###   ########.fr       */
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
	fd_set				save_set;
	int					next_client_id;
}	t_server;

void	fatal_error()
{
	char	*msg = "Fatal error\n";
	write(2, msg, strlen(msg));
	exit(1);
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

t_client	*add_client(t_client *current_list, t_server *server, fd_set write_set)
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
	// add_back the new client (and announce to all others)
	if (current_list == NULL)
		return (new_client);
	else
	{
		char	buf[42];
		sprintf(buf, "server: client %d just arrived\n", new_client->id);
		t_client	*browse = current_list;
		while (browse->next)
		{
			if (FD_ISSET(browse->sock, &write_set))
				write(browse->sock, buf, strlen(buf));
			browse = browse->next;
		}
		if (FD_ISSET(browse->sock, &write_set))
			write(browse->sock, buf, strlen(buf));
		browse->next = new_client;
		return (current_list);
	}
}

void	dispatch_message_from_client(t_client **browser, t_client **list, int *skip_increment) // need to access the save_set too to remove clients
{
	
}


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		write(1, "wrong number of arguments\n", 26);
		exit(1);
	}

	t_server	mini_serv;
	t_client	*client_list = NULL;

	setup_server(&mini_serv, atoi(argv[1]));

	while (1)
	{
		fd_set	read_set = mini_serv.save_set;
		fd_set	write_set = mini_serv.save_set; // no need to remove the listening socket, because we will check the client list only
		if (select(FD_SETSIZE + 1, &read_set, &write_set, NULL, NULL) == -1)
			fatal_error();

		if (FD_ISSET(mini_serv.listening_socket, &read_set))
			client_list = add_client(client_list, &mini_serv, write_set); // accept the new client, add it to save_set, annouce to all others and then add to the list

		t_client	*client_browser = client_list;
		while (client_browser)
		{
			int	skip_increment = 0;
			if (FD_ISSET(client_browser->sock, &read_set))
				dispatch_message_from_client(&client_browser, &client_list, &skip_increment); // read the message and send it to all other clients from the list. If message has length 0, we remove the client from the list and notify all others that he left
			if (skip_increment == 0) // if a client was removed, we should not increment, the browser will already point to the next client (or to NULL)
				client_browser = client_browser->next;
		}
	}
}

// make the client list part of the server struct. We can pass the server struct to all the functions, and it can always access everything

