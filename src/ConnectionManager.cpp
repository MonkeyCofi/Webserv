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

#include "ConnectionManager.hpp"

bool	g_quit = false;

void sigint_handle(int signal)
{
	if (signal == SIGINT)
		g_quit = true;
	std::cout << "\n";
}

ConnectionManager::ConnectionManager(): main_listeners(0)
{
	
}

ConnectionManager::ConnectionManager(Http *protocol): main_listeners(0)
{
	if (!protocol)
		throw std::runtime_error("e1");
	std::vector<Server *>	servers = protocol->getServers();
	for (std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		for (unsigned int i = 0; i < (*it)->getIPs().size(); i++)
		{
			str	ipp = (*it)->getIP(i) + ":" + (*it)->getPort(i);
			std::vector<str>::iterator	found_socket = std::find(reqs.begin(), reqs.end(), ipp);
			if (found_socket == reqs.end())
			{
				addSocket((*it)->getIP(i), (*it)->getPort(i));
				servers_per_ippp.push_back(std::map<str, Server *>());
				reqs.push_back(ipp);
				defaults.push_back(*it);
				handlers.push_back(NULL);
				addServerToMap(servers_per_ippp.back(), **it);
			}
			else
				addServerToMap(servers_per_ippp.at(found_socket - reqs.begin()), **it);
		}
	}
	for (unsigned int i = 0; i < reqs.size(); i++)
		reqs[i] = "";
}

int ConnectionManager::setupSocket(str ip, str port)
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
		freeaddrinfo(info);
		throw (std::exception());
	}
	if (listen(fd, 128) == -1)
	{
		freeaddrinfo(info);
		throw (std::exception());
	}
	freeaddrinfo(info);
	return fd;
}

void ConnectionManager::addSocket(str ip, str port)
{
	struct pollfd	temp;
	temp.fd = setupSocket(ip, port);
	fcntl(temp.fd, F_SETFL, fcntl(temp.fd, F_GETFL) | O_NONBLOCK);
	fcntl(temp.fd, F_SETFD, fcntl(temp.fd, F_GETFD) | FD_CLOEXEC);
	temp.events = POLLIN | POLLPRI;
	temp.revents = 0;
	sock_fds.push_back(temp);
}

void ConnectionManager::addServerToMap(std::map<str, Server *>	&map, Server &server)
{
	std::vector<str>	names = server.getNames();
	for (std::vector<str>::iterator it = names.begin(); it != names.end(); it++)
		map[*it] = &server;
}

ConnectionManager::~ConnectionManager()
{
	
}

ConnectionManager::ConnectionManager(const ConnectionManager &obj): main_listeners(0)
{
	(void)obj;
}

ConnectionManager &ConnectionManager::operator=(const ConnectionManager &obj)
{
	(void)obj;
	return (*this);
}

void ConnectionManager::newClient(int i, struct pollfd sock)
{
	struct sockaddr_in	client_addr;
	struct pollfd		client;
	socklen_t			len;
	int					acc_sock;

	len = sizeof(client_addr);
	acc_sock = accept(sock.fd, (sockaddr *)&client_addr, &len);
	// if (errno == EWOULDBLOCK)
	// {
	// 	std::cout << "Nothing to accept\n";
	// 	return ;
	// }
	std::cout << "Received a request from a new client\n";
	client.fd = acc_sock;
	fcntl(client.fd, F_SETFL, fcntl(client.fd, F_GETFL) | O_NONBLOCK);
	fcntl(client.fd, F_SETFD, fcntl(client.fd, F_GETFD) | FD_CLOEXEC);
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	handlers.push_back(NULL);
	sock_fds.push_back(client);
	reqs.push_back("");
	defaults.push_back(defaults.at(i));
	servers_per_ippp.push_back(std::map<str, Server *>(servers_per_ippp.at(i)));
}

void ConnectionManager::printError(int revents)
{
	if (revents & POLLHUP)
		std::cerr << "Hangup\n";
	else if (revents & POLLERR)
		std::cerr << "Error\n";
	else if (revents & POLLNVAL)
		std::cerr << "INVALID\n";
}

void ConnectionManager::passRequestToServer(int i, Request **req)
{
	std::cout << "HOSTNAME : " << (*req)->getHost() << "\n";
	if (!(*req)->isValidRequest() || servers_per_ippp.at(i).find((*req)->getHost()) == servers_per_ippp.at(i).end())
		handlers.at(i) = defaults.at(i);
	else
		handlers.at(i) = servers_per_ippp.at(i)[(*req)->getHost()];
	handlers.at(i)->handleRequest(*req);
	delete *req;
	*req = NULL;
}

void ConnectionManager::startConnections()
{
	int		res;
	char	buffer[4096];
	ssize_t	bytes;
	Request	*req = NULL;

	for(std::vector<std::map<str, Server *> >::iterator it2 = servers_per_ippp.begin(); it2 != servers_per_ippp.end(); it2++)
	{
		std::cout << "=-=-=-=-==--=-=-=-=-=-=\n";
		for(std::map<str, Server *>::iterator it = it2->begin(); it != it2->end(); it++)
			std::cout << "Server per hostname " << it->first << ": " << (*it2)[it->first] << "\n";
	}
	// return ;
	main_listeners = sock_fds.size();
	signal(SIGINT, sigint_handle);
	while (g_quit != true)
	{
	 	res = poll(&sock_fds[0], sock_fds.size(), 500);
		if (res == 0)
			continue ;
		if (res < 0)
		{
			if (g_quit)
				break ;
			perror("poll");
			throw std::runtime_error("e2");
		}
		for (unsigned int i = 0; i < main_listeners; i++)
		{
			if (sock_fds.at(i).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
				newClient(i, sock_fds.at(i));
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			else if (sock_fds.at(i).revents & POLLIN)
			{
				reqs.at(i) = "";
				memset(buffer, 0, sizeof(buffer));
				while ((bytes = recv(sock_fds.at(i).fd, buffer, sizeof(buffer), 0)) > 0)
				{
					reqs.at(i) += str(buffer);
					memset(buffer, 0, sizeof(buffer));
				}
				std::cout << "% " << bytes << "\n";
				if (bytes == 0)
				{
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					reqs.erase(reqs.begin() + i);
					i--;
					continue ;
				}
				std::cout << "==> " << i << std::endl;
				std::cout << "\n--------------\n";
				std::cout << reqs.at(i) << "\n";
				std::cout << "--------------\n\n";
				std::cout << "Beginning:\n";
				req = new Request(reqs.at(i));
				passRequestToServer(i, &req);
			}
			else if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handlers.at(i))
				{
					if (!handlers.at(i)->respond(sock_fds.at(i).fd))
					{
						close(sock_fds.at(i).fd);
						sock_fds.erase(sock_fds.begin() + i);
						reqs.erase(reqs.begin() + i);
						i--;
					}
				}
				else
				{
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					reqs.erase(reqs.begin() + i);
					i--;
				}
			}
			else
			{
				printError(sock_fds.at(i).revents);
				close(sock_fds.at(i).fd);
				sock_fds.erase(sock_fds.begin() + i);
				reqs.erase(reqs.begin() + i);
				i--;
			}
		}
	}
	for (unsigned int i = 0; i < sock_fds.size(); i++)
	{
		close(sock_fds.at(i).fd);
		sock_fds.pop_back();
		reqs.pop_back();
	}
	signal(SIGINT, SIG_DFL);
}