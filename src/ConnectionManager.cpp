/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/06/17 16:42:58 by ppolinta         ###   ########.fr       */
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
	fcntl(client.fd, F_SETFD, fcntl(client.fd, F_GETFD) | FD_CLOEXEC);
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	std::cout << "Pushing back client fd " << client.fd << " at index " << sock_fds.size() << "\n";
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

// if cgi is requested, don't erase the request for the client_fd
void ConnectionManager::passRequestToServer(int i, Request **req, 
	std::vector<struct pollfd>& pollfds, std::map<int, int>& cgiFds)
{
	std::cout << "Request host: " << (*req)->getHost() << "\n";
	if (!(*req)->isValidRequest() || servers_per_ippp[i].find((*req)->getHost()) == servers_per_ippp[i].end())
		handlers.at(i) = defaults.at(i);
	else
		handlers.at(i) = servers_per_ippp[i][(*req)->getHost()];
	handlers.at(i)->handleRequest(sock_fds.at(i).fd, *req, pollfds, cgiFds);
	// delete *req;
	// *req = NULL;
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	close(sock_fds.at(index).fd);
	std::cout << "\033[31mClosing fd " << sock_fds.at(index).fd << "\033[0m\n";
	sock_fds.erase(sock_fds.begin() + index);
	if (index < reqs.size())
		reqs.erase(reqs.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	index--;
}

void	ConnectionManager::openTempFile(Request* req, std::fstream& file)
{
	// the filename is stored in the request object
	// store the temp files in a vector
	// when the server is closed, delete every file in the vector
	if (file.is_open() == false)	// open temp file for reading message body
	{

		char uniqueName[255] = {0};
		strcpy(uniqueName, ".tempXXXXXX");
		mktemp(uniqueName);
		str filename = str(uniqueName);
		req->setTempFileName(filename);
		// req->tempFileName = filename;

		std::cout << "Trying to open with filename " << filename << "\n";
		file.open(filename.c_str(), std::ios::binary | std::ios::out);
		if (file.fail())
		{
			perror("Open");
			throw (std::runtime_error("Couldn't open temp file\n"));
		}
		else if (file.good())
		{
			// this->tempFileNames.push_back(filename);

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

	tempFile.open(req->getTempFileName().c_str(), std::ios::in);
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
	if (!access(req->getTempFileName().c_str(), F_OK))	// remove the temp file used for storing binary data0
		std::remove(req->getTempFileName().c_str());
}

ConnectionManager::State	ConnectionManager::handleFirstRecv(std::fstream& tempFile, 
	str _request, Request* req, char* buffer, ssize_t& r)
{
	// std::fstream&	tempFile = req->getBodyFile();

	str	rq = req->getRequest();
	req->parseRequest(rq);
	std::cout << req->getRequest();
	req->setHeaderReceived(true);
	if (req->getContentLen() != 0)
		req->setHasBody(true);
	std::cerr << (req->getHasBody() ? "Request has a body" : "Request does not have a body") << "\n";
	if (req->getHasBody() == true)	// if the header is already fully received AND the request contains a body
	{
		size_t	endPos;
		openTempFile(req, tempFile);
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
		tempFile.write(&buffer[endPos], r - endPos);
		if (tempFile.bad() || tempFile.fail())
			throw(std::runtime_error("Couldn't write data to temp file\n"));
		if (req->bytesReceived == req->getContentLen())
		{
			parseBodyFile(req);
			return (FINISH);
		}
	}
	return (FINISH);
}

ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
{
	char			buffer[BUFFER_SIZE + 1];
	ssize_t			r;
	std::string		_request;
	std::fstream&	file = req->getBodyFile();

	if (!req)
	{
		std::cout << "There is no request\n";
	}
	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r <= 0)
	{
		closeSocket(index);
		std::cout << "\033[31mRecv returned " << r << "\033[0m\n";
		std::cout << MAGENTA << "FD: " << client_fd << "\nIndex: " << index << RESET << "\n";
		// index--;
		return (INVALID);
	}
	buffer[r] = 0;
	_request = buffer;
	req->pushRequest(_request);
	// std::cout << CYAN << req << NL;
	if ((_request.find("\r\n\r\n") != std::string::npos && req->getHeaderReceived() == false))	// the buffer contains the end of the request header
	{
		// return (handleFirstRecv(file, _request, req, buffer, r));
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
				throw (std::runtime_error("Couldn't write data to temp file\n"));
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
	// std::cout << "Request hasn't been fully received\n";
	(void)state;
	return (INCOMPLETE);
}

/// @brief handles all pollout events
/// @param state the current state of the response
/// @param i the index of the handling server
/// @param requests the map of requests wherein the socket fd is the key and the request object is the value
void	ConnectionManager::handlePollout(State& state, unsigned int& i, std::map<int, Request *> &requests)
{
	if (handlers[i] && state == FINISH)	// the request has been parsed and ready for response building
	{
		bool keep_open = handlers[i]->respond(sock_fds[i].fd);
		if (keep_open && handlers[i]->getState() == Server::returnFinish())
		{
			std::cerr << "Finished responding to request\n";
			if (requests.find(sock_fds[i].fd) != requests.end())
			{
				std::cout << "\033[31mRemoving request from map\033[0m\n";
				requests.erase(sock_fds[i].fd);
			}
		}
		// bool keep_open = false;	// keep-alive becomes false once fd reaches certain time without any event
		// if ((keep_open = handlers[i]->respond(sock_fds[i].fd)) && handlers[i]->getState() == Server::returnFinish())
		// {
		// 	std::cerr << "Finished responding to request\n";
		// 	if (requests.find(sock_fds[i].fd) != requests.end())
		// 	{
		// 		std::cout << "\033[31mRemoving request from map\033[0m\n";
		// 		requests.erase(sock_fds[i].fd);
		// 	}
		// }
		else if (keep_open == false && handlers[i]->getState() == Server::returnFinish())
		{
			if (requests.find(sock_fds[i].fd) == requests.end())
				std::cout << "Not found\n";
			else
				std::cout << "Request exists and will now be deleted\n";
			delete requests[(sock_fds[i].fd)];
			requests.erase(sock_fds[i].fd);
			std::cout << "Closing client socket fd " << sock_fds[i].fd << "\n";
			std::cout << "Closing socket\n";
			closeSocket(i);
		}
		if (!handlers[i])
			std::cout << "There is no handler\n";
		// if (handlers[i]->sent_bytes)
		// 	std::cerr << "Sent " << handlers[i]->sent_bytes << "\n";
		handlers[i]->setState(Server::returnIncomplete());
		// handlers[i]->sent_bytes = 0;
	}
}

void	ConnectionManager::handleCGIPollout(State& state, char* buf, unsigned int& i, 
		std::map<int, Request *> &requests)
{
	for (std::map<int, Request*>::iterator it = requests.begin(); it != requests.end(); it++)
	{
		std::cout << RED << "FIRST: " << (*it).first << NL;
		std::cout << RED << "SECOND: " << (*it).second << NL;
	}
	if (requests.find(sock_fds.at(i).fd) != requests.end())
		std::cout << requests.find(sock_fds.at(i).fd)->second->getRequest() << "\n";
	else
		std::cout << "Requests map doesn't contain a request for fd " << sock_fds.at(i).fd << "\n";
	std::cout << "CGI pollout\n";
	send(sock_fds.at(i).fd, buf, BUFFER_SIZE, 0);
	std::cout << BLUE << "Buffer: " << buf << NL;
	std::memset(buf, 0, BUFFER_SIZE);
	if (buf[0] == 0)
		std::cout << "empty\n";
	if (handlers[i])
		handlers[i]->setState(Server::returnFinish());
	else
		std::cout << "There is no handler for current request\n";
	(void)state;
	(void)requests;
}

void	ConnectionManager::handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests,
	std::map<int, int>& cgiFds)
{
	std::cout << "POLLIN fd: " << sock_fds.at(i).fd << "Index: " << i << "\n";
	std::map<int, Request*>::iterator it = requests.find(sock_fds.at(i).fd);
	//Print here and inside of receiveRequest to know where it's stopping
	if (it == requests.end())
	{
		std::cout << CYAN <<  "Creating a new request for fd " << sock_fds.at(i).fd << "\n" << RESET;
		requests.insert(std::pair<int, Request*>(sock_fds.at(i).fd, new Request));
		std::cout << "Creating a new request for fd " << sock_fds.at(i).fd << "\n";
	}
	if ((state = receiveRequest(sock_fds.at(i).fd, requests.at(sock_fds.at(i).fd), i, state)) == FINISH)	// request has been fully received
	{
		std::cout << CYAN << "Passing request from fd " << sock_fds.at(i).fd << " to server\n" << RESET;
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd], sock_fds, cgiFds);
		std::cout << "Passing request from fd " << sock_fds.at(i).fd << " to server\n";
	}
	//Print here and inside of receiveRequest to know where it's stopping
}

void	ConnectionManager::handleCGIread(char* buf, unsigned int& i, std::map<int, int>& cgiFds)
{
	// if the cgi_fd is ready for POLLIN,
	std::cout << "in cgi read\n";
	ssize_t r;
	int	status;

	// std::cout << "in  CGI read\n";
	r = read(sock_fds.at(i).fd, buf, BUFFER_SIZE);
	if (r == 0)
	{
		close(sock_fds.at(i).fd);
		sock_fds.erase(sock_fds.begin() + i);	// remove the fd from the poll() snce reading is finished
		std::cout << "Fully received response\n";
		i--;
		std::cout << "fd : " << sock_fds.at(i).fd << "\n";
		if (cgiFds.find(sock_fds.at(i).fd) != cgiFds.end())
		{
			std::cout << RED << "Erasing fd from map" << NL;
			cgiFds.erase(sock_fds.at(i).fd); // remove the file descriptor from the map
		}
		else
			std::cout << "Fd is not in map\n";
	}
	else if (r == -1)
	{
		std::cerr << "EXITING SERVER\n";
		exit(1);
	}
	else
	{
		buf[r] = 0;
		std::cout << buf << "\n";
	}
	std::cout << YELLOW << "Read " << buf << NL;
	waitpid(sock_fds.at(i).fd, &status, WNOHANG);
	(void)status;
}

void ConnectionManager::startConnections()
{
	int							res;
	State						state = INCOMPLETE;
	std::map<int, int>			cgiFds;	// key is the read end of the pipe and value is the client_fd

	std::map<int, Request *>	requests;
	char	buf[BUFFER_SIZE] = {0};

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
			// don't add a new client if there is an existing fd that can be used to handle the request
			if (sock_fds.at(i).revents & POLLIN && sock_fds.size() == main_listeners)
				newClient(i, sock_fds.at(i));
		}
		for (unsigned int i = main_listeners; i < sock_fds.size(); i++)
		// for (unsigned int i = sock_fds.size() - 1; i >= main_listeners; i--)
		{
			// std::cout << "i: " << i << " pollfds size: " << sock_fds.size() << "\n";
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				/* 
					if the read end file descriptor is found in the map of fds
						read from the file descriptor
					// once the response has been fully read, remove the pair from the map
				*/
				if (cgiFds.find(sock_fds.at(i).fd) != cgiFds.end())
				{
					std::cout << "Found " << sock_fds.at(i).fd << " in the map of fds\n";
					std::cout << "Handling cgi\n";
					handleCGIread(buf, i, cgiFds);
				}
				else
				{
					handlePollin(i, state, requests, cgiFds);
					continue ;
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)	// cgi_fd will never enter pollout
			{
				bool	cgiPollout = false;
				// if the client fd is in the cgi fds map, this means it is ready to send response
				for (std::map<int, int>::iterator it = cgiFds.begin(); it != cgiFds.end(); it++)
				{
					if ((*it).second == sock_fds.at(i).fd)
						cgiPollout = !cgiPollout;

				}
				if (cgiPollout)
				{
					std::cout << "in CGI pollout\n";
					handleCGIPollout(state, buf, i, requests);
				}
				else
				{
					handlePollout(state, i, requests);
					continue ;
				}
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				std::cout << "POLLHUP event\n";
				if (cgiFds.find(sock_fds.at(i).fd) != cgiFds.end())
				{
					std::cout << "CGI fd is in POLLHUP\n";
					close(sock_fds.at(i).fd);
					cgiFds.erase(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					i--;
				}
				else
				{
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
	{
		closeSocket(i);
	}
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
