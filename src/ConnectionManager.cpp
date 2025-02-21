/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/04 14:02:23 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConnectionManager.hpp"

bool	g_quit = false;

void	sigint_handle(int signal)
{
	if (signal == SIGINT)
		g_quit = true;
}

ConnectionManager::ConnectionManager()
{
	
}

ConnectionManager::ConnectionManager(std::vector<Server *> servers)
{
	std::vector<struct pollfd>				sock_fds;
	for (std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		for (int i = 0; i < (*it).getIPs(); i++)
		{
			str	ipp = (*it).getIP(i) + ":" + (*it).getPort(i);
			std::vector<str>::iterator	found_socket = std::find(reqs.begin(), reqs.end(), ipp);
			if (found_socket == reqs.end())
			{
				servers_per_ippp.push_back(std::map<str, Server *>());
				reqs.push_back(ipp);
				addServerToMap(servers_per_ippp.back(), it);
			}
			else
				addServerToMap(servers_per_ippp.at(found_socket - reqs.begin()), it);
		}
	}
	for (int i = 0; i < reqs.size(); i++)
		reqs[i] = "";
}

struct sockaddr_in ConnectionManager::setupSocket(str ip, str port)
{
	struct sockaddr_in	ret;
	struct addrinfo		*info;
	const char			*caddr;
	const char			*cport;
	int					fd;
	int					opt = 1;
	
	caddr = (ip == "none" ? NULL : ip.c_str());
	cport = (port == "none" ? NULL : port.c_str());
	int addr_ret = getaddrinfo(caddr, cport, NULL, &info);
	if (addr_ret != 0)
	{
		std::cout << gai_strerror(addr_ret) << "\n";
		throw (std::exception());
	}
	ret = *((sockaddr_in *)info->ai_addr);
	fd = socket(PF_INET, SOCK_STREAM, 0);
	ret.sin_family = AF_INET;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(fd, (sockaddr *)&ret, sizeof(ret)) < 0)
	{
		perror("Bind in parameterized constructor");
		throw (std::exception());
	}
	if (listen(fd, 128) == -1)
		throw (std::exception());
}

void	ConnectionManager::addSocket(std::vector<struct pollfd> &sock_fds, str ip, str port)
{
	struct pollfd	temp;
	temp.fd = setupSocket(ip, port);
	fcntl(temp.fd, F_SETFL, fcntl(temp.fd, F_GETFL) | O_NONBLOCK);
	fcntl(temp.fd, F_SETFD, fcntl(temp.fd, F_GETFD) | FD_CLOEXEC);
	temp.events = POLLIN;
	temp.revents = 0;
	sock_fds.push_back(temp);
}

void	ConnectionManager::addServerToMap(std::map<str, Server *>	&map, Server &server)
{
	std::vector<str>	names = server.getNames();
	for (std::vector<str>::iterator it = names; it != names.end(); it++)
	{
		map.insert(std::pair<*it, &server>);
	}
}

ConnectionManager::~ConnectionManager()
{
	if (serv_sock != -1)
		close(serv_sock);
}

ConnectionManager::ConnectionManager(const ConnectionManager &obj)
{
	(void)obj;
}

ConnectionManager	&ConnectionManager::operator=(const ConnectionManager &obj)
{
	(void)obj;
	this->serv_sock = obj.serv_sock;
	return (*this);
}

void	ConnectionManager::startConnections()
{
	signal(SIGINT, sigint_handle);
	while (g_quit != true)
	{
	 	int res = poll(&sock_fds[0], sock_fds.size(), 500);
		// std::cout << "Number of sockets " << sock_fds.size() << "\n";
		if (res < 0)
		{
			std::cout << "TEST\n";
			perror("poll");
			exit(EXIT_FAILURE);
		}
		if (res == 0)
			continue ;
		if (sock_fds.at(0).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
		{
			struct pollfd		cli;
			struct sockaddr_in	client;
			socklen_t			len;
			int					acc_sock;

			len = sizeof(client);
			acc_sock = accept(sock_fds.at(0).fd, (sockaddr *)&client, &len);
			if (errno == EWOULDBLOCK)
			{
				std::cout << "Nothing to accept\n";
				continue ;
			}
			std::cout << "Received a request from a new client\n";
			cli.fd = acc_sock;
			fcntl(cli.fd, F_SETFL, fcntl(cli.fd, F_GETFL) | O_NONBLOCK);
			fcntl(cli.fd, F_SETFD, fcntl(cli.fd, F_GETFD) | FD_CLOEXEC);
			cli.events = POLLIN | POLLOUT;
			cli.revents = 0;
			sock_fds.push_back(cli);
			reqs.push_back("");
			std::cout << "Number of sockets in poll(): " << sock_fds.size() << "\n";
		}
		for (unsigned int i = 1; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLHUP)
			{
				std::cout << "Hangup\n";
				// if (i < listeners.size())
				// {
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					reqs.erase(reqs.begin() + i);
					i--;
				// }
				continue ;
			}
			if (sock_fds.at(i).revents & POLLERR)
			{
				std::cout << "Error\n";
				// if (i < listeners.size())
				// {
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					reqs.erase(reqs.begin() + i);
					i--;
				// }
				continue ;
			}
			if (sock_fds.at(i).revents & POLLNVAL)
			{
				std::cout << "INVALID\n";
				// if (i < listeners.size())
				// {
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					reqs.erase(reqs.begin() + i);
					i--;
				// }
				continue ;
			}
			if (sock_fds.at(i).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
			{
				j = 0;
				std::cout << "POLLIN(TAN)\n";
				char buf[2048];
				//memset(buf, 0, sizeof(buf));
				ssize_t	bytes = recv(sock_fds.at(i).fd, buf, sizeof(buf), 0);
				(void)bytes;
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (j == 0)
				{
					j = 1;
					std::cout << "POLLOUT(TAN)\n";
				}
			}
		}
	}
	if (g_quit == true)
	{
		// close all sockets
		for (unsigned int i = 0; i < sock_fds.size(); i++)
		{
			close(sock_fds.at(i).fd);
			sock_fds.pop_back();
			reqs.pop_back();
		}
		for (unsigned int i = 0; i < listeners.size(); i++)
		{
			delete listeners[i];
			listeners.pop_back();
		}
	}
	signal(SIGINT, SIG_DFLT);
}