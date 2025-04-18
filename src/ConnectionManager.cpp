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
	this->header_complete = false;
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
		throw std::runtime_error(gai_strerror(addr_ret));
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
	if (acc_sock == -1)
		return ;
	client.fd = acc_sock;
	fcntl(client.fd, F_SETFL, fcntl(client.fd, F_GETFL) | O_NONBLOCK);
	fcntl(client.fd, F_SETFD, fcntl(client.fd, F_GETFD) | FD_CLOEXEC);
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	sock_fds.push_back(client);
	reqs.push_back("");
	handlers.push_back(NULL);
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
	if ((*req) == NULL)
	{
		std::cout << "Request is NULL\n";
		return ;
	}
	if (!(*req)->isValidRequest() || servers_per_ippp.at(i).find((*req)->getHost()) == servers_per_ippp.at(i).end())
		handlers.at(i) = defaults.at(i);
	else
		handlers.at(i) = servers_per_ippp.at(i)[(*req)->getHost()];
	handlers.at(i)->handleRequest(*req);
	delete *req;
	*req = NULL;
}

Request*	ConnectionManager::receiveRequest(int client_fd, unsigned int& index)
{
	char	buffer[BUFFER_SIZE + 1];
	ssize_t	r;
	ssize_t	bytes_read;

	std::memset(buffer, 0, BUFFER_SIZE + 1);
	r = 1;
	bytes_read = 0;
	this->request_header = "";
	while (r > 0)
	{
		if (buffer[0] != 0)
			std::memset(buffer, 0, BUFFER_SIZE + 1);
		r = recv(client_fd, buffer, BUFFER_SIZE, 0);
		if (r < 0)
		{
			std::cerr << "Recv: -1\n";
			return (NULL);
		}
		else if (r == 0)
		{
			close(sock_fds.at(index).fd);
			sock_fds.erase(sock_fds.begin() + index);
			reqs.erase(reqs.begin() + index);
			handlers.erase(handlers.begin() + index);
			defaults.erase(defaults.begin() + index);
			servers_per_ippp.erase(servers_per_ippp.begin() + index);
			index--;
			return (new Request(this->request_header));
		}
		bytes_read += r;
		this->request_header.append(buffer, BUFFER_SIZE);	// append what was read to the request_header string
		std::cout << "\033[32mHeader: " << this->request_header << "\033[0m\n";
		if (this->request_header.find("\r\n\r\n") != std::string::npos)	// the header has been fully received
		{
			std::cout << "Found end of header\n";
			this->header_complete = true;
			return (new Request(this->request_header));
		}
	}
	return (NULL);
}

void	ConnectionManager::parseBody()
{
	std::ofstream	bodyFile;
	// open a temporary file in binary mode
	bodyFile.open(".temp", std::ios::binary);
	// get the position of 
	// read the body into the file
	// if the size exceeds CLIENT_MAX_BODY_SIZE, return with an error and close the socket
	// if the bytes read are less than 
}

void ConnectionManager::startConnections()
{
	int		res;
	// char	buffer[BUFFER_SIZE];
	// ssize_t	bytes;
	Request	*req = NULL;
	
	main_listeners = sock_fds.size();
	signal(SIGPIPE, SIG_IGN);
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
			perror("Poll");
			throw std::runtime_error("unexpected error in poll function");
		}
		for (unsigned int i = 0; i < main_listeners; i++)
		{
			if (sock_fds.at(i).revents & POLLIN)
				newClient(i, sock_fds.at(i));
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				// ssize_t	r = 0;
				// std::cout << "IN POLLIN\n";
				// reqs.at(i) = "";
				// std::memset(buffer, 0, BUFFER_SIZE);
				// bytes = recv(sock_fds.at(i).fd, buffer, BUFFER_SIZE, 0);
				// if (bytes == 0)
				// {
				// 	close(sock_fds.at(i).fd);
				// 	sock_fds.erase(sock_fds.begin() + i);
				// 	reqs.erase(reqs.begin() + i);
				// 	handlers.erase(handlers.begin() + i);
				// 	defaults.erase(defaults.begin() + i);
				// 	servers_per_ippp.erase(servers_per_ippp.begin() + i);
				// 	i--;
				// 	continue ;
				// }
				// reqs.at(i).append(buffer);
				// std::cout << reqs.at(i) << "\n";
				// std::cout << "\033[32mRead " << r << "  bytes\033[0m\n";
				// req = new Request(this->reqs.at(i));
				req = receiveRequest(sock_fds.at(i).fd, i);
				// // header has been parsed; now go through body and store into disk
				if (req && (req->getContentType() != "" || req->getContentLen() != 0))	// indicates that request contains body
					parseBody();
				this->passRequestToServer(i, &req);
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handlers.at(i))
				{
					if (handlers.at(i)->respond(sock_fds.at(i).fd))
					{
						close(sock_fds.at(i).fd);
						sock_fds.erase(sock_fds.begin() + i);
						reqs.erase(reqs.begin() + i);
						handlers.erase(handlers.begin() + i);
						defaults.erase(defaults.begin() + i);
						servers_per_ippp.erase(servers_per_ippp.begin() + i);
						i--;
					}
				}
				else
					continue;
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				printError(sock_fds.at(i).revents);
				close(sock_fds.at(i).fd);
				sock_fds.erase(sock_fds.begin() + i);
				reqs.erase(reqs.begin() + i);
				handlers.erase(handlers.begin() + i);
				defaults.erase(defaults.begin() + i);
				servers_per_ippp.erase(servers_per_ippp.begin() + i);
				i--;
			}
		}
	}
	std::cout << "Ending Server...\n";
	for (unsigned int i = 0; i < sock_fds.size(); i++)
	{
		close(sock_fds.at(i).fd);
		sock_fds.pop_back();
		reqs.pop_back();
		handlers.pop_back();
		defaults.pop_back();
		servers_per_ippp.pop_back();
	}
	std::cout << "Server closed!\n";
	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
}
