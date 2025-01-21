/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/01/21 17:38:24 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket()
{
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	this->server.sin_port = htons(80);
	this->server.sin_family = AF_INET;
	this->server.sin_addr.s_addr = inet_addr("0.0.0.0");
	bzero(&this->server.sin_zero, sizeof(this->server.sin_zero));
	if (bind(serv_sock, (sockaddr *)&this->server, sizeof(this->server)) < 0)
	{
		perror(NULL);
		throw (std::exception());
	}
}

Socket::~Socket()
{
	if (serv_sock != -1)
		close(serv_sock);
}

Socket::Socket(const Socket &obj)
{
	(void)obj;
}

Socket	&Socket::operator=(const Socket &obj)
{
	(void)obj;
	this->serv_sock = obj.serv_sock;
	return (*this);
}

Socket::Socket(Server &obj, int serv_index)
{
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	// get the port and address from the server object
	std::cout << "Port: " << obj.returnPort(serv_index) << "\n";
	server.sin_port = htons(atoi(obj.returnPort(serv_index).c_str()));
	if (obj.returnIP(serv_index) != "none")
		server.sin_addr.s_addr = inet_addr(obj.returnIP(serv_index).c_str());
	else
		server.sin_addr.s_addr = INADDR_ANY;
	if (bind(serv_sock, (sockaddr *)&this->server, sizeof(this->server)) < 0)
	{
		perror("Bind");
		throw (std::exception());
	}
}

int	Socket::returnSocket(int index)
{
	(void)index;
	return (this->serv_sock);
}

struct sockaddr_in	Socket::returnClient()
{
	return (this->client);
}
