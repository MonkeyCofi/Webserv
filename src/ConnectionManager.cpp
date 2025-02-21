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
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	this->server.sin_port = htons(80);
	this->server.sin_family = AF_INET;
	this->server.sin_addr.s_addr = inet_addr("0.0.0.0");
	// bzero(&this->server.sin_zero, sizeof(this->server.sin_zero));
	memset(&this->server.sin_zero, 0, sizeof(this->server.sin_zero));
	if (bind(serv_sock, (sockaddr *)&this->server, sizeof(this->server)) < 0)
	{
		perror(NULL);
		throw (std::exception());
	}
}

ConnectionManager::ConnectionManager(std::vector<Server *> servers)
{
	std::vector<std::string>				reqs;
	std::vector<struct pollfd>				sock_fds;
	std::vector<std::map<str, Server *>	>	servers_per_ippp;
	for (std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		for ()
		{
			servers_per_ippp.push_back(std::map<str, Server *>());
		}
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