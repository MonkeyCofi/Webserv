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

unsigned int	ConnectionManager::number = 0;
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
	if (!(*req)->isValidRequest() || servers_per_ippp.at(i).find((*req)->getHost()) == servers_per_ippp.at(i).end())
		handlers.at(i) = defaults.at(i);
	else
		handlers.at(i) = servers_per_ippp.at(i)[(*req)->getHost()];
	handlers.at(i)->handleRequest(*req);
	delete *req;
	*req = NULL;
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	close(sock_fds.at(index).fd);
	sock_fds.erase(sock_fds.begin() + index);
	reqs.erase(reqs.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	index--;
}

ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index)
{
	char	buffer[BUFFER_SIZE + 1];
	ssize_t	r;
	std::string	_request;

	if (req == NULL)
	{
		std::cerr << "No Request\n";
		throw std::exception();
	}
	else
		std::cout << "Request lies in address " << req << "\n";
	std::memset(buffer, 0, BUFFER_SIZE + 1);
	r = 1;
	// while (r > 0)
	// {
		if (std::string(buffer).empty() == false)
			std::memset(buffer, 0, BUFFER_SIZE + 1);
		r = recv(client_fd, buffer, BUFFER_SIZE, 0);
		if (r < 0)
		{
			// closeSocket(index);
			std::cout << "\033[31mRecv returning -1\033[0m\n";
			return (INCOMPLETE);
		}
		else if (!r)	//client closed the conneciton
		{
			closeSocket(index);
			std::cout << "Returning FINISH because recv returned 0\n";
			return ((req->getHeaderReceived() == true) ? FINISH : INCOMPLETE);
		}
		else	// recv received data
		{
			_request = buffer;
			req->pushRequest(_request);
			if (_request.find("\r\n\r\n") != std::string::npos)	// the request header has fully been received
			{
				if (req->getHasBody() == true)
				{
					std::fstream&	file = req->getBodyFile();
					std::cout << "Has body\n";
					if (file.is_open() == false)	// open temp file for reading message body
					{
						str	filename = TEMP_FILE;
						filename += static_cast<char>((ConnectionManager::number++ % 10) + '0');
						std::cout << "Trying to open with filename " << filename << "\n";
						file.open(filename.c_str(), std::ios::binary | std::ios::out);
						if (file.fail())
						{
							perror("Open");
							throw (std::runtime_error("Couldn't open temp file\n"));
						}
						else if (file.good())
							std::cout << "Successfully opened temp file for body\n";
					}
					file.write(buffer, r);
					if (file.bad())
						throw(std::runtime_error("Couldn't write data to temp file\n"));
					std::string	buffer_str = buffer;
					if (buffer_str.find(req->getBoundary() + "--") != std::string::npos)	// found the end of the request body
					{
						req->setFullyReceived(true);
						return (FINISH);
					}
				}
				else
				{
					req->setHeaderReceived(true);	// set header received to true
					str	rq = req->getRequest();
					req->parseRequest(rq);	// parse the header of the request
					if (req->getContentLen() != 0)
					{
						std::cout << "There is a body\n";
						req->setHasBody(true);
					}
					else
					{
						req->setFullyReceived(true);
						std::cout << "There is no body\n";
						return (FINISH);	// this means no body, therefore request is fully received
					}
				}
			}
		}
	// }
	return (INCOMPLETE);
}

// Request*	ConnectionManager::receiveRequest(int client_fd, unsigned int& index)
// {
// 	char	buffer[BUFFER_SIZE + 1];
// 	ssize_t	r;
// 	ssize_t	bytes_read;
// 	size_t	written = 0;
// 	Request	*req;

// 	std::memset(buffer, 0, BUFFER_SIZE + 1);
// 	r = 1;
// 	bytes_read = 0;
// 	this->request_header = "";
// 	req = NULL;
// 	if (this->request_body.is_open() == false)
// 	{
// 		std::cout << "Opening temp file\n";
// 		this->request_body.open(TEMP_FILE, std::ios::out | std::ios::binary);
// 		if (this->request_body.fail())
// 			std::cerr << "\033[31mFailed to open temp file\033[0m\n";
// 		else if (this->request_body.good())
// 			std::cout << "Temp file has successfully been opened\n";
// 	}
// 	while (r > 0)
// 	{
// 		if (std::string(buffer).empty() == false)
// 			std::memset(buffer, 0, BUFFER_SIZE + 1);
// 		r = recv(client_fd, buffer, BUFFER_SIZE, 0);
// 		if (r < 0)	// either there is no more data to read, or error happened on recv
// 		{
// 			std::cerr << "Recv: -1\n";
// 			perror("Why");
// 			closeSocket(index);
// 			break ;
// 		}
// 		else if (r == 0)	// client closed the connection
// 		{
// 			std::cout << "Recv returned 0\n";
// 			closeSocket(index);
// 			req = new Request(this->request_header);
// 			std::cout << this->request_header << "\n";
// 			this->state = COMPLETE;
// 			// return (new Request(this->request_header));
// 		}
// 		bytes_read += r;
// 		if (this->state == HEADER)
// 		{
// 			this->request_header.append(buffer, r);
// 			if (std::string(buffer).find("\r\n\r\n") != std::string::npos)
// 			{
// 				req = new Request(this->request_header);
// 				if (req->getContentLen() != 0)
// 					this->state = BODY;
// 				else
// 					this->state = COMPLETE;
// 				if (req->getContentLen() != 0 && this->state == BODY)	// message body present
// 				{
// 					size_t	body_pos = std::string(buffer).find("\r\n\r\n") + 4;
// 					std::string	body = buffer + body_pos;
// 					std::cout << "\033[36m" << body << "\033[0m\n";
// 					written += std::string(buffer + body_pos).length();
// 					std::cout << body.c_str() << "\n";
// 					this->request_body.write(body.c_str(), body.length());
// 					if (this->request_body.fail())
// 						std::cout << "\033[31mFailed\033[0m\n";
// 					else if (this->request_body.good())
// 						std::cout << "Good\n";
// 					if (written == req->getContentLen())
// 						this->state = COMPLETE;
// 					if (this->state == BODY)
// 						continue ;
// 				}
// 			}
// 		}
// 		if (this->state == BODY)
// 		{
// 			this->request_body.write(buffer, r);
// 			if (this->request_body.fail())
// 				std::cout << "\033[31mFailed\033[0m\n";
// 			written += r;
// 			if (req && written == req->getContentLen())
// 				this->state = COMPLETE;
// 		}
// 		if (this->state == COMPLETE)	// request is fully parsed; server is ready to send response
// 		{
// 			std::cout << "Request has been fully received\n";
// 			if (req->getBoundary() != "")	// message body present
// 			{
// 				std::cout << "There is a body\n";
// 				std::cout << this->request_header << "\n";
// 				parseBody(req);
// 			}
// 			else
// 			{
// 				if (req->getMethod() == "POST")
// 					std::cout << this->request_header << "\n";
// 				std::cout << "There is no body boundary\n";
// 			}
// 			this->state = HEADER;
// 			break ;
// 		}
// 	}
// 	(void)index;
// 	(void)to_skip;
// 	this->request_body.close();
// 	return (req);
// }

void	ConnectionManager::parseBody(Request* req)
{
	if (!req)	{std::cout << "returning\n"; return ;}
	std::ifstream				inStream;
	const std::string			endBoundary(req->getBoundary() + "--");
	std::string					line;

	std::cout << "in here\n";
	inStream.open(TEMP_FILE, std::ios::binary);
	while (std::getline(this->request_body, line))
	{
		if (line.find("filename=") != std::string::npos)
		{
			// open the file
			std::string	fileName = line.substr(line.find("filename=") + std::strlen("filename="), std::string::npos);
			std::cout << "Opneing " << fileName << "\n";
			std::ofstream	file;
			file.open(std::string("." + fileName).c_str(), std::ios::binary);
			if (file.fail())
				std::cout << "Failed to open\n";
		}
	}
}

// void ConnectionManager::startConnections()
// {
// 	int		res;
// 	std::vector<Request *>	requests;
// 	Request	*req = NULL;
	
// 	main_listeners = sock_fds.size();
// 	signal(SIGPIPE, SIG_IGN);
// 	signal(SIGINT, sigint_handle);
// 	while (g_quit != true)
// 	{
// 		res = poll(&sock_fds[0], sock_fds.size(), 500);
// 		if (res == 0)
// 			continue ;
// 		if (res < 0)
// 		{
// 			if (g_quit)
// 				break ;
// 			perror("Poll");
// 			throw std::runtime_error("unexpected error in poll function");
// 		}
// 		for (unsigned int i = 0; i < main_listeners; i++)
// 		{
// 			if (sock_fds.at(i).revents & POLLIN)
// 			{
// 				newClient(i, sock_fds.at(i));
// 				requests.push_back(NULL);	// push back a NULL pointer to requests vector when a client is created
// 			}
// 		}
// 		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
// 		{
// 			if (sock_fds.at(i).revents == 0)
// 				continue ;
// 			if (sock_fds.at(i).revents & POLLIN)
// 			{
// 				std::cout << "num: " << main_listeners - i << "\n";
// 				requests.at(main_listeners - i) = receiveRequest(sock_fds.at(i).fd, i);
// 				if (requests.at(main_listeners - i)->isFullyReceived() == true)
// 					this->passRequestToServer(i, &req);
// 				// req = receiveRequest(sock_fds.at(i).fd, i);
// 				// if (req)
// 				// 	this->passRequestToServer(i, &req);
// 				// else
// 				// 	continue ;
// 			}
// 			if (sock_fds.at(i).revents & POLLOUT)
// 			{
// 				req = NULL;
// 				if (handlers.at(i))
// 				{
// 					std::cout << "Responding to request\n";
// 					if (handlers.at(i)->respond(sock_fds.at(i).fd))
// 						closeSocket(i);
// 				}
// 			}
// 			if (sock_fds.at(i).revents & POLLHUP)
// 			{
// 				printError(sock_fds.at(i).revents);
// 				closeSocket(i);
// 			}
// 		}
// 	}
// 	std::cout << "Ending Server...\n";
// 	this->request_body.close();
// 	unlink(TEMP_FILE);
// 	for (unsigned int i = 0; i < sock_fds.size(); i++)
// 	{
// 		close(sock_fds.at(i).fd);
// 		sock_fds.pop_back();
// 		reqs.pop_back();
// 		handlers.pop_back();
// 		defaults.pop_back();
// 		servers_per_ippp.pop_back();
// 	}
// 	std::cout << "Server closed!\n";
// 	signal(SIGINT, SIG_DFL);
// 	signal(SIGPIPE, SIG_DFL);
// }
void ConnectionManager::startConnections()
{
	int							res;
	State						state = INCOMPLETE;
	std::map<int, Request *>	requests;
	
	main_listeners = sock_fds.size();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handle);
	std::cout << "Server has " << main_listeners << " listeners\n";
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
			{
				newClient(i, sock_fds.at(i));
				std::cout << "Pushing a new request and a client\n";
				requests[sock_fds.at(i).fd] = NULL;
			}
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				if (requests[sock_fds.at(i).fd] == NULL)
					requests[sock_fds.at(i).fd] = new Request();
				if ((state = receiveRequest(sock_fds.at(i).fd, requests[sock_fds.at(i).fd], i)) == FINISH)	// request has been fully received
					this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handlers.at(i) && state == FINISH)
				{
					std::cout << "Responding to request\n";
					if (handlers.at(i)->respond(sock_fds.at(i).fd))
					{
						std::cout << "Removing request from vector\n";
						requests.erase(sock_fds.at(i).fd);
						// requests.erase(requests.begin() + (test));
						closeSocket(i);
					}
				}
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				printError(sock_fds.at(i).revents);
				closeSocket(i);
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
