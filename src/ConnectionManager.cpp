/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 18:40:42 by pipolint          #+#    #+#             */
/*   Updated: 2025/07/09 23:50:50 by ppolinta         ###   ########.fr       */
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
	client.events = POLLIN;
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
void ConnectionManager::passRequestToServer(int i, Request **req)
{
	if (!(*req))
		return ;
	// std::cerr << "Passing request to server with " << ((*req)->isCompleteRequest() ? "a complete request" : "a partial request") << "\n";
	if (!(*req)->isValidRequest())
		handlers.at(i) = defaults.at(i);
	if (servers_per_ippp[i].find((*req)->getHost()) == servers_per_ippp[i].end())
		handlers.at(i) = defaults.at(i);
	else if((*req)->isValidRequest())
		handlers.at(i) = servers_per_ippp[i][(*req)->getHost()];
	handlers.at(i)->handleRequest(i, sock_fds.at(i).fd, *req, this->sock_fds, cgiProcesses);
}

void	ConnectionManager::deleteRequest(unsigned int i)
{
	Request*	temp = this->requests[sock_fds[i].fd];
	this->requests.erase(sock_fds[i].fd);
	delete temp;
}

void	ConnectionManager::closeSocket(unsigned int& index)
{
	close(sock_fds.at(index).fd);
	std::cout << "\033[31mClosing fd " << sock_fds.at(index).fd << "\033[0m\n";
	if (requests.find(sock_fds.at(index).fd) == requests.end())
		std::cout << "Not found\n";
	else
	{
		std::cout << "Request exists and will now be deleted\n";
		deleteRequest(index);
		// delete requests[(sock_fds.at(index).fd)];
	}
	sock_fds.erase(sock_fds.begin() + index);
	if (index < reqs.size())
		reqs.erase(reqs.begin() + index);
	handlers.erase(handlers.begin() + index);
	defaults.erase(defaults.begin() + index);
	servers_per_ippp.erase(servers_per_ippp.begin() + index);
	index--;
}

// void	ConnectionManager::openTempFile(Request* req, std::fstream& file)
// {
// 	// the filename is stored in the request object
// 	// store the temp files in a vector
// 	// when the server is closed, delete every file in the vector
// 	if (file.is_open() == false)	// open temp file for reading message body
// 	{

// 		char uniqueName[255] = {0};
// 		strcpy(uniqueName, ".tempXXXXXX");
// 		int tmp = mkstemp(uniqueName);
// 		str filename = str(uniqueName);
// 		// req->setTempFileName(filename);
// 		// req->tempFileName = filename;

// 		std::cout << "Trying to open with filename " << filename << "\n";
// 		close(tmp);
// 		file.open(filename.c_str(), std::ios::binary | std::ios::out);
// 		if (file.fail())
// 		{
// 			perror("Open");
// 			throw (std::runtime_error("Couldn't open temp file\n"));
// 		}
// 		else if (file.good())
// 		{
// 			// this->tempFileNames.push_back(filename);

// 			std::cout << "Successfully opened " << filename << "\n";
// 			return ;
// 		}
// 	}
// 	std::cout << "Temp file is already opened\n";
// 	(void)req;
// }

// void	ConnectionManager::parseBodyFile(Request* req)
// {
// 	// std::ifstream	tempFile(req->tempFileName.c_str());	// read from the temp file
// 	std::ifstream		tempFile;	// read from the temp file
// 	std::ofstream		newFile;
// 	str					line;
// 	str					name = "";
// 	bool				reading_content = false;
// 	char				buffer[BUFFER_SIZE];
// 	size_t				r, k;
// 	size_t				total = 0;	// get the amount of characters read from the file so that you know how many bytes to seek ahead
// 	size_t				i = 0, j = 0, prevBoundPos = 0;
// 	str					tempBdr;
// 	size_t				boundaryPosInFile = 0;
// 	const str 			boundary = "\r\n" + req->getBoundary();
// 	bool				sheet;

// 	tempFile.open(req->getTempFileName().c_str(), std::ios::in);
// 	while (std::getline(tempFile, line))
// 	{
// 		// if Content-Disposition contains a filename=" field, then create a file with the given file's extension
// 		// match it against the Content-Type in the next line for extra security
// 		if (line.find("Content-Type: ") != str::npos)
// 		{
// 			std::getline(tempFile, line);
// 			reading_content = true;
// 		}
// 		if (line.find("Content-Disposition: ") != str::npos)
// 		{
// 			std::cout << "Found content disposition\n";
// 			size_t	fileNamePos = line.find("filename=\"");
// 			if (fileNamePos != str::npos)
// 			{
// 				size_t	endDblquotePos = line.find_last_of('\"');
// 				std::cout << "Found filename\n";
// 				name = line.substr(fileNamePos + 10);
// 				if (name.find("\"\r") != str::npos)
// 					name.erase(name.find("\"\r"));
// 				std::cout << "Filename: " << name << "\n";
// 				size_t	dotPos = line.find_last_of('.');
// 				str file_extension = line.substr(dotPos, endDblquotePos - dotPos);
// 				std::cout << "Extension: " << file_extension << "\n";
// 				newFile.open(name.c_str(), std::ios::binary | std::ios::out);
// 			}
// 		}
// 		if (reading_content == true)
// 		{
// 			std::cout << "putting data to file\n";
// 			if (newFile.is_open() == false)
// 			{
// 				std::cerr << name << ": ";
// 				throw (std::runtime_error("Couldn't open file to write"));
// 			}
// 			total = i = j = prevBoundPos = boundaryPosInFile = 0;
// 			sheet = true;
// 			while (sheet)
// 			{
// 				tempFile.read(&buffer[0], BUFFER_SIZE);	// read BUFFER_SIZE bytes into the character buffer
// 				r = tempFile.gcount();
// 				if (r == 0)
// 				{
// 					if (prevBoundPos)
// 						newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
// 					break ;
// 				}
// 				total += r;
// 				k = 0;
// 				while (prevBoundPos && k < r && k + prevBoundPos < boundary.length() && buffer[k] == boundary[k + prevBoundPos])
// 					k++;
// 				if (k + prevBoundPos == boundary.length())
// 				{
// 					// handle found boundary
// 					break ;
// 				}
// 				else if (prevBoundPos)
// 				{
// 					newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
// 				}
// 				prevBoundPos = 0;
// 				i = 0;
// 				for (; i < r; i++)
// 				{
// 					j = 0;
// 					while (j < boundary.length() && i + j < r && buffer[i + j] == boundary[j])
// 						j++;
// 					if (j == boundary.length())	// this means some characters matched the boundary at the end of buffer
// 					{
// 						if (i > 0)
// 						{
// 							newFile.write(&buffer[0], i);
// 						}
// 						sheet = false;
// 						break ;
// 					}
// 					if (i + j == r)	// this means some characters matched the boundary at the end of buffer
// 					{
// 						// tempBdr = buffer[j - i];	// create a temp string that contains the boundary characters found so far
// 						prevBoundPos = j;
// 						break;
// 					}
// 				}
// 				if (i && sheet)
// 					newFile.write(&buffer[0], i);
// 			}
// 			newFile.close();
// 			tempFile.seekg(i);
// 			reading_content = false;
// 			(void)boundaryPosInFile;
// 		}
// 	}
// 	if (!access(req->getTempFileName().c_str(), F_OK))	// remove the temp file used for storing binary data0
// 		std::remove(req->getTempFileName().c_str());
// }

/*
	this function is called in handlePollin
	it returns a state which is either: INVALID, INCOMPLETE, HEADER_FINISHED, and FINISHED
	INVALID typically means its an invalid request
	INCOMPLETE means the request was partially received and there is still more to receive
	HEADER_FINISHED means that the header has been received and parsed, so it should now be passed to the server. This is only possible if POST request
	FINISHED means that the request has been fully received, with its headers parsed, and it is now ready to be sent to its appropriate handler
*/
ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
{
	std::string	_request;
	ssize_t		r;
	size_t		barbenindex;
	int			outcome;
	char		buffer[BUFFER_SIZE + 1];

	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r < 0)
		return (INCOMPLETE);
	if (r == 0)
	{
		std::cerr << "Closing socket in receive request due to recv() returning 0\n";
		closeSocket(index);
		return (INVALID);
	}
	// 1- Have a function like pushRequest that handles body parts (pushBody). For example discards the boundaries and body fields and saves the files.
	// 2- In order to save the file I need to open an FD and put it in the array with the others to be POLLed
	if (req->getHeaderReceived())
	{
		req->addReceivedBytes(r);
		if (req->isCGI())	// write receive bytes to the CGI_PIPE'S write end
		{
			Cgi* cgi = req->getCgiObj();
			cgi->writeToFd(cgi->get_stdin()[WRITE], buffer, r, req);	// this function will write to the fd and will close it if its done writing
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
			case 502:
				if (outcome == 501)
					req->setStatus("501");
				req->setValid(false);
				return (INVALID);
			case 1:
				if (!req->isValidRequest())
					return (INVALID);
				else
				{
					if (servers_per_ippp[index].find(req->getHost()) == servers_per_ippp[index].end())
						handlers.at(index) = defaults.at(index);
					else if(req->isValidRequest())
						handlers.at(index) = servers_per_ippp[index][req->getHost()];
					if (handlers.at(index)->checkRequestValidity(req) == false)
					{
						req->setStatus((req->getContentLen() > handlers.at(index)->getMaxBodySize()) ? "501" : "405");
						req->setValid(false);
						return (INVALID);
					}
				}
				barbenindex = (_request.find("\r\n\r\n") != std::string::npos ? _request.find("\r\n\r\n") + 4 : std::string::npos);
				if ((ssize_t)barbenindex < r) // If there are left-overs.
				{
					r = r - barbenindex;
					std::memmove(buffer, buffer + barbenindex, r + 1);
					if (!req->isCGI())
						break ;
					req->setLeftOvers(buffer, r);	// add leftovers to the request object
					req->addReceivedBytes(r);
				}
				if (req->getContentLen() != 0)
				{
					if (req->getReceivedBytes() == req->getContentLen())
						return (FINISH);
					std::cerr << "Partially received body bytes\n";
					return (req->getHeaderReceived() ? HEADER_FINISHED : INCOMPLETE);
				}
				std::cerr << "There is no body for this request\n";
				return (FINISH);
				// The header is fully received, the leftovers are saved in _request
				// 1) Regular POST: Start sifting through the current and following _request strings for files to be saved as upload
				// 2) CGI: Save this current leftover in whatever server will respond (how?), and after sending it to the subprocess immediately send following _request to the stdin of the subprocess
			default:
				return (INCOMPLETE);
		}
	}
	if (req->getHeaderReceived() && req->getMethod() == "POST" && !req->isCGI() && r > 0)
	{
		// outcome = req->pushBody(buffer, r);
		outcome = 1;
		switch(outcome)
		{
			case -1:
				// > maxbodysize
				// > contentlength
				// No boundary
			case 1:
				return (FINISH);
			default:
				return (INCOMPLETE);
		}
	}
	(void)state;
	return (FINISH);
}

// ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
// {
// 	char			buffer[BUFFER_SIZE + 1];
// 	ssize_t			r;
// 	std::string		_request;
// 	std::fstream&	file = req->getBodyFile();

// 	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
// 	if (r < 0)
// 		return (INCOMPLETE);
// 	if (r == 0)
// 	{
// 		closeSocket(index);
// 		return (INVALID);
// 	}
// 	buffer[r] = 0;
// 	_request = buffer;
// 	req->pushRequest(_request);
// 	if ((_request.find("\r\n\r\n") != std::string::npos && req->getHeaderReceived() == false))	// the buffer contains the end of the request header
// 	{
// 		std::cout << MAGENTA << "Found end of header for fd " << client_fd << NL;
// 		str rq = req->getRequest();
// 		req->parseRequest(rq);
// 		std::cout << req->getRequest();
// 		req->setHeaderReceived(true);
// 		if (req->getContentLen() != 0)
// 			req->setHasBody(true);
// 		else
// 		{
// 			std::cout << CYAN << "Received request fully for fd " << client_fd << NL;
// 			return (FINISH);
// 		}
// 		if (req->getHasBody() == true)	// if the header is already fully received AND the request contains a body
// 		{
// 			size_t	endPos;
// 			openTempFile(req, file);
// 			endPos = (_request.find("\r\n\r\n") != str::npos ? _request.find("\r\n\r\n") + 4 : str::npos);
// 			str	b(buffer);
// 			b = b.substr(0, endPos);
// 			std::cout << "Length of header part: " << b.length() << "\n";
// 			if (endPos == str::npos || static_cast<ssize_t>(endPos) == r)
// 			{
// 				std::cout << RED << "There is a body but it is not present in request" << NL;
// 				return (INCOMPLETE);
// 			}
// 			std::cout << "Received " << r << " bytes total of which " << r - endPos << " bytes are for body\n";
// 			req->bytesReceived += r - endPos;
// 			file.write(&buffer[endPos], r - endPos);
// 			if (file.bad() || file.fail())
// 				throw (std::runtime_error("Couldn't write data to temp file\n"));
// 			if (req->bytesReceived == req->getContentLen())
// 			{
// 				parseBodyFile(req);
// 				req->clearVector();
// 				return (FINISH);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (req->getHasBody())
// 		{
// 			str	buf = buffer;
// 			req->bytesReceived += r;
// 			file.write(&buffer[0], r);
// 			if (req->bytesReceived == req->getContentLen())
// 			{
// 				std::cout << YELLOW << "Temp file size: " << file.tellp() << NL;
// 				file.close();
// 				parseBodyFile(req);
// 				return (FINISH);
// 			}
// 			return (INCOMPLETE);
// 		}
// 	}
// 	(void)state;
// 	return (INCOMPLETE);
// }

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
			std::cout << "Closed socket\n";
			return ;
		}
		std::cerr << RED << "INCOMPLETE" << NL;
		handler->setState(Server::returnIncomplete());
	}
}

bool	ConnectionManager::handleCGIPollout(unsigned int& i)
{
	CGIinfo* infoPtr = NULL;
	int	pipe_fd = -1;

	for (std::map<int, CGIinfo>::iterator it = cgiProcesses.begin(); it != cgiProcesses.end(); it++)
	{
		// if the client fd is present in the map and the cgi script has finished executing
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

	if (handler->cgiRespond(infoPtr) == true)	// if true, remove the cgiProcess from the map
	{
		std::cout << "Closing client socket fd " << this->sock_fds[i].fd << " in cgi pollout\n";
		closeSocket(i);
		std::cout << "Closed socket\n";
		cgiProcesses.erase(pipe_fd);
	}
	return (true);
}

void	ConnectionManager::handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests)
{
	std::cout << "POLLIN fd: " << sock_fds.at(i).fd << "Index: " << i << "\n";
	std::map<int, Request*>::iterator it = requests.find(sock_fds.at(i).fd);
	if (it == requests.end())
	{
		std::cout << CYAN <<  "Creating a new request for fd " << sock_fds.at(i).fd << "\n" << RESET;
		requests.insert(std::pair<int, Request*>(sock_fds.at(i).fd, new Request));
		std::cout << "Creating a new request for fd " << sock_fds.at(i).fd << "\n";
	}
	state = receiveRequest(sock_fds.at(i).fd, requests.at(sock_fds.at(i).fd), i, state);	// request has been fully received
	if (state == INVALID)
	{
		std::cerr << "STATE: " << "INVALID\n";
		this->sock_fds[i].events &= ~POLLIN;
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
	}
	if (state == FINISH || state == HEADER_FINISHED)	// HEADER_FINISHED indicates partial request
	{
		std::cout << CYAN << "Passing request from fd " << sock_fds.at(i).fd << " to server\n" << RESET;
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd]);
	}
}

void	ConnectionManager::handleCGIread(unsigned int& i)
{
	ssize_t r;
	char	buffer[BUFFER_SIZE + 1] = {0};

	std::cout << "in CGI read\n";
	r = read(sock_fds.at(i).fd, buffer, BUFFER_SIZE);
	if (r == -1)
	{
		std::cerr << "EXITING SERVER\n";
		exit(1);
	}
	else if (r == 0)
	{
		std::cout << "read 0 bytes\n";
		CGIinfo& info = cgiProcesses[sock_fds.at(i).fd];
		int	status;

		pid_t	res = waitpid(info.getPid(), &status, WNOHANG);
		std::cout << "Attempting to wait for pid: " << info.getPid() << "\n";
		std::cout << "Res: " << res << " pid: " << info.getPid() << "\n";
		if (res == info.getPid())
		{
			// the cgi pid can safely be cleaned up
			std::cout << "Script finished executing\n";
		}	// else cleanup will happen later
		info.completeResponse();	// once read returns 0, that means its all good to respond to

		// in the pollfds vector, find the client fd's index and set its event field to fd.event |= POLLOUT
		const int	client_fd = info.getClientFd();
		std::vector<struct pollfd>&	fds = this->sock_fds;
		for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
		{
			if (it->fd == client_fd)
			{
				std::cout << "Setting cgi fd " << info.getClientFd() << "'s client fd " << it->fd << " to POLLOUT\n";
				it->events |= POLLOUT;
				break ;
			}
		}
		close(sock_fds.at(i).fd);	// close the read end of the cgi pipe
		sock_fds.erase(sock_fds.begin() + i);	// remove the fd from the pollfds
		i--;
		(void)status;
	}
	else
	{
		buffer[r] = 0;
		cgiProcesses[sock_fds.at(i).fd].concatBuffer(std::string(buffer));
	}
	std::cout << buffer << "\n";
}

void	ConnectionManager::reapProcesses()
{
	for(std::map<int, CGIinfo>::iterator it = this->cgiProcesses.begin(); it != this->cgiProcesses.end(); it++)
	{
		int	s;
		if (waitpid(it->second.getPid(), &s, WNOHANG) > 0)
		{
			std::cout << "Reaping pid: " << it->second.getPid() << "\n";
			this->cgiProcesses.erase(it);
		}
	}
}

void	ConnectionManager::handleCgiPollhup(unsigned int i)
{
	int	status;

	CGIinfo& info = cgiProcesses[sock_fds.at(i).fd];
	pid_t	pid = waitpid(info.getPid(), &status, 0);
	std::cout << "pid: " << pid << "\n";
	info.completeResponse();

	const int& client_fd = info.getClientFd();
	for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
	{
		if (it->fd == client_fd)
		{
			std::cout << it->fd << " event getting OR'd with POLLOUT\n";
			it->events |= POLLOUT;
			break ;
		}
	}
}

void ConnectionManager::startConnections()
{
	int						res;
	State					state = INCOMPLETE;

	main_listeners = sock_fds.size();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handle);
	std::cout << "Server has " << main_listeners << " listeners\n";
	while (g_quit != true)
	{
		res = poll(&sock_fds[0], sock_fds.size(), -1);
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
			reapProcesses();
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					std::cout << "in CGI pollin\n";
					std::cout << "Found " << sock_fds.at(i).fd << " in the map of fds\n";
					handleCGIread(i);
				}
				else
				{
					std::cout << "in normal POLLIN\n";
					handlePollin(i, state, requests);
					continue ;
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				std::cerr << "FD: " << this->sock_fds[i].fd << " is ready for POLLOUT\n";
				if (handleCGIPollout(i) == true)
					continue ;
				handlePollout(state, i, requests);
			}
			if (sock_fds.at(i).revents & POLLHUP)
			{
				std::cout << "POLLHUP event\n";
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					std::cout << "POLLHUP CGI event\n";
					handleCgiPollhup(i);
					close(sock_fds.at(i).fd);
					sock_fds.erase(sock_fds.begin() + i);
					i--;
				}
				else
				{
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
