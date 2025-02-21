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

}