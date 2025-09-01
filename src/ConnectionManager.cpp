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
	std::cout << "Pushing back client fd " << client.fd << " at index " << sock_fds.size() << "\n";
	sock_fds.push_back(client);
	// reqs.push_back("");
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

void ConnectionManager::printError(int revents)
{
	if (revents & POLLHUP)
		std::cerr << "Hangup\n";
	else if (revents & POLLERR)
		std::cerr << "Error\n";
	else if (revents & POLLNVAL)
		std::cerr << "INVALID\n";
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
	// sock_fds.at(i).events &= ~POLLIN;
	handlers.at(i)->handleRequest(i, sock_fds.at(i).fd, *req, *this);
}

void	ConnectionManager::deleteRequest(unsigned int i)
{
	std::cout << RED << "Delete request called" << NL;
	Request*	temp = this->requests[sock_fds[i].fd];
	this->requests.erase(sock_fds[i].fd);
	delete temp;
}

void	ConnectionManager::closeSocketNoRef(unsigned int index)
{
	std::cout << "\033[31mClosing fd " << sock_fds.at(index).fd << "\033[0m\n";
	close(sock_fds.at(index).fd);
	if (requests.find(sock_fds.at(index).fd) == requests.end())
		std::cout << "Request for fd " << sock_fds.at(index).fd << " is not found\n";
	else
	{
		std::cout << "Request for fd " << sock_fds.at(index).fd << " exists and will now be deleted\n";
		delete requests[(sock_fds.at(index).fd)];
	}
	sock_fds.erase(sock_fds.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	states.erase(states.begin() + index);
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	std::cout << "\033[31mClosing fd " << sock_fds.at(index).fd << "\033[0m\n";
	close(sock_fds.at(index).fd);
	if (requests.find(sock_fds.at(index).fd) == requests.end())
		std::cout << "Request for fd " << sock_fds.at(index).fd << " is not found\n";
	else
	{
		std::cout << "Request for fd " << sock_fds.at(index).fd << " exists and will now be deleted\n";
		deleteRequest(index);
		// delete requests[(sock_fds.at(index).fd)];
	}
	std::cout << RED << "closing at index: " << index << NL;
	sock_fds.erase(sock_fds.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	states.erase(states.begin() + index);
	index--;
}

/*
	this function is called in handlePollin
	it returns a state which is either: INVALID, INCOMPLETE, HEADER_FINISHED, and FINISHED
	INVALID typically means its an invalid request
	INCOMPLETE means the request was partially received and there is still more to receive
	HEADER_FINISHED means that the header has been received and parsed, so it should now be passed to the server. This is only possible if POST request
	FINISHED means that the request has been fully received, with its headers parsed, and it is now ready to be sent to its appropriate handler
*/

void ConnectionManager::debugVectorSizes(const std::string& location)
{
    std::cerr << "=== Vector sizes at " << location << " ===\n";
    std::cerr << "sock_fds.size(): " << sock_fds.size() << "\n";
    std::cerr << "handlers.size(): " << handlers.size() << "\n";
    std::cerr << "defaults.size(): " << defaults.size() << "\n";
    std::cerr << "servers_per_ippp.size(): " << servers_per_ippp.size() << "\n";
    std::cerr << "states.size(): " << states.size() << "\n";
    std::cerr << "main_listeners: " << main_listeners << "\n";
    std::cerr << "==========================================\n";
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
		// std::cout << "Ignoring fd " << client_fd << " because it is a CGI pipe\n";
		state = IGNORE;
		return (-1);
	}
	// std::cout << "In receive request\n";
	if (!req)
	{
		// std::cerr << "REQUEST DONT EXIST\n";
		state = IGNORE;
		return (-1);
	}
	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r < 0)
	{
		// sock_fds.at(index).events &= ~POLLIN;
		state = INCOMPLETE;
		return (0);
	}
	if (r == 0)
	{
		std::cerr << "Closing fd " << sock_fds.at(index).fd << " in receive request due to recv() returning 0\n";
		std::cout << "Ignoring fd " << this->sock_fds[index].fd << " due to recv() returning 0\n";
		closeSocket(index);
		// state = IGNORE;
		return (-1);
	}
	// 1- Have a function like pushRequest that handles body parts (pushBody). For example discards the boundaries and body fields and saves the files.
	// 2- In order to save the file I need to open an FD and put it in the array with the others to be POLLed
	if (req->getHeaderReceived())
	{
		// std::cout << "heree\n";
		req->addReceivedBytes(r);
		if (req->isCGI())	// write receive bytes to the CGI_PIPE'S write end
		{
			// std::cout << "in here writing\n";
			Cgi* cgi = req->getCgiObj();
			std::cout << "something unique " << buffer << " melon\n";
			bool finished = cgi->writeToFd(cgi->get_stdin()[WRITE], const_cast<const char *>(buffer), r, req);	// this function will write to the fd and will close it if its done writing
			(void)finished;
			if (finished)
			{
				std::cout << "FINISHED\n";
				
				for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
				{
					if (it->fd == client_fd)
					{
						// it->events &= ~POLLIN;
						it->events |= POLLOUT;	// add pollout to the cgi client fd
						break ;
					}
				}
				if (req->shouldKeepAlive() != true)
				{
					std::cout << "Closing socket\n";
					closeSocket(index);
				}
				return (FINISH);
			}
		}
	}
	else
	{
		buffer[r] = 0;
		_request = str(buffer);
		std::string leftover_str(buffer);
		// outcome = req->pushRequest(_request);
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
				std::cout << "Invalid here\n";
				state = INVALID;
				return -2;
			case 1:
				if (!req->isValidRequest())
				{
					state = INVALID;
					std::cout << "INVALID HERE\n";
					return -2;
				}
				else
				{
					// debugVectorSizes("receiveRequest");
					// std::cout << "trying to access " << index << "th handler\n";
					if (servers_per_ippp.at(index).find(req->getHost()) == servers_per_ippp.at(index).end())
						handlers.at(index) = defaults.at(index);
					else if(req->isValidRequest())
						handlers.at(index) = servers_per_ippp[index][req->getHost()];
					if (handlers.at(index)->checkRequestValidity(req) == false)
					{
						std::cout << "Current request status: " << req->getStatus() << "\n";
						req->setStatus(req->getStatus() != "405" && (req->getContentLen() > handlers.at(index)->getMaxBodySize()) ? "413" : "405");
						req->setValid(false);
						std::cout << "Invalid here5\n";
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
					std::cerr << "Partially received body bytes\n";
					state = (req->getHeaderReceived() ? HEADER_FINISHED : INCOMPLETE);
					return (req->getHeaderReceived() ? 2 : 0);
				}
				std::cerr << "There is no body for this request\n";
				state = FINISH;
				return (1);
				// The header is fully received, the leftovers are saved in _request
				// 1) Regular POST: Start sifting through the current and following _request strings for files to be saved as upload
				// 2) CGI: Save this current leftover in whatever server will respond (how?), and after sending it to the subprocess immediately send following _request to the stdin of the subprocess
			default:
				std::cerr << "Incomplete request\n";
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
			std::cout << "Invalid here2\n";
			state = INVALID;
			return (-2);
		}
		// std::cout << "inside here\n";
		// outcome = req->pushBody(buffer, r);
		outcome = req->fileUpload(handlers.at(index)->matchLocation(req->getFileURI()), buffer, r);
		// outcome = 1;
		switch(outcome)
		{
			case -1:
				// std::cout << "Returning invalid\n";
				req->setStatus("413");
				req->setValid(false);
				state = INVALID;
				return (-2);
			case 0:
				// std::cout << "Returning incomplete\n";
				state = INCOMPLETE;
				return (0);
			case 1:
				// std::cout << "Returning finished\n";
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
	// std::cout << "in pollout function\n";
	if (handler && state == FINISH)	// the request has been parsed and ready for response building
	{
		// std::cout << YELLOW << "in handlePollout FINISH" << NL;
		bool keep_open = handler->respond(sock_fds[i].fd);
		if (keep_open && handler->getState() == Server::returnFinish())
		{
			sock_fds.at(i).events &= ~POLLOUT;	// remove POLLOUT event from the FD
			sock_fds.at(i).events |= POLLIN;
			std::cerr << "Finished responding to request\n";
			if (requests.find(sock_fds[i].fd) != requests.end())
			{
				std::cout << "\033[31mRemoving request from map\033[0m\n";
				Request* temp = requests[sock_fds[i].fd];
				requests.erase(sock_fds[i].fd);
				delete temp;
			}
		}
		else if (keep_open == false && handler->getState() == Server::returnFinish())
		{
			std::cout << "Closing client socket fd " << sock_fds[i].fd << " because connection is NOT keep-alive\n";
			closeSocket(i);
		}
	}
	else if (state == INVALID)
	{
		std::cout << "responding invalid\n";
		sock_fds[i].events &= ~POLLOUT;
		handler->respond(sock_fds[i].fd);
		sock_fds[i].events |= POLLIN;
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
	std::cout << "in CGI pollout\n";
	/* 
		the handler is the server object that would contain the response map where: [key] = client_fd, 
		[value] = response object once there is a client fd that is ready for pollout and it is 
		a client of a cgi request, respond to the request once the client has been responded to, 
		remove the cgi object from the ma of cgi processes
	*/
	Server* handler = handlers[i];
	std::cout << handler << "fwfwfewfwefwe\n";
	bool keep_alive = handler->cgiRespond(infoPtr);
	if (infoPtr->getResponseStatus() == true) // response has been fully sent
	{
		std::cout << "Done responding to cgi client: " << this->sock_fds.at(i).fd << "\n";
		// don't close cgi client, but rather the client's cgi fd
		if (!keep_alive)
		{
			std::cout << "Closing client socket fd " << this->sock_fds[i].fd << " in cgi pollout\n";
			closeSocket(i);
		}
		if (pipe_fd > -1)
			cgiProcesses.erase(pipe_fd);
		handler->setState(Server::returnIncomplete());
	}
	return (true);
}

void	ConnectionManager::handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests)
{
	// std::cout << "POLLIN fd: " << sock_fds.at(i).fd << "Index: " << i << "\n";
	std::map<int, Request*>::iterator it = requests.find(sock_fds.at(i).fd);
	if (it == requests.end())
	{
		// std::cout << CYAN <<  "Creating a new request for fd " << sock_fds.at(i).fd << "\n" << RESET;
		requests.insert(std::pair<int, Request*>(sock_fds.at(i).fd, new Request()));
	}
	int ret = receiveRequest(sock_fds.at(i).fd, requests.at(sock_fds.at(i).fd), i, state);	// request has been fully received

	if (ret == -1)
	{
		// std::cout << "Ignoring fd " << sock_fds.at(i).fd << " due to recv() returning 0\n";
		return ;
	}
	if (state == INVALID)
	{
		// std::cerr << "STATE: " << "INVALID\n";
		this->sock_fds[i].events &= ~POLLIN;	// removing POLLIN; unsure why
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
	}
	// if (state == FINISH)
	// 	this->sock_fds.at(i).events &= ~POLLIN;
	if (state == FINISH || (state == HEADER_FINISHED && requests.at(sock_fds.at(i).fd)->isCGI()))	// HEADER_FINISHED indicates partial request
	{
		std::cout << CYAN << "Passing request from fd " << sock_fds.at(i).fd << " to server\n" << RESET;
		this->passRequestToServer(i, &requests.at(sock_fds.at(i).fd));
	}
}

void	ConnectionManager::handleCGIread(unsigned int& i)
{
	ssize_t r;
	char	buffer[BUFFER_SIZE + 1] = {0};
	CGIinfo& cgi_info = cgiProcesses.at(sock_fds.at(i).fd);

	std::cout << "in CGI read\n";
	r = read(sock_fds.at(i).fd, buffer, BUFFER_SIZE);
	if (r == -1)	// should be handled
	{
		std::cerr << "handle cgi pollin read -1\n";
		return ;
	}
	else if (r == 0)
	{
		std::cout << "read 0 bytes\n";
		int	status;

		pid_t	res = waitpid(cgi_info.getPid(), &status, WNOHANG);
		std::cout << "Attempting to wait for pid: " << cgi_info.getPid() << "\n";
		std::cout << "Res: " << res << " pid: " << cgi_info.getPid() << "\n";
		if (res == cgi_info.getPid())
		{
			// the cgi pid can safely be cleaned up
			std::cout << "Script finished executing\n";
		}	// else cleanup will happen later
		cgi_info.completeResponse();	// once read returns 0, that means its all good to respond to

		// in the pollfds vector, find the client fd's index and set its event field to fd.event |= POLLOUT
		const int	client_fd = cgi_info.getClientFd();
		for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
		{
			if (it->fd == client_fd)
			{
				std::cout << "Setting cgi fd " << cgi_info.getClientFd() << "'s client fd " << it->fd << " to POLLOUT\n";
				// it->events &= ~POLLIN;
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
		std::cout << YELLOW << buffer << " will be added to buffer" << NL;
		cgi_info.concatBuffer(std::string(buffer));
		std::cout << BLUE << "Concatenating buffer" << NL;
	}
	std::cout << buffer << "\n";
}

void	ConnectionManager::reapProcesses()
{
	std::cout << "In reap processes function\n";
	for(std::map<int, CGIinfo>::iterator it = this->cgiProcesses.begin(); it != this->cgiProcesses.end(); it++)
	{
		std::cout << YELLOW;
		it->second.printInfo();
		std::cout << RESET;
		int	s;
		pid_t	pid = waitpid(it->second.getPid(), &s, WNOHANG);
		if (pid < 0)
		{
			perror("reapProcesses");
			continue ;
		}
		std::cout << "attempting to reap pid: " << pid << "\n";
		if (pid > 0)
		{
			std::cout << "Reaping pid: " << it->second.getPid() << "\n";
			this->cgiProcesses.erase(it);
		}
	}
}

void	ConnectionManager::handleCgiPollhup(unsigned int& i)
{
	int	status;

	CGIinfo& info = cgiProcesses[sock_fds.at(i).fd];
	pid_t	pid = waitpid(info.getPid(), &status, 0);
	std::cout << "pid: " << pid << "\n";
	info.completeResponse();

	const int& client_fd = info.getClientFd();
	// find the cgi object's client fd
	for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
	{
		if (it->fd == client_fd)
		{
			std::cout << it->fd << " event getting OR'd with POLLOUT\n";
			it->events |= POLLOUT;	// set the client fd's event to POLLOUT
			// it->events &= ~POLLIN;	// remove POLLIN from the client fd
			break ;
		}
	}
}

// void	ConnectionManager::checkAllCGItimeouts()
// {
//     std::vector<std::map<int, CGIinfo>::iterator> timeouts_to_erase;
    
//     // First pass: identify all timeouts
//     for (std::map<int, CGIinfo>::iterator it = cgiProcesses.begin(); it != cgiProcesses.end(); it++)
//     {
//         if (it->second.timedOut(1) == true)
//         {
//             std::cout << "CGI process timed out for pipe fd: " << it->first << " client fd: " << it->second.getClientFd() << "\n";
//             timeouts_to_erase.push_back(it);
//         }
//     }
    
//     // Second pass: handle all timeouts
//     for (std::vector<std::map<int, CGIinfo>::iterator>::iterator timeout_it = timeouts_to_erase.begin(); 
//          timeout_it != timeouts_to_erase.end(); timeout_it++)
//     {
//         std::map<int, CGIinfo>::iterator it = *timeout_it;
        
//         // Kill the CGI process
//         kill(it->second.getPid(), SIGKILL);
        
//         // Find client fd index
//         int client_fd = it->second.getClientFd();
//         unsigned int client_index = 0;
//         bool found_client = false;
        
//         for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
//         {
//             if (sock_fds.at(i).fd == client_fd)
//             {
//                 client_index = i;
//                 found_client = true;
//                 break;
//             }
//         }
        
//         if (found_client)
//         {
//             // Set up timeout response
//             Request* req = requests.at(client_fd);
//             req->setValid(false);
//             req->setStatus("504");
            
//             // Clean up CGI object
//             if (req->getCgiObj() != NULL)
//             {
//                 delete(req->getCgiObj());
//                 req->setCgi(NULL);
//             }
            
//             // Set client to POLLOUT
//             sock_fds.at(client_index).events |= POLLOUT;
            
//             // Pass to server
//             passRequestToServer(client_index, &req);
// 			handlers.at(client_index)->respond(client_index);
//         }
        
//         // Close CGI pipe fd
//         int pipe_fd = it->first;
//         for (unsigned int idx = 0; idx < sock_fds.size(); idx++)
//         {
//             if (sock_fds.at(idx).fd == pipe_fd)
//             {
//                 closeSocketNoRef(idx);
//                 break;
//             }
//         }
        
//         // Remove from map
//         cgiProcesses.erase(it);
//     }
// }

void ConnectionManager::startConnections()
{
	int						res;

	main_listeners = sock_fds.size();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handle);
	std::cout << "Server has " << main_listeners << " listeners\n";
	std::cout << "Listener fds: ";
	for (unsigned int i = 0; i < main_listeners; i++)
		std::cout << sock_fds.at(i).fd << " ";
	std::cout << "\n";
	(void)res;
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
		// checkAllCGItimeouts();
		for (unsigned int i = 0; i < main_listeners; i++)
		{
			if (sock_fds.at(i).revents & POLLIN)
			{
				// debugVectorSizes("startConnections - before newClient");
				newClient(i, sock_fds.at(i));
			}
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			// checkCGItimeouts(i);
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					std::cout << "in CGI pollin\n";
					std::cout << "Found " << sock_fds.at(i).fd << " in the map of fds\n";
					handleCGIread(i);
					continue ;
				}
				else
				{
					handlePollin(i, states.at(i), requests);
					// continue ;
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				// std::cerr << "FD: " << this->sock_fds[i].fd << " is ready for POLLOUT\n";
				if (handleCGIPollout(i) == true)
					continue ;
				handlePollout(states.at(i), i, requests);
				// continue ;
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					std::cout << "POLLHUP CGI event\n";
					handleCgiPollhup(i);
					closeSocket(i);
				}
				else
				{
					std::cout << "POLLHUP event\n";
					std::cerr << "Closing fd " << this->sock_fds.at(i).fd << " becuase POLLHUP\n";
					printError(sock_fds.at(i).revents);
					closeSocket(i);
				}
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
