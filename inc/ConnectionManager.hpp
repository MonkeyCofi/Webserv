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
# include <unistd.h>
# include <cstring>
# include <cstdio>
# include "Server.hpp"

typedef std::string str;

class ConnectionManager
{
	private:
		std::vector<struct pollfd>	sock_fds;
		std::vector<std::string>	reqs;
		std::vector<Socket *>		listeners;

	public:
		ConnectionManager();
		~ConnectionManager();
		ConnectionManager(const ConnectionManager &obj);
		ConnectionManager	&operator=(const ConnectionManager &obj);
		
		ConnectionManager(Server &obj, int listener_index);
		int					returnConnectionManager(int index);
		struct sockaddr_in	returnClient();
		
		void	printAddress();
		
		class	BindException
		{
			const char*	what();
		};
		
		class	AddrinfoException
		{
			const char*	what();
		};
		class	ListenException
		{
			const char*	what();
		};
};

#endif
