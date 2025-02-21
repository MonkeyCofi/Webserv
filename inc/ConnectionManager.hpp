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
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <poll.h>
# include <signal.h>
# include <fcntl.h>
# include <pair>
# include <cstring>
# include <map>
# include <cstdio>
# include "Server.hpp"

typedef std::string str;

class ConnectionManager
{
	private:
		std::vector<std::string>				reqs;
		std::vector<struct pollfd>				sock_fds;
		std::vector<std::map<str, Server *>	>	servers_per_ippp;

		ConnectionManager();
		ConnectionManager(const ConnectionManager &obj);
		ConnectionManager	&operator=(const ConnectionManager &obj);
		void	addServerToMap(std::map<str, Server *>	&map, Server &server);
		void	addSocket(std::vector<struct pollfd> &sock_fds, str ip, str port);
		struct sockaddr_in setupSocket(str ip, str port);

	public:
		ConnectionManager(std::vector<Server *> servers);

		void	startConnections();

		~ConnectionManager();
};

#endif
