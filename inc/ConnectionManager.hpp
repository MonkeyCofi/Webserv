/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehammoud <ehammoud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 16:48:00 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/05 12:35:07 by ehammoud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONMANAGERS_HPP
# define CONNECTIONMANAGERS_HPP

# include <netdb.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <iostream>
# include <exception>
# include <algorithm>
# include <cerrno>
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <poll.h>
# include <signal.h>
# include <fcntl.h>
# include <cstring>
# include <map>
# include <cstdio>
# include "Http.hpp"
# include "Server.hpp"
# include "Request.hpp"

# define BUFFER_SIZE 8192
typedef std::string str;

class ConnectionManager
{
	private:
		unsigned int							main_listeners;
		std::vector<Server *>					defaults;
		std::vector<Server *>					handlers;
		std::vector<std::string>				reqs;
		std::vector<struct pollfd>				sock_fds;
		std::vector<std::map<str, Server *>	>	servers_per_ippp;

		ConnectionManager();
		ConnectionManager(const ConnectionManager &obj);
		ConnectionManager	&operator=(const ConnectionManager &obj);
	
		int		setupSocket(str ip, str port);
		void	addServerToMap(std::map<str, Server *>	&map, Server &server);
		void	addSocket(str ip, str port);
		void	newClient(int i, struct pollfd sock);
		void	printError(int revents);
		void	passRequestToServer(int i, Request **req);

	public:
		ConnectionManager(Http *protocol);

		void	startConnections();

		~ConnectionManager();
};

#endif
