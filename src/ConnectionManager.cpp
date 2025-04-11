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
			std::cout << "IP-Port-Pair: " << ipp << "\n";
			std::vector<str>::iterator	found_socket = std::find(reqs.begin(), reqs.end(), ipp);
			if (found_socket == reqs.end())
			{
				std::cout << "Adding IP-Port-Pair to map of IP-Port-Pairs\n";
				addSocket((*it)->getIP(i), (*it)->getPort(i));
				servers_per_ippp.push_back(std::map<str, Server *>());
				reqs.push_back(ipp);
				defaults.push_back(*it);
				handlers.push_back(NULL);
				addServerToMap(servers_per_ippp.back(), **it);
			}
			else
			{
				std::cout << "Adding already existing IP-Port-Pair to map\n";
				addServerToMap(servers_per_ippp.at(found_socket - reqs.begin()), **it);
			}
		}
	}
	for (unsigned int i = 0; i < reqs.size(); i++)
		reqs[i] = "";
}

/// @brief sets up the listener socket
/// @param ip the ip address
/// @param port the port number
/// @return returns the setup socket
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

/// @brief add listener socket to sock_fds vector in ConnectionManager q
/// @param ip the ip address
/// @param port the port number
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
	{
		std::cout << "Name: " << (*it) << "\n";
		map[*it] = &server;
	}
	for (std::map<Server *)
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
	if (!(*req)->isValidRequest() || servers_per_ippp.at(i).find((*req)->getHost()) == servers_per_ippp.at(i).end())
		handlers.at(i) = defaults.at(i);
	else
		handlers.at(i) = servers_per_ippp.at(i)[(*req)->getHost()];
	handlers.at(i)->handleRequest(*req);
	delete *req;
	*req = NULL;
}

void	ConnectionManager::loopForClients()
{
	for (unsigned int i = 0; i < main_listeners; i++)
	{
		if (this->sock_fds.at(i).revents & POLLIN)	// add a client into the sock_fds vector
		{
			newClient(i, this->sock_fds.at(i));
		}
	}
}

void	 ConnectionManager::listenForRequest()
{
	char request_buffer[4096] = {0};

	//for (unsigned int i = this->main_listeners; i < sock_fds.size(); i++)
	//{
	//	//std::cout << "currently at index: " << i << "\n";
	//	if (this->sock_fds.at(i).revents == 0)
	//	{
	//		std::cout << "No event occured on client\n";
	//		continue ;
	//	}
	//	else if (this->sock_fds.at(i).revents & POLLIN)
	//	{
	//		std::cout << "Socket " << i << " in POLLIN\n";
	//		unsigned int	bytes_read = 0;
	//		bytes_read = recv(this->sock_fds.at(i).fd, request_buffer, sizeof(request_buffer), 0);
	//		request_buffer[bytes_read] = 0;
	//		std::cout << "Request: " << request_buffer;
	//		if (bytes_read == 0)
	//		{
	//			std::cout << "Finished\n";
	//			reqs.at(i) = request_buffer;
	//			// remove the socket from sock_fds
	//			this->sock_fds.erase(sock_fds.begin() + i);
	//			i--;
	//			continue ;
	//		}
	//	}
	//	else if (this->sock_fds.at(i).revents & POLLOUT)	// the client is ready to write
	//	{
	//		std::cout << "Socket " << i << " in POLLOUT\n";
	//		this->sock_fds.erase(sock_fds.begin() + i);
	//		i--;
	//		//char buffer[4096] = {0};
	//		//std::string buf;
	//		//buf = "HTTP/1.1 201 OK\r\nConnection: close\r\n\r\n<html><p>Text</p></html>";
	//		//ssize_t	bytes_sent = send(this->sock_fds.at(i).fd, buf.c_str(), buf.size(), 0);
	//		//if (bytes_sent == -1)
	//		//{
	//		//	std::cerr << "Error while responding\n";
	//		//	this->sock_fds.erase(sock_fds.begin() + i);
	//		//	i--;
	//		//	return ;
	//		//}
	//		//this->sock_fds.erase(sock_fds.begin() + i);
	//		//i--;
	//		////std::cout << "Socket " << i << " in POLLOUT\n";
	//		////this->sock_fds.at(i).revents = POLLIN;
	//		////this->sock_fds.erase(sock_fds.begin() + i);
	//	}
	//}
	for (unsigned int i = this->main_listeners; i < sock_fds.size(); i++)
	{
		//std::cout << "currently at index: " << i << "\n";
		if (this->sock_fds.at(i).revents == 0)
		{
			std::cout << "No event occured on client\n";
			continue ;
		}
		else if (this->sock_fds.at(i).revents & POLLIN)
		{
			std::cout << "Socket " << i << " in POLLIN\n";
			unsigned int	bytes_read = 0;
			bytes_read = recv(this->sock_fds.at(i).fd, request_buffer, sizeof(request_buffer), 0);
			request_buffer[bytes_read] = 0;
			std::cout << "Request: " << request_buffer;
			if (bytes_read == 0)
			{
				std::cout << "Finished\n";
				reqs.at(i) = request_buffer;
				// remove the socket from sock_fds
				this->sock_fds.erase(sock_fds.begin() + i);
				i--;
				continue ;
			}
		}
		else if (this->sock_fds.at(i).revents & POLLOUT)	// the client is ready to write
		{
			std::cout << "Socket " << i << " in POLLOUT\n";
			this->sock_fds.erase(sock_fds.begin() + i);
			i--;
			//char buffer[4096] = {0};
			//std::string buf;
			//buf = "HTTP/1.1 201 OK\r\nConnection: close\r\n\r\n<html><p>Text</p></html>";
			//ssize_t	bytes_sent = send(this->sock_fds.at(i).fd, buf.c_str(), buf.size(), 0);
			//if (bytes_sent == -1)
			//{
			//	std::cerr << "Error while responding\n";
			//	this->sock_fds.erase(sock_fds.begin() + i);
			//	i--;
			//	return ;
			//}
			//this->sock_fds.erase(sock_fds.begin() + i);
			//i--;
			////std::cout << "Socket " << i << " in POLLOUT\n";
			////this->sock_fds.at(i).revents = POLLIN;
			////this->sock_fds.erase(sock_fds.begin() + i);
		}
	}
}

void ConnectionManager::startConnections()
{
	int		res;
	char	buffer[4096];
	ssize_t	bytes;
	Request	*req = NULL;

	main_listeners = sock_fds.size();
	std::cout << "There are " << main_listeners << " listeners\n";
	signal(SIGINT, sigint_handle);
	while (g_quit != true)
	{
	 	res = poll(&sock_fds[0], sock_fds.size(), 2000);
		if (res == 0)
		{
			std::cout << "waiting\n";
			continue ;
		}
		if (res < 0)
		{
			if (g_quit)
			{
				std::cout << "quitting\n";
				break ;
			}
			perror("poll");
			throw std::runtime_error("e2");
		}
		loopForClients();
		listenForRequest();
		//for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		//{
		//	if (sock_fds.at(i).revents == 0)
		//	{
		//		std::cout << "no event\n";
		//		continue ;
		//	}
		//	else if (sock_fds.at(i).revents & POLLIN)
		//	{
		//		std::cout << "in pollin\n";
		//		reqs.at(i) = "";
		//		memset(buffer, 0, sizeof(buffer));
		//		bytes = recv(sock_fds.at(i).fd, buffer, sizeof(buffer), 0);
		//		reqs.at(i) = str(buffer);
		//		if (bytes == 0)
		//		{
		//			close(sock_fds.at(i).fd);
		//			sock_fds.erase(sock_fds.begin() + i);
		//			reqs.erase(reqs.begin() + i);
		//			handlers.erase(handlers.begin() + i);
		//			defaults.erase(defaults.begin() + i);
		//			servers_per_ippp.erase(servers_per_ippp.begin() + i);
		//			i--;
		//			continue ;
		//		}
		//		req = new Request(reqs.at(i));
		//		passRequestToServer(i, &req);
		//		sock_fds.at(i).revents = 0;
		//	}
		//	else if (sock_fds.at(i).revents & POLLOUT)
		//	{
		//		std::cout << "In POLLOUT for cient\n";
		//		if (handlers.at(i))
		//		{
		//			if (handlers.at(i)->respond(sock_fds.at(i).fd))
		//			{
		//				close(sock_fds.at(i).fd);
		//				sock_fds.erase(sock_fds.begin() + i);
		//				reqs.erase(reqs.begin() + i);
		//				handlers.erase(handlers.begin() + i);
		//				defaults.erase(defaults.begin() + i);
		//				servers_per_ippp.erase(servers_per_ippp.begin() + i);
		//				i--;
		//			}
		//		}
		//		else
		//		{
		//			close(sock_fds.at(i).fd);
		//			sock_fds.erase(sock_fds.begin() + i);
		//			reqs.erase(reqs.begin() + i);
		//			handlers.erase(handlers.begin() + i);
		//			defaults.erase(defaults.begin() + i);
		//			servers_per_ippp.erase(servers_per_ippp.begin() + i);
		//			i--;
		//		}
		//	}
		//	else
		//	{
		//		printError(sock_fds.at(i).revents);
		//		close(sock_fds.at(i).fd);
		//		sock_fds.erase(sock_fds.begin() + i);
		//		reqs.erase(reqs.begin() + i);
		//		handlers.erase(handlers.begin() + i);
		//		defaults.erase(defaults.begin() + i);
		//		servers_per_ippp.erase(servers_per_ippp.begin() + i);
		//		i--;
		//	}
		//}
	}
	for (unsigned int i = 0; i < sock_fds.size(); i++)
	{
		std::cout << "closing socket\n";
		close(sock_fds.at(i).fd);
		sock_fds.pop_back();
		reqs.pop_back();
		handlers.pop_back();
		defaults.pop_back();
		servers_per_ippp.pop_back();
	}
	signal(SIGINT, SIG_DFL);
}
