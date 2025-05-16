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
		// perror("Bind in parameterized constructor");
		freeaddrinfo(info);
		const str	e = "Error! " + ip + ":" + port + ": " + strerror(errno);
		throw (std::runtime_error(e));
		// throw (ConnectionManager::bindException());
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
		file.open(filename.c_str(), std::ios::binary | std::ios::out);
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
	// std::ifstream	tempFile(req->tempFileName.c_str());	// read from the temp file
	std::ifstream		tempFile;	// read from the temp file
	std::ofstream		newFile;
	str					line;
	str					name = "";
	bool				reading_content = false;
	char				buffer[BUFFER_SIZE];
	size_t				r, k;
	size_t				total = 0;	// get the amount of characters read from the file so that you know how many bytes to seek ahead
	size_t				i = 0, j = 0, prevBoundPos = 0;
	str					tempBdr;
	size_t				boundaryPosInFile = 0;
	const str 			boundary = "\r\n" + req->getBoundary();
	bool				sheet;

	tempFile.open(req->tempFileName.c_str(), std::ios::in);
	while (std::getline(tempFile, line))
	{
		// if Content-Disposition contains a filename=" field, then create a file with the given file's extension
		// match it against the Content-Type in the next line for extra security
		if (line.find("Content-Type: ") != str::npos)
		{
			std::getline(tempFile, line);
			reading_content = true;
		}
		if (line.find("Content-Disposition: ") != str::npos)
		{
			std::cout << "Found content disposition\n";
			size_t	fileNamePos = line.find("filename=\"");
			if (fileNamePos != str::npos)
			{
				size_t	endDblquotePos = line.find_last_of('\"');
				std::cout << "Found filename\n";
				name = line.substr(fileNamePos + 10);	// the 10 is the length of filename="
				if (name.find("\"\r") != str::npos)
					name.erase(name.find("\"\r"));
				std::cout << "Filename: " << name << "\n";
				size_t	dotPos = line.find_last_of('.');
				str file_extension = line.substr(dotPos, endDblquotePos - dotPos);
				std::cout << "Extension: " << file_extension << "\n";
				newFile.open(name.c_str(), std::ios::binary | std::ios::out);
			}
		}
		if (reading_content == true)
		{
			std::cout << "putting data to file\n";
			if (newFile.is_open() == false)
			{
				std::cerr << name << ": ";
				throw (std::runtime_error("Couldn't open file to write"));
			}
			total = i = j = prevBoundPos = boundaryPosInFile = 0;
			sheet = true;
			while (sheet)
			{
				tempFile.read(&buffer[0], BUFFER_SIZE);	// read BUFFER_SIZE bytes into the character buffer
				r = tempFile.gcount();
				if (r == 0)
				{
					if (prevBoundPos)
						newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
					break ;
				}
				total += r;
				/*
					- go through each character of the buffer
					- if a match of the boundary character is found, check each proceeding character with the boundary (strstr)
					- if more than 1/4ths of the boundary is found in current buffer, it might mean boundary is present in the next read
					- therefore in the next read, instead of reading BUFFER_SIZE characters, read only up until the missing boundary characters
					(maybe)
				*/
				k = 0;
				while (prevBoundPos && k < r && k + prevBoundPos < boundary.length() && buffer[k] == boundary[k + prevBoundPos])
					k++;
				if (k + prevBoundPos == boundary.length())
				{
					// handle found boundary
					break ;
				}
				else if (prevBoundPos)
				{
					newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
				}
				prevBoundPos = 0;
				i = 0;
				for (; i < r; i++)
				{
					j = 0;
					while (j < boundary.length() && i + j < r && buffer[i + j] == boundary[j])
						j++;
					if (j == boundary.length())	// this means some characters matched the boundary at the end of buffer
					{
						if (i > 0)
						{
							newFile.write(&buffer[0], i);
						}
						sheet = false;
						break ;
					}
					if (i + j == r)	// this means some characters matched the boundary at the end of buffer
					{
						// tempBdr = buffer[j - i];	// create a temp string that contains the boundary characters found so far
						prevBoundPos = j;
						break;
					}
				}
				if (i && sheet)
					newFile.write(&buffer[0], i);
			}
			newFile.close();
			tempFile.seekg(i);
			reading_content = false;
			(void)boundaryPosInFile;
		}
	}
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
		return (INCOMPLETE);
		// return (INVALID);
	}
	buffer[r] = 0;
	// if (req->getHasBody())
	// 	std::cout << MAGENTA << "Received " << req->bytesReceived << " body bytes so far" << NL;
	_request = buffer;
	// std::cout << YELLOW << r << NL;
	// for (size_t i = 0; i < _request.size(); i++)
	// {
	// 	if (_request[i] == '\n')
	// 		std::cout << "\\n";
	// 	else if (_request[i] == '\r')
	// 		std::cout << "\\r";
	// 	else if (_request[i] == '\0')
	// 		std::cout << "\\0";
	// 	else
	// 		std::cout << _request[i];
	// }
	// std::cout << "\n";
	req->pushRequest(_request);
	// if ((_request.find("\r\n\r\n") != std::string::npos && req->getHeaderReceived() == false) || _request[0] == '\r')	// the buffer contains the end of the request header
	if ((_request.find("\r\n\r\n") != std::string::npos && req->getHeaderReceived() == false))	// the buffer contains the end of the request header
	{
		std::cout << MAGENTA << "Found end of header for fd " << client_fd << NL;
		str	rq = req->getRequest();
		req->parseRequest(rq);
		std::cout << req->getRequest();
		req->setHeaderReceived(true);
		if (req->getContentLen() != 0)
			req->setHasBody(true);
		else
		{
			std::cout << CYAN << "Received request fully for fd " << client_fd << NL;
			return (FINISH);
		}
		// write the body's bytes onto the temp file
		if (req->getHasBody() == true)	// if the header is already fully received AND the request contains a body
		{
			size_t	endPos;
			openTempFile(req, file);
			endPos = (_request.find("\r\n\r\n") != str::npos ? _request.find("\r\n\r\n") + 4 : str::npos);
			str	b(buffer);
			b = b.substr(0, endPos);
			std::cout << "Length of header part: " << b.length() << "\n";
			if (endPos == str::npos || static_cast<ssize_t>(endPos) == r)
			{
				std::cout << RED << "There is a body but it is not present in request" << NL;
				return (INCOMPLETE);
			}
			std::cout << "Received " << r << " bytes total of which " << r - endPos << " bytes are for body\n";
			req->bytesReceived += r - endPos;
			file.write(&buffer[endPos], r - endPos);
			if (file.bad() || file.fail())
				throw(std::runtime_error("Couldn't write data to temp file\n"));
			if (req->bytesReceived == req->getContentLen())
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
			if (req->bytesReceived == req->getContentLen())
			{
				std::cout << YELLOW << "Temp file size: " << file.tellp() << NL;
				file.close();
				parseBodyFile(req);
				return (FINISH);
			}
			// std::cout << "received " << req->bytesReceived << " bytes so far\n";
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
			std::cout << "No FD event occurring\n";
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
					this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
				}
				continue ;
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handlers.at(i) && state == FINISH)
				{
					// std::cout << "Responding to request\n";
					bool should_close;
					if ((should_close = handlers.at(i)->respond(sock_fds.at(i).fd)) && handlers.at(i)->getState() == Server::returnFinish())
					{
						std::cout << "\033[31mRemoving request from map\033[0m\n";
						delete requests[(sock_fds.at(i).fd)];
						requests.erase(sock_fds.at(i).fd);
						// requests.erase(requests.begin() + (test));
						if (should_close)
						{
							std::cout << "Closing client socket fd " << sock_fds.at(i).fd << "\n";
							closeSocket(i);
						}
					}
				}
				continue ;
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
