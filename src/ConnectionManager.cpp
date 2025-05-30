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
	// fcntl(client.fd, F_SETFD, fcntl(client.fd, F_GETFD) | FD_CLOEXEC);
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	sock_fds.push_back(client);
	std::cout << "Pushing back client fd " << client.fd << "\n";
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
	// delete *req;
	// *req = NULL;
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	close(sock_fds.at(index).fd);
	std::cout << "\033[31mClosing fd " << sock_fds.at(index).fd << "\033[0m\n";
	sock_fds.erase(sock_fds.begin() + index);
	reqs.erase(reqs.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	// index--;
}

void	ConnectionManager::openTempFile(Request* req, std::fstream& file)
{
	if (file.is_open() == false)	// open temp file for reading message body
	{
		str	filename = TEMP_FILE;
		filename += static_cast<char>((ConnectionManager::number++ % 10) + '0');
		req->tempFileName = filename;
		std::cout << "Trying to open with filename " << filename << "\n";
		file.open(filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
		if (file.fail())
		{
			perror("Open");
			throw (std::runtime_error("Couldn't open temp file\n"));
		}
		else if (file.good())
		{
			std::cout << "Successfully opened " << filename << "\n";
			return ;
		}
	}
	std::cout << "Temp file is already opened\n";
	(void)req;
}

void	ConnectionManager::parseBodyFile(Request* req)
{
	std::fstream	tempFile;
	str				line;
	bool			newBound = false;
	bool			writeFile = false;
	std::ofstream	binFile;
	ssize_t			written = 0;

	tempFile.open(req->tempFileName.c_str(), std::ios::in | std::ios::binary);
	while (std::getline(tempFile, line))
	{
		// std::cout << RED << "Line: " << line << NL;
		if (line.find(req->getBoundary()) != str::npos && writeFile == false)
		{
			newBound ^= true;
			// if (newBound)
			// 	newBound = false;
			continue ;
		}
		else if (writeFile)
		{
			if (line.find("Content-Type") != str::npos)
				continue ;
			if (line.find(req->getBoundary()) != str::npos)
			{
				std::cout << "Size: " << binFile.tellp() << NL;
				std::cout << "Wrote: " << written << NL;
				binFile.close();
				writeFile = false;
			}
			else
			{
				std::cout << "writing\n";
				binFile.write(&line[0], line.length());
				written += line.length();
			}
		}
		if (line.find("filename=") != str::npos)
		{
			// str	filename = 
			// create new file that stores the binary data of the file
			// std::ofstream	binFile(line.substr(line.find("filename=\"") + std::strlen("filename=\""), str::npos), std::ios::binary | std::ios::out);
			str				file = line.substr(line.find("filename=\"") + std::strlen("filename=\""), str::npos);
			file.erase(file.begin() + file.find_last_of('\"'), file.end());
			binFile.open(file.c_str(), std::ios::binary | std::ios::out);
			if (binFile.bad())
				throw std::runtime_error("Couldn't open upload file");
			writeFile = true;
		}
	}
	(void)newBound;
}

ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
{
	char			buffer[BUFFER_SIZE + 1];
	ssize_t			r;
	std::string		_request;
	std::fstream&	file = req->getBodyFile();

	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r <= 0)
	{
		closeSocket(index);
		std::cout << "\033[31mRecv returned " << r << "\033[0m\n";
		return (state);
	}
	buffer[r] = '\0';
	// if (req->getHasBody())
	// 	std::cout << MAGENTA << "Received " << req->bytesReceived << " body bytes so far" << NL;
	_request = str(buffer);
	req->pushRequest(_request);
	if (req->getHeaderReceived() == false && _request.find("\r\n\r\n") != str::npos)	// the buffer contains the end of the request header
	{
		std::cout << MAGENTA << "Found end of header" << NL;
		str	rq = req->getRequest();
		req->parseRequest(rq);
		req->setHeaderReceived(true);
		if (req->getContentLen() != 0)
			req->setHasBody(true);
		else
			return (FINISH);
		// write the body's bytes onto the temp file
		if (req->getHeaderReceived() == true && req->getHasBody() == true)	// if the header is already fully received AND the request contains a body
		{
			size_t	endPos;
			openTempFile(req, file);
			endPos = (rq.find("\r\n\r\n") != str::npos ? rq.find("\r\n\r\n") + 4 : str::npos);
			std::cout << CYAN << "Position of header end: " << endPos << NL;
			if (endPos == str::npos || static_cast<ssize_t>(endPos) == r)
			{
				std::cout << RED << "There is a body but it is not present in request" << NL;
				return (INCOMPLETE);
			}
			str	body = rq.substr(endPos, r);
			std::cout << YELLOW << "r - endPos: " << r - endPos << " endpos: " << endPos << NL;
			req->bytesReceived += r - endPos;
			// req->bytesReceived += body.length();
			file.write(&body[0], body.length());
			if (file.bad() || file.fail())
				throw(std::runtime_error("Couldn't write data to temp file\n"));
			if (body.find(req->body_boundaryEnd) != str::npos)
			{
				parseBodyFile(req);
				return (FINISH);
			}
		}
	}
	else
	{
		if (req->getHasBody())
		{
			str	buf = buffer;
			req->bytesReceived += r;
			file.write(&buffer[0], r);
			if (buf.find(req->body_boundaryEnd) != std::string::npos || req->bytesReceived == req->getContentLen())
			{
				std::cout << "Found ending boundary\n";
				std::cout << CYAN << "Content len: " << req->getContentLen() << " Received the full content length's worth of bytes: " << req->bytesReceived << NL;
				std::cout << RED << buf << NL;
				std::cout << YELLOW << "File size: " << file.tellp() << NL;
				file.close();
				parseBodyFile(req);
				return (FINISH);
			}
			std::cout << "received " << req->bytesReceived << " bytes so far\n";
			return (INCOMPLETE);
		}
	}
	std::cout << "Request hasn't been fully received\n";
	(void)state;
	return (INCOMPLETE);
}

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
		{
			std::cout << "No FD event occuring\n";
			continue ;
		}
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
				// continue ;
			}
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		{
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				std::cout << "POLLIN fd: " << sock_fds.at(i).fd << "\n";
				if (requests[sock_fds.at(i).fd] == NULL)
				{
					requests[sock_fds.at(i).fd] = new Request();
					std::cout << "Creating a new request for fd " << sock_fds.at(i).fd << "\n";
				}
				if ((state = receiveRequest(sock_fds.at(i).fd, requests[sock_fds.at(i).fd], i, state)) == FINISH)	// request has been fully received
				{
					std::cout << "Passing request from fd " << sock_fds.at(i).fd << " to server\n";
					// std::cout << MAGENTA << requests[sock_fds.at(i).fd]->getRequest() << NL;
					this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handlers.at(i) && state == FINISH)
				{
					std::cout << "Responding to request\n";
					if (handlers.at(i)->respond(sock_fds.at(i).fd))
					{
						std::cout << "\033[31mRemoving request from map\033[0m\n";
						delete requests[(sock_fds.at(i).fd)];
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
