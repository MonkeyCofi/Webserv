/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/09/01 12:18:29 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "ConnectionManager.hpp"
#include "Cgi.hpp"

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
				states.push_back(INVALID);
				addServerToMap(servers_per_ippp.back(), **it);
			}
			else
				addServerToMap(servers_per_ippp.at(found_socket - reqs.begin()), **it);
		}
	}
	for (unsigned int i = 0; i < reqs.size(); i++)
		reqs[i] = "";
	this->cgi_pipes[0] = this->cgi_pipes[1] = -1;
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
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		throw (std::runtime_error("setsockopt error"));
	}
	if (bind(fd, (sockaddr *)&ret, sizeof(ret)) < 0)
	{
		freeaddrinfo(info);
		const str	e = "Error! " + ip + ":" + port + ": " + strerror(errno);
		throw (std::runtime_error(e));
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
	{
		perror("accept");
		return ;
	}
	client.fd = acc_sock;
	fcntl(client.fd, F_SETFL, fcntl(client.fd, F_GETFL) | O_NONBLOCK);
	fcntl(client.fd, F_SETFD, fcntl(client.fd, F_GETFD) | FD_CLOEXEC);
	client.events = POLLIN;
	client.revents = 0;
	sock_fds.push_back(client);
	handlers.push_back(NULL);
	defaults.push_back(defaults.at(i));
	servers_per_ippp.push_back(std::map<str, Server *>(servers_per_ippp.at(i)));
	states.push_back(INCOMPLETE);
}

void	ConnectionManager::addFdtoPoll(unsigned int i, struct pollfd fd)
{
	this->sock_fds.push_back(fd);
	handlers.push_back(NULL);
	defaults.push_back(defaults.at(i));
	servers_per_ippp.push_back(std::map<str, Server *>(servers_per_ippp.at(i)));
	states.push_back(INVALID);
}

// if cgi is requested, don't erase the request for the client_fd
void ConnectionManager::passRequestToServer(unsigned int& i, Request **req)
{
	if (!(*req)->isValidRequest())
		handlers.at(i) = defaults.at(i);
	if (servers_per_ippp.at(i).find((*req)->getHost()) == servers_per_ippp.at(i).end())
		handlers.at(i) = defaults.at(i);
	else if((*req)->isValidRequest())
		handlers.at(i) = servers_per_ippp.at(i).at((*req)->getHost());
	handlers.at(i)->handleRequest(i, sock_fds.at(i).fd, *req, *this);
}

void	ConnectionManager::deleteRequest(unsigned int i)
{
	Request*	temp = this->requests[sock_fds[i].fd];
	this->requests.erase(sock_fds[i].fd);
	delete temp;
}

void	ConnectionManager::closeSocketNoRef(unsigned int index)
{
	close(sock_fds.at(index).fd);
	if (requests.find(sock_fds.at(index).fd) != requests.end())
		delete requests[(sock_fds.at(index).fd)];
	sock_fds.erase(sock_fds.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	states.erase(states.begin() + index);
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	close(sock_fds.at(index).fd);
	if (requests.find(sock_fds.at(index).fd) == requests.end())
		;
	else
	{
		deleteRequest(index);
	}
	sock_fds.erase(sock_fds.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	states.erase(states.begin() + index);
	index--;
}

int	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
{
	std::string	_request;
	ssize_t		r;
	size_t		barbenindex;
	int			outcome;
	char		buffer[BUFFER_SIZE + 1] = {0};

	// if the cient fd is a cgi pipe, then we don't want to receive a request from it
	if (cgiProcesses.find(client_fd) != cgiProcesses.end())
	{
		state = IGNORE;
		return (-1);
	}
	if (!req)
	{
		state = IGNORE;
		return (-1);
	}
	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r < 0)
	{
		state = INCOMPLETE;
		return (0);
	}
	if (r == 0)
	{
		closeSocket(index);
		return (-1);
	}
	// 1- Have a function like pushRequest that handles body parts (pushBody). For example discards the boundaries and body fields and saves the files.
	// 2- In order to save the file I need to open an FD and put it in the array with the others to be POLLed
	if (req->getHeaderReceived())
	{
		req->addReceivedBytes(r);
		if (req->isCGI())	// write receive bytes to the CGI_PIPE'S write end
		{
			Cgi* cgi = req->getCgiObj();
			bool finished = cgi->writeToFd(cgi->get_stdin()[WRITE], const_cast<const char *>(buffer), r, req);	// this function will write to the fd and will close it if its done writing
			(void)finished;
			if (finished)
			{	
				for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
				{
					if (it->fd == client_fd)
					{
						it->events |= POLLOUT;	// add pollout to the cgi client fd
						break ;
					}
				}
				if (req->shouldKeepAlive() != true)
					closeSocket(index);
				return (FINISH);
			}
		}
	}
	else
	{
		buffer[r] = 0;
		_request = str(buffer);
		std::string leftover_str(buffer);
		outcome = req->pushRequest(leftover_str);	// because pushRequest modifies the reference string passed to be the leftovers
		switch (outcome)
		{
			case 400:
				req->setStatus("400");
				// fallthrough
			case 502:
				if (outcome == 501)
					req->setStatus("501");
				req->setValid(false);
				state = INVALID;
				return -2;
			case 1:
				if (!req->isValidRequest())
				{
					state = INVALID;
					return -2;
				}
				else
				{
					if (servers_per_ippp.at(index).find(req->getHost()) == servers_per_ippp.at(index).end())
						handlers.at(index) = defaults.at(index);
					else if(req->isValidRequest())
						handlers.at(index) = servers_per_ippp[index][req->getHost()];
					if (handlers.at(index)->checkRequestValidity(req) == false)
					{
						req->setStatus(req->getStatus() != "405" && (req->getContentLen() > handlers.at(index)->getMaxBodySize()) ? "413" : "405");
						req->setValid(false);
						state = INVALID;
						return -2;
					}
				}
				barbenindex = (_request.find("\r\n\r\n") != std::string::npos ? _request.find("\r\n\r\n") + 4 : std::string::npos);
				if ((ssize_t)barbenindex < r) // move leftovers to beginning of buffer
				{
					r = r - barbenindex;
					req->addReceivedBytes(r);
					std::memmove(buffer, buffer + barbenindex, r + 1);
					if (!req->isCGI())
						break ;
					req->setLeftOvers(buffer, r);	// add leftovers to the request object
				}
				if (req->getContentLen() != 0)
				{
					if (req->getReceivedBytes() == req->getContentLen())
					{
						state = FINISH;
						return (1);
					}
					state = (req->getHeaderReceived() ? HEADER_FINISHED : INCOMPLETE);
					return (req->getHeaderReceived() ? 2 : 0);
				}
				state = FINISH;
				return (1);
			default:
				state = INCOMPLETE;
				return (0);
		}
	}
	if (req->getHeaderReceived() && req->getMethod() == "POST" && !req->isCGI() && r > 0)
	{
		if (servers_per_ippp.at(index).find(req->getHost()) == servers_per_ippp.at(index).end())
			handlers.at(index) = defaults.at(index);
		else if(req->isValidRequest())
			handlers.at(index) = servers_per_ippp[index][req->getHost()];
		if (handlers.at(index)->checkRequestValidity(req) == false)
		{
			req->setStatus((req->getContentLen() > handlers.at(index)->getMaxBodySize()) ? "501" : "405");
			req->setValid(false);
			state = INVALID;
			return (-2);
		}
		outcome = req->fileUpload(handlers.at(index), handlers.at(index)->matchLocation(req->getFileURI()), buffer, r);
		switch(outcome)
		{
			case -2:
				req->setStatus("500");
				req->setValid(false);
				state = INVALID;
				return (-2);
			case -1:
				req->setStatus("413");
				req->setValid(false);
				state = INVALID;
				return (-2);
			case 0:
				state = INCOMPLETE;
				return (0);
			case 1:
				state = FINISH;
				return (1);
			default:
				state = INCOMPLETE;
				return (0);
		}
	}
	state = FINISH;
	return (1);
}

/// @brief handles all pollout events
/// @param state the current state of the response
/// @param i the index of the handling server
/// @param requests the map of requests wherein the socket fd is the key and the request object is the value
void	ConnectionManager::handlePollout(State& state, unsigned int& i, std::map<int, Request *> &requests)
{
	Server* handler = handlers[i];
	if (handler && state == FINISH)	// the request has been parsed and ready for response building
	{
		bool keep_open = handler->respond(sock_fds[i].fd);
		if (keep_open && handler->getState() == Server::returnFinish())
		{
			sock_fds.at(i).events &= ~POLLOUT;	// remove POLLOUT event from the FD
			sock_fds.at(i).events |= POLLIN;
			if (requests.find(sock_fds[i].fd) != requests.end())
			{
				Request* temp = requests[sock_fds[i].fd];
				requests.erase(sock_fds[i].fd);
				delete temp;
			}
		}
		else if (keep_open == false && handler->getState() == Server::returnFinish())
			closeSocket(i);
	}
	else if (state == INVALID)
	{
		handler->respond(sock_fds[i].fd);
		sock_fds[i].events |= POLLIN;
		sock_fds[i].events &= ~POLLOUT;
		handler->setState(Server::returnIncomplete());
	}
	handler->setState(Server::returnIncomplete());
	(void)state;
}

bool	ConnectionManager::handleCGIPollout(unsigned int& i)
{
	CGIinfo* infoPtr = NULL;
	int	pipe_fd = -1;

	for (std::map<int, CGIinfo>::iterator it = cgiProcesses.begin(); it != cgiProcesses.end(); it++)
	{
		// if the client fd is present in the map and finished reading the response
		if (it->second.getClientFd() == sock_fds.at(i).fd && it->second.isComplete())
		{
			infoPtr = &it->second;
			pipe_fd = it->first;
			break ;
		}
	}

	if (infoPtr == NULL)
		return (false);

	Server* handler = handlers[i];
	bool keep_alive = handler->cgiRespond(infoPtr);
	if (infoPtr->getResponseStatus() == true) // response has been fully sent
	{
		// don't close cgi client, but rather the client's cgi fd
		if (!keep_alive)
			closeSocket(i);
		if (pipe_fd > -1)
			cgiProcesses.erase(pipe_fd);
		handler->setState(Server::returnIncomplete());
	}
	return (true);
}

void	ConnectionManager::handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests)
{
	std::map<int, Request*>::iterator it = requests.find(sock_fds.at(i).fd);
	if (it == requests.end())
		requests.insert(std::pair<int, Request*>(sock_fds.at(i).fd, new Request()));
	int ret = receiveRequest(sock_fds.at(i).fd, requests.at(sock_fds.at(i).fd), i, state);	// request has been fully received
	if (ret == -1)
		return ;
	if (state == INVALID)
	{
		this->sock_fds[i].events &= ~POLLIN;	// removing POLLIN; unsure why
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
	}
	if (state == FINISH || (state == HEADER_FINISHED && requests.at(sock_fds.at(i).fd)->isCGI()))	// HEADER_FINISHED indicates partial request
		this->passRequestToServer(i, &requests.at(sock_fds.at(i).fd));
}

void	ConnectionManager::handleCGIread(unsigned int& i)
{
	ssize_t r;
	char	buffer[BUFFER_SIZE + 1] = {0};
	CGIinfo& cgi_info = cgiProcesses.at(sock_fds.at(i).fd);

	r = read(sock_fds.at(i).fd, buffer, BUFFER_SIZE);
	if (r == -1)	// should be handled
	{
		return ;
	}
	else if (r == 0)
	{
		int	status;

		pid_t	res = waitpid(cgi_info.getPid(), &status, WNOHANG);
		if (res == cgi_info.getPid())
		{
			// the cgi pid can safely be cleaned up
		}	// else cleanup will happen later
		cgi_info.completeResponse();	// once read returns 0, that means its all good to respond to

		// in the pollfds vector, find the client fd's index and set its event field to fd.event |= POLLOUT
		const int	client_fd = cgi_info.getClientFd();
		for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
		{
			if (it->fd == client_fd)
			{
				it->events |= POLLOUT;	// add pollout to the cgi client fd
				break ;
			}
		}
		closeSocket(i);
		(void)status;
	}
	else
	{
		buffer[r] = 0;
		cgi_info.concatBuffer(std::string(buffer));
	}
}

void	ConnectionManager::handleCgiPollhup(unsigned int& i)
{
	CGIinfo& info = cgiProcesses[sock_fds.at(i).fd];
	info.completeResponse();

	const int& client_fd = info.getClientFd();
	// find the cgi object's client fd
	for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
	{
		if (it->fd == client_fd)
		{
			it->events |= POLLOUT;	// set the client fd's event to POLLOUT
			break ;
		}
	}
}

void ConnectionManager::startConnections()
{
	int						res;

	main_listeners = sock_fds.size();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handle);
	std::cout << "Server has " << main_listeners << " listeners\n";
	while (g_quit != true)
	{
		res = poll(&sock_fds[0], sock_fds.size(), 100);
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
			}
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					handleCGIread(i);
					continue ;
				}
				else
				{
					handlePollin(i, states.at(i), requests);
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handleCGIPollout(i) == true)
					continue ;
				handlePollout(states.at(i), i, requests);
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					handleCgiPollhup(i);
					closeSocket(i);
				}
				else
					closeSocket(i);
			}
		}
	}
	std::cout << "Ending Server...\n";
	this->deleteTempFiles();
	this->request_body.close();
	for (unsigned int i = 0; i < sock_fds.size(); i++)
		closeSocket(i);
	sock_fds.clear();
	std::cout << "Server closed!\n";
	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
}

void	ConnectionManager::deleteTempFiles()
{
	for (std::vector<std::string>::iterator it = this->tempFileNames.begin(); it != this->tempFileNames.end(); it++)
	{
		// if file exists
		if (!access((*it).c_str(), F_OK))
		{
			// remove it
			std::remove((*it).c_str());
		}
	}
}

std::vector<struct pollfd>& ConnectionManager::getPollFds()
{
	return (this->sock_fds);
}

std::map<int, CGIinfo>& ConnectionManager::getCgiProcesses()
{
	return (this->cgiProcesses);
}
