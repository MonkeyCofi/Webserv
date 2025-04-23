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
	this->state = HEADER;
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
	size_t	written = 0;
	Request	*req;

	std::memset(buffer, 0, BUFFER_SIZE + 1);
	r = 1;
	bytes_read = 0;
	this->request_header = "";
	req = NULL;

	if (this->request_body.is_open() == false)
	{
		std::cout << "Opening temp file\n";
		this->request_body.open(TEMP_FILE, std::ios::binary);
		if (this->request_body.fail())
			std::cerr << "\033[31mFailed to open temp file\033[0m\n";
		else if (this->request_body.good())
			std::cout << "Temp file has successfully been opened\n";
	}
	while (r > 0)
	{
		if (std::string(buffer).empty() == false)
			std::memset(buffer, 0, BUFFER_SIZE + 1);
		r = recv(client_fd, buffer, BUFFER_SIZE, 0);
		if (r < 0)
		{
			std::cerr << "Recv: -1\n";
			return (NULL);
		}
		else if (r == 0)
		{
			std::cout << "Recv returned 0\n";
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
		if (this->state == HEADER)
		{
			this->request_header.append(buffer, BUFFER_SIZE);
			if (std::string(buffer).find("\r\n\r\n") != std::string::npos)
			{
				req = new Request(this->request_header);
				if (req->getMethod() == "POST")
					std::cout << "\033[36m" << this->request_header << "\033[0m\n";
				if (req->getContentLen() != 0)
					this->state = BODY;
				else
					this->state = COMPLETE;
				if (req->getContentLen() != 0)	// message body present
				{
					size_t	body_pos = std::string(buffer).find("\r\n\r\n") + 4;
					if (!*(buffer + body_pos))
					{
						std::cout << "Nothing to write\n";
						continue ;
					}
					written += std::string(buffer + body_pos).length();
					std::string test = std::string(buffer + body_pos).substr(0, std::string::npos);
					this->request_body.write(test.c_str(), test.length());
					if (written == req->getContentLen())
						this->state = COMPLETE;
					if (this->state == BODY)
						continue ;
				}
			}
		}
		if (this->state == BODY)
		{
			std::string	body = buffer;
			// if (body.find(req->getBoundary()) != std::string::npos)
			// {
			// 	std::cout << "Erasing " << body.substr(body.find(req->getBoundary()), body.find("\r\n\r\n"));
			// 	body.erase(body.find(req->getBoundary()), body.find("\r\n\r\n"));
			// 	std::cout << "\033[31m" << body << "\033[0m\n";
			// }
			this->request_body.write(buffer, BUFFER_SIZE);
			// this->request_body.write(body.c_str(), body.length());
			if (this->request_body.bad())
				std::cerr << "\033[31mFailed\033[0m\n";
			written += r;
			std::cout << "Written " << written << " bytes to file\n";
			if (written == req->getContentLen())
				this->state = COMPLETE;
		}
		if (this->state == COMPLETE)	// request is fully parsed; server is ready to send response
		{
			std::cout << "Request has been fully received\n";
			this->state = HEADER;
			break ;
		}
	}
	(void)index;
	return (req);
}

void	ConnectionManager::parseBody()
{
	char	buffer[BUFFER_SIZE + 1] = {0};
	(void)buffer;
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
		res = poll(&sock_fds[0], sock_fds.size(), 1000);
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
	this->request_body.close();
	unlink(TEMP_FILE);
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
