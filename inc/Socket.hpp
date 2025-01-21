/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 16:48:00 by pipolint          #+#    #+#             */
/*   Updated: 2025/01/21 20:09:07 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETS_HPP
# define SOCKETS_HPP

# include <netdb.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <iostream>
# include <exception>
# include <unistd.h>
# include <cstring>
# include <cstdio>
# include "Server.hpp"

typedef std::string str;

class Socket
{
	private:
		struct sockaddr_in	server;
		struct sockaddr_in	client;
		int					serv_sock;
		//std::vector<int>	serv_sock;
	public:
		Socket();
		~Socket();
		Socket(const Socket &obj);
		Socket	&operator=(const Socket &obj);
		Socket  (Server &obj, int server_index);
		
		Socket(Server &obj, int serv_index);
		int	returnSocket(int index);
		struct sockaddr_in	returnClient();
		
		class	BindException
		{
			const char*	what();
		};
};

#endif
