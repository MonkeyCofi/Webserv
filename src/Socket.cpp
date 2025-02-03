/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/03 17:55:13 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket()
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

Socket::Socket(Server &obj, int listener_index)
{
	struct addrinfo	*info;
	struct sockaddr_in	*temp;
	const char	*addr;
	const char	*port;
	
	addr = (obj.returnIP(listener_index) == "none" ? NULL : obj.returnIP(listener_index).c_str());
	port = (obj.returnPort(listener_index) == "none" ? NULL : obj.returnPort(listener_index).c_str());
	int addr_ret = getaddrinfo(addr, port, NULL, &info);
	if (addr_ret != 0)
	{
		std::cout << gai_strerror(addr_ret) << "\n";
		throw (AddrinfoException());
	}
	temp = (sockaddr_in *)info->ai_addr;
	this->server = *temp;
	this->serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	char	buf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &server.sin_addr, buf, INET_ADDRSTRLEN);
	std::cout << "Address is: " << buf << "\n";
	//server.sin_port = htons(atoi(obj.returnPort(listener_index).c_str()));
	//if (obj.returnIP(listener_index) != "none")
	//{
	//	std::cout << "Address to bind: " << obj.returnIP(listener_index) << "\n";
	//	//server.sin_addr.s_addr = inet_addr(obj.returnIP(listener_index).c_str());
	//}
	//else
	//	server.sin_addr.s_addr = INADDR_ANY;
	int	opt = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(serv_sock, (sockaddr *)&this->server, sizeof(this->server)) < 0)
	{
		perror("Bind in parameterized constructor");
		throw (BindException());
	}
	if (listen(this->serv_sock, 128) == -1)
		throw (ListenException());
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

const char*	Socket::BindException::what()
{
	return ("Bind failed");
}

const char*	Socket::AddrinfoException::what()
{
	return ("Failed to get address info");
}

const char*	Socket::ListenException::what()
{
	return ("Failed to create connection");
}
