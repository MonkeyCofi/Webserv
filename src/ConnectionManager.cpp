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
void ConnectionManager::passRequestToServer(int i, Request **req, 
	std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses)
{
	// std::cout << "servers per ippp size: " << servers_per_ippp.size() << " i: " << i << "\n";
	if (servers_per_ippp[i].find((*req)->getHost()) == servers_per_ippp[i].end())
		handlers.at(i) = defaults.at(i);
	else if((*req)->isValidRequest())
		handlers.at(i) = servers_per_ippp[i][(*req)->getHost()];
	if ((*req)->isValidRequest())
		handlers.at(i)->handleRequest(i, sock_fds.at(i).fd, *req, pollfds, cgiProcesses);
	// delete *req;
	// *req = NULL;
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
		Request* temp = requests[sock_fds[index].fd];
		requests.erase(sock_fds.at(index).fd);
		delete temp;
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

// void	ConnectionManager::handleCGIfd()
// {
	
// }

void	ConnectionManager::openTempFile(Request* req, std::fstream& file)
{
	// the filename is stored in the request object
	// store the temp files in a vector
	// when the server is closed, delete every file in the vector
	if (file.is_open() == false)	// open temp file for reading message body
	{

		char uniqueName[255] = {0};
		strcpy(uniqueName, ".tempXXXXXX");
		int tmp = mkstemp(uniqueName);
		str filename = str(uniqueName);
		req->setTempFileName(filename);
		// req->tempFileName = filename;

		std::cout << "Trying to open with filename " << filename << "\n";
		close(tmp);
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
				name = line.substr(fileNamePos + 10);
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

ConnectionManager::State	ConnectionManager::receiveRequest(int client_fd, Request* req, unsigned int& index, State& state)
{
	char			buffer[BUFFER_SIZE + 1];
	ssize_t			r;
	std::string		_request;
	std::fstream&	file = req->getBodyFile();

	r = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (r < 0)
		return (INCOMPLETE);
	if (r == 0)
	{
		// std::cout << "\033[31mRecv returned " << r << "\033[0m\n";
		// std::cout << MAGENTA << "FD: " << client_fd << "\nIndex: " << index << RESET << "\n";
		closeSocket(index);
		return (INVALID);
	}
	buffer[r] = 0;
	_request = buffer;
	req->pushRequest(_request);
	if ((_request.find("\r\n\r\n") != std::string::npos && req->getHeaderReceived() == false))	// the buffer contains the end of the request header
	{
		std::cout << MAGENTA << "Found end of header for fd " << client_fd << NL;
		str rq = req->getRequest();
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
				req->clearVector();
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
			return (INCOMPLETE);
		}
	}
	(void)state;
	return (INCOMPLETE);
}

/// @brief handles all pollout events
/// @param state the current state of the response
/// @param i the index of the handling server
/// @param requests the map of requests wherein the socket fd is the key and the request object is the value
void	ConnectionManager:: handlePollout(State& state, unsigned int& i, std::map<int, Request *> &requests)
{
	Server* handler = handlers[i];

	if (handler && state == FINISH)	// the request has been parsed and ready for response building
	{
		bool keep_open = handler->respond(sock_fds[i].fd);
		if (keep_open && handler->getState() == Server::returnFinish())
		{
			sock_fds.at(i).events &= ~POLLOUT;
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
			std::cout << "Closing client socket fd " << sock_fds[i].fd << "\n";
			closeSocket(i);
			std::cout << "Closed socket\n";
			return ;
		}
		handler->setState(Server::returnIncomplete());
	}
}

bool	ConnectionManager::handleCGIPollout(unsigned int& i, std::map<int, CGIinfo>& cgiProcesses)
{
	CGIinfo* infoPtr = NULL;
	int	pipe_fd = -1;

	std::cout << "in CGI pollout\n";
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
	{
		// std::cout << "There is no complete CGI response\n";
		return (false);
	}

	/* 
		the handler is the server object that would contain the response map where: [key] = client_fd, 
		[value] = response object once there is a client fd that is ready for pollout and it is 
		a client of a cgi request, respond to the request once the client has been responded to, 
		remove the cgi object from the ma of cgi processes
	*/
	Server* handler = handlers[i];

	if (handler->cgiRespond(infoPtr) == true)	// if true, remove the cgiProcess from the map
	{
		std::cout << "Closing client socket fd " << this->sock_fds[i].fd << "\n";
		closeSocket(i);
		std::cout << "Closed socket\n";
		cgiProcesses.erase(pipe_fd);
	}
	return (true);
}

void	ConnectionManager::handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests,
	std::map<int, CGIinfo>& cgiProcesses)
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
		this->passRequestToServer(i, &requests[sock_fds.at(i).fd], sock_fds, cgiProcesses);
		std::cout << "Passing request from fd " << sock_fds.at(i).fd << " to server\n";
	}
	//Print here and inside of receiveRequest to know where it's stopping
}

void	ConnectionManager::handleCGIread(unsigned int& i, std::map<int, CGIinfo>& cgiProcesses)
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

		info.printInfo();
		// pid_t	res = waitpid(info.getPid(), &status, WNOHANG);
		// std::cout << "Attempting to wait for pid: " << info.getPid() << "\n";
		// std::cout << "Res: " << res << " pid: " << info.getPid() << "\n";
		// if (res == info.getPid())
		// {
		// 	std::cout << "Script finished executing\n";
		// 	info.completeResponse();
		// }
		// kill(info.getPid(), SIGQUIT);
		// in the pollfds vector, find the client fd's index and set its event field to fd.event |= POLLOUT
		const int	client_fd = info.getClientFd();
		std::vector<struct pollfd>&	fds = this->sock_fds;
		std::vector<struct pollfd>::iterator client_it = fds.end();
		for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
		{
			if (it->fd == client_fd)
			{
				client_it = it;
				break ;
			}
		}
		if (client_it != fds.end())
			client_it->events |= POLLOUT;
		info.completeResponse();
		close(sock_fds.at(i).fd);
		sock_fds.erase(sock_fds.begin() + i);
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

void ConnectionManager::startConnections()
{
	int						res;
	State					state = INCOMPLETE;
	std::map<int, CGIinfo>	cgiProcesses;	// key is read_end, value is CGIinfo object

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
			if (sock_fds.at(i).revents == 0)
				continue ;
			if (sock_fds.at(i).revents & POLLIN)
			{
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					std::cout << "Found " << sock_fds.at(i).fd << " in the map of fds\n";
					handleCGIread(i, cgiProcesses);
				}
				else
				{
					handlePollin(i, state, requests, cgiProcesses);
					continue ;
				}
			}
			if (sock_fds.at(i).revents & POLLOUT)
			{
				if (handleCGIPollout(i, cgiProcesses) == true)
					continue ;
				handlePollout(state, i, requests);
			}
			std::cout << "servers_per_ippp size: " << servers_per_ippp.size() << "\n";
			std::cout << "handlers size: " << handlers.size() << "\n";
			std::cout << "defaults size: " << defaults.size() << "\n";
			if (sock_fds.at(i).revents & POLLHUP)
			{
				std::cout << "POLLHUP event\n";
				if (cgiProcesses.find(sock_fds.at(i).fd) != cgiProcesses.end())
				{
					CGIinfo& info = cgiProcesses[sock_fds.at(i).fd];
					// kill the process
					int	status;
					info.printInfo();
					pid_t	pid = waitpid(info.getPid(), &status, 0);
					std::cout << "pid: " << pid << "\n";
					info.completeResponse();
					const int& client_fd = info.getClientFd();
					std::vector<struct pollfd>::iterator client_it = this->sock_fds.end();
					for (std::vector<struct pollfd>::iterator it = this->sock_fds.begin(); it != this->sock_fds.end(); it++)
					{
						if (it->fd == client_fd)
						{
							client_it = it;
							break ;
						}
					}
					if (client_it != this->sock_fds.end())
					{
						std::cout << client_it->fd << " event getting OR'd with POLLOUT\n";
						client_it->events |= POLLOUT;
					}
					// closeSocket(i);
					close(sock_fds.at(i).fd);
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
