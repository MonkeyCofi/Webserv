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

typedef std::string str;

class ConnectionManager
{
	private:
		static size_t	max_request_size;
		std::vector<std::string>				reqs;
		std::vector<struct pollfd>				sock_fds;
		std::vector<std::map<str, Server *>	>	servers_per_ippp;
		unsigned int							main_listeners;

		ConnectionManager();
		ConnectionManager(const ConnectionManager &obj);
		ConnectionManager	&operator=(const ConnectionManager &obj);
	
		int		setupSocket(str ip, str port);
		void	addServerToMap(std::map<str, Server *>	&map, Server &server);
		void	addSocket(std::vector<struct pollfd> &sock_fds, str ip, str port);
		void	newClient(struct pollfd client);
		void	printError(int revents);

	public:
		ConnectionManager(Http *protocol);

		void	startConnections();

		~ConnectionManager();
};

#endif
