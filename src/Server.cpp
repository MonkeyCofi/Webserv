#include "Server.hpp"
#include "Cgi.hpp"

const str	Server::default_ip = "127.0.0.1";
const str	Server::default_port = "80";
const str	Server::directives[] = { "root", "listen", "index", "server_name", "error_page", "client_max_body_size", "min_delete_depth", "autoindex", "return", "" };

Server::Server() : BlockOBJ()
{
	root = "/";
	index.push_back("index.html");
	autoindex = false;
	min_del_depth = 0;
	sent_bytes = 0;
}

Server::Server(const Server &copy): BlockOBJ(copy)
{
	root = "/";
	index.push_back("index.html");
	autoindex = false;
	min_del_depth = 0;
	(void)copy;
	sent_bytes = 0;
}

Server::~Server()
{
	for(std::vector<Location *>::iterator it = locations.begin(); it != locations.end(); it++)
		delete *it;
}

bool Server::validAddress(str address)
{
	unsigned int	dot_count = 0;
	bool			colon = false;

	for (unsigned int i = 0; i < address.length(); i++)
	{
		if (!std::isdigit(address[i]) && address[i] != '.' && address[i] != ':')
			return false;
		else if (address[i] == '.')
		{
			dot_count++;
			if (i == 0 || i == address.length() - 1 || !std::isdigit(address[i - 1]) || !std::isdigit(address[i + 1]))
				return false;
		}
		else if (address[i] == ':')
		{
			colon = true;
			if (i == 0 || i == address.length() - 1 || dot_count < 3 || !std::isdigit(address[i - 1]) || !std::isdigit(address[i + 1]))
				return false;
		}
	}
	if (dot_count != 3 && (dot_count != 0 || colon))
		return false;
	return true;
}

bool Server::handleAddress(str address)
{
	int	last_colon;

	if (!validAddress(address))
		return false;
	if (address.find(":") == str::npos)
	{
		if (address.find_first_of('.') == std::string::npos)
		{
			ips.push_back(Server::default_ip);
			ports.push_back(address);
		}
		else
		{
			ips.push_back(address);
			ports.push_back(Server::default_port);
		}
	}
	else
	{
		last_colon = address.find(":");
		ips.push_back(address.substr(0, last_colon));
		ports.push_back(address.substr(last_colon + 1));
	}
	return true;
}

bool Server::handleDirective(std::queue<str> opts)
{
	bool	parent_ret;
	size_t	pos;

	if (opts.size() == 0 || !inDirectives(opts.front(), directives))
		return false;
	parent_ret = BlockOBJ::handleDirective(opts);
	if (opts.front() == "listen" && opts.size() == 2)
	{
		opts.pop();
		return (handleAddress(opts.front()));
	}
	else if (opts.front() == "root" && opts.size() == 2)
	{
		opts.pop();
		if (opts.front().length() == 0 || opts.front().at(0) != '/')
			return false;
		root = opts.front();
		if (root.at(root.length() - 1) == '/')
			root = root.substr(0, root.length() - 1);
		pos = root.find("%20");
		while (pos != str::npos)
		{
			root.replace(pos, 3, " ");
			pos += 3;
			pos = root.find("%20", pos);
		}
	}
	else if (opts.front() == "index" && opts.size() >= 2)
	{
		opts.pop();
		while (!opts.empty())
		{
			if (opts.front() == "index.html")
			{
				opts.pop();
				continue ;
			}
			index.push_back(opts.front());
			if (index.back().at(index.back().length() - 1) == '/' || index.back().at(0) == '/')
				return false;
			pos = index.back().find("%20");
			while (pos != str::npos)
			{
				index.back().replace(pos, 3, " ");
				pos += 3;
				pos = index.back().find("%20", pos);
			}
			opts.pop();
		}
	}
	else if (opts.front() == "server_name" && opts.size() >= 2)
	{
		opts.pop();
		while (opts.size() > 0)
		{
			if (opts.front().length() == 0)
				return false;
			names.push_back(opts.front());
			opts.pop();
		}
	}
	else if (opts.front() == "error_page" && opts.size() >= 3)
	{
		opts.pop();
		while (opts.size() > 1)
		{
			if (opts.front().length() != 3 || opts.front().find_first_not_of("0123456789") != str::npos)
				return false;
			error_pages[opts.front()] = opts.back();
			opts.pop();
		}
	}
	else if (opts.front() == "autoindex" && opts.size() == 2)
	{
		opts.pop();
		if (opts.front() != "off" && opts.front() != "on")
			return false;
		autoindex = (opts.front() == "on");
		opts.pop();
	}
	else if (opts.front() == "min_delete_depth" && opts.size() == 2)
	{
		opts.pop();
		if (opts.front().length() > 10)
			return false;
		for (unsigned int i = 0; i < opts.front().length(); i++)
		{
			if (opts.front().at(i) < '0' || opts.front().at(i) > '9')
				return false;
		}
		min_del_depth = std::atoi(opts.front().c_str());
		opts.pop();
	}
	else
		return parent_ret;
	return true;
}

BlockOBJ *Server::handleBlock(std::queue<str> opts)
{
	if (opts.size() < 2 || opts.front() != "location")
		return NULL;
	locations.push_back(new Location());
	return locations.back();
}

const Server &Server::operator =(const Server &copy)
{
	(void)copy;
	return *this;
}

str	Server::getPort(int index)
{
	if ((unsigned int) index > ports.size())
		return ("out of range");
	if (ports.empty())
		return ("empty");
	return (ports.at(index));
}

str	Server::getIP(int index)
{
	if ((unsigned int) index > ips.size())
		return ("out of range");
	if (ips.empty())
		return ("empty");
	return (ips.at(index));
}

str	Server::getType()
{
	return ("server");
}

std::vector<str> Server::getNames()
{
	return (this->names);
}

std::vector<str> Server::getIPs()
{
	return (this->ips);
}

std::vector<str> Server::getPorts()
{
	return (this->ports);
}

std::vector<Location *> Server::getLocations()
{
	return locations;
}

void Server::setDefault()
{
	names.push_back("_");
}

str	Server::fileType(str file_name)
{
	str	type;

	if (file_name.at(file_name.length() - 1) == '/')
		return "text/html";
	if (file_name.find(".") == str::npos || file_name.length() - file_name.find_last_of(".") > 5)
		return "text/plain";
	type = file_name.substr(file_name.find_last_of(".") + 1);
	if (type == "jpg" || type == "svg" || type == "jpeg" || type == "png" || type == "gif" || type == "avif" || type == "webp" || type == "ico" || type == "tiff")
		return "image/" + type;
	else if (type == "mp4" || type == "vod")
		return "video/" + type;
	else if (type == "m4a" || type == "mp3")
		return "sound/" + type;
	return "text/" + type;
}

str	Server::ssizeToStr(ssize_t x)
{
	std::stringstream s;
	s << std::hex << x;
	return s.str();
}

void Server::handleError(str error_code, int client_fd)
{
	bool	keep;
	int		fd;

	keep = this->response[client_fd].keepAlive();
	this->response[client_fd].clear();
	this->response[client_fd].setCode(error_code);
	this->response[client_fd].setKeepAlive(keep);
	if (error_pages.find(error_code) == error_pages.end() || (fd = open(error_pages[error_code].c_str(), O_RDONLY)) == -1)
	{
		this->response[client_fd].setBody(this->response[client_fd].errorPage(error_code), "text/html");
		return ;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	this->response[client_fd].setBodyFd(fd);
}

str	Server::getRoot() const
{
	return (this->root);
}

// void Server::getInfo(str &path)
// {
//     struct stat s;
//     if (stat(path.c_str(), &s) == 0) {
//         // Check if it's a directory
//         if (S_ISDIR(s.st_mode)) {
//             std::cout << path << " is a directory.\n";
//         } else if (S_ISREG(s.st_mode)) {
//             std::cout << path << " is a file.\n";

//             // Print size
//             std::cout << "Size: " << s.st_size << " bytes\n";

//             // Print last modified date
//             std::cout << "Last modified: " << std::ctime(&s.st_mtime);
//         } else {
//             std::cout << path << " exists but is not a regular file or directory.\n";
//         }
//     } else {
//         std::perror("stat");
//         std::cout << "Could not access path: " << path << "\n";
//     }
// }

bool Server::isDirectory(const std::string& path)
{
	struct stat	s;
	
    if (stat(path.c_str(), &s) == 0)
		return S_ISDIR(s.st_mode);
	return false;
}

void Server::directoryResponse(Request* req, str path, int client_fd)
{
	str				indexpath, full_path, filename, body;
    DIR				*dir;
	int				file_fd;
	bool			redir;
    struct dirent	*item;

	redir = false;
	if (path.at(path.length() - 1) != '/')
	{
		path += "/";
		redir = true;
	}
	full_path = root + path;
	std::cout << RED << 1 << NL;
	for (unsigned int i = 0; i < this->index.size(); i++)
	{
		indexpath = full_path + index.at(i);
		file_fd = open(indexpath.c_str(), O_RDONLY);
		if (file_fd > -1)
		{
			std::cout << GREEN << "FileREsponsing In th3 d1r3ct0ry!\n" << RESET;
			fileResponse(req, path + index.at(i), file_fd, client_fd);
			return ;
		}
		else if (i != index.size() - 1)
			continue ;
		else if (!autoindex)
		{
			handleError("403", client_fd);
			return ;
		}
	}
	dir = opendir(full_path.c_str());
    if (dir == NULL)
	{
		handleError("500", client_fd);
		return ;
	}
	body = "<html><head><title>Index of " + path + "</title></head><body><h1>Index of " + path + "</h1><hr>";
	body += "<a href=\"../\">..</a> <br>";
    while ((item = readdir(dir)) != NULL)
	{
		filename = str(item->d_name);
		if (filename != "." && filename != "..")
		{
			if (isDirectory(full_path + filename))
				filename += "/";
			body += "<a href=\"./" + filename + "\">" + filename + "</a> <br>";
		}
	}
	body += "<hr></body></html>\r\n";
	this->response[client_fd].setCode((redir ? "301" : "200"));
	this->response[client_fd].setKeepAlive(req->shouldKeepAlive());
	this->response[client_fd].setBody(body, "text/html");
	if (redir)
		this->response[client_fd].setHeaderField("Location", path);
    closedir(dir);
}

void Server::fileResponse(Request* req, str path, int file_fd, int client_fd)
{
	str	status = "200";

	if (file_fd != -1 && path != "/index.html")
		status = "302";
	else
	{
		path = root + path;
		file_fd = open(path.c_str(), O_RDONLY);
	}
	if (file_fd == -1)
	{
		handleError("404", client_fd);
		return ;
	}
	this->response[client_fd].setCode(status);
	this->response[client_fd].setKeepAlive(req->shouldKeepAlive());
	this->response[client_fd].setHeaderField("Content-Type", fileType(path));
	this->response[client_fd].setBodyFd(file_fd);
	if (status == "302")
		this->response[client_fd].setHeaderField("Location", path);
}

void Server::handleRequest(int& i, int& client_fd, Request *req, 
	std::vector<struct pollfd>& pollfds, std::map<int, int>& cgiFds, std::map<int, CGIinfo>& cgiProcesses)
{
	struct stat 	s;
	struct dirent*	entry;
	unsigned int	len, count;
	str				file, uri;
	DIR* 			dir;

	if (response.find(client_fd) == response.end())
		response[client_fd] = Response();
	response[client_fd].clear();
	pollfds.at(i).events |= POLLOUT;
	// std::cout << RED << ((this->response.keepAlive()) == true ? "Keep connection alive" : "End connection") << NL;
	if (!req->isValidRequest())
	{
		handleError(req->getStatus(), client_fd);
		return ;
	}
	else if (req->getMethod() == "GET")
	{
		file = req->getFileURI();
		// pass the pollfds to the CGI handler
		if (req->getFileURI().find("cgi") != str::npos)
		{
			Cgi	cgi(req->getFileURI(), this);
			cgi.setupEnvAndRun(client_fd, req, this, pollfds, cgiFds, cgiProcesses);
		}
		else if (file.at(file.length() - 1) == '/' || isDirectory(root + file))
			directoryResponse(req, file, client_fd);
		else
			fileResponse(req, file, -1, client_fd);
	}
	else if (req->getMethod() == "POST")
	{
		if (req->getFileURI().find("cgi") != str::npos)
		{
			Cgi	cgi(req->getFileURI(), this);
			cgi.setupEnvAndRun(client_fd, req, this, pollfds, cgiFds, cgiProcesses);
		}
		else
		{
			if (req->getContentLen() == 0)
				return ;
			this->response[client_fd].setCode("200");
			this->response[client_fd].setBody("<html><h1>POSTED</h1></html>\r\n", "text/html");
		}
	}
	else if (req->getMethod() == "DELETE")
	{
		uri = req->getFileURI();
		len = uri.length();
		count = -1;
		for (unsigned int i = 0; i < len; i++)
			count += (uri.at(i) == '/');
		count -= (uri.at(len - 1) == '/' && len > 1);
		if (count < min_del_depth)
		{
			handleError("409", client_fd);
			return ;
		}
		uri = root + uri;
		if (stat(uri.c_str(), &s) == 0) 
		{
			// Check if it's a directory
			if (S_ISDIR(s.st_mode))
			{
				dir = opendir(uri.c_str());
				if (!dir)
				{
					handleError("500", client_fd);
					return ;
				}
				count = 0;
				while ((entry = readdir(dir)) != NULL) {
					if (++count > 2) {
						break;
					}
				}
				closedir(dir);
				if (count > 2)
				{
					handleError("409", client_fd);
					return ;
				}
				if (rmdir(uri.c_str()) == -1)
				{
					handleError("500", client_fd);
					return ;
				}
				this->response[client_fd].setCode("204");
				this->response[client_fd].setKeepAlive(req->shouldKeepAlive());
			}
			else if (S_ISREG(s.st_mode))
			{
				if (remove(uri.c_str()) == -1)
				{
					handleError("500", client_fd);
					return ;
				}
				this->response[client_fd].setCode("204");
				this->response[client_fd].setKeepAlive(req->shouldKeepAlive());
			}
			else
			{
				handleError("400", client_fd);
				return ;
			}
		}
		else
		{
			handleError("404", client_fd);
			return ;
		}
	}
}

void	Server::setState(Server::ResponseState state)
{
	this->responseState = state;
}

Server::ResponseState	Server::getState() const
{
	return (this->responseState);
}

bool	Server::cgiRespond(CGIinfo* infoPtr)
{
	std::cout << "In cgi respond function\n";
	const int&	client_fd = infoPtr->getClientFd();
	if (this->response.find(client_fd) == this->response.end())
	{
		std::cout << "adding cgi response object to map of responses\n";
		this->response[client_fd] = infoPtr->parseCgiResponse();
	}
	else
	{
		if (this->response[client_fd].getHeader().empty())
		{
			this->response[client_fd] = infoPtr->parseCgiResponse();
		}
		this->response[client_fd].printResponse();
		std::cout << "Sending cgi response object's response to client fd " << client_fd << "\n";
		respond(client_fd);
	}
	return (true);
}

bool Server::respond(int client_fd)
{
	bool	ret;
	int		file_fd;
	str		tmp, header;
	ssize_t	bytes, tw;
	char	buffer[BUFFER_SIZE + 1];

	if (this->response[client_fd].doneSending())
		return this->response[client_fd].keepAlive();
	else
		this->response[client_fd].printResponse();
	file_fd = this->response[client_fd].getBodyFd();
	header = this->response[client_fd].getHeader();
	if (!this->response[client_fd].isChunked())
		header += this->response[client_fd].getBody();
	if (!this->response[client_fd].headerSent())
	{
		if ((tw = send(client_fd, header.c_str(), header.length(), 0)) <= 0)
			return this->response[client_fd].keepAlive();
		std::cout << "--------\n";
		std::cout << "Sent bytes: " << tw << " to fd " << client_fd << "\n";
		std::cout << "--------\n";
		this->response[client_fd].setHeaderSent(true);
		std::cout << RED << "Header sent hellbent!\n";
	}
	ret = this->response[client_fd].keepAlive();
	if (!this->response[client_fd].isChunked())
		std::cout << BLUE << "Done responding!\n\nSent:\n" << header << "\n\n" << RESET;
	else
	{
		std::cout << BLUE << "Done sending header!\n" << RESET;
		bytes = read(file_fd, buffer, BUFFER_SIZE);
		if (bytes == -1)
		{
			std::cout << "Read returned -1 for reading response body\n";
			perror("Read");
			return (false);
		}
		std::cout << "Read " << bytes << " from body fd\n";
		buffer[bytes] = 0;
		sent_bytes += bytes;
		if (bytes == 0)	// file has been fully read and responded with
		{
			std::cout << "Finished reading response from body file\n";
			setState(FINISH);
			close(file_fd);
			this->response[client_fd].setBodyFd(-1);
			tmp = "0\r\n\r\n";
			if ((tw = send(client_fd, tmp.c_str(), tmp.length(), 0)) <= 0)	// send the ending byte to the client
				return (this->response[client_fd].keepAlive());
			std::cout << "--------\n";
			std::cout << "Sent bytes: " << tw << " to fd " << client_fd << "\n";
			std::cout << "--------\n";
			return (ret);
		}
		tmp = ssizeToStr(bytes) + "\r\n";
		if ((tw = send(client_fd, tmp.c_str(), tmp.length(), 0)) <= 0)
			return (ret);
		std::cout << "--------\n";
		std::cout << "Sent bytes: " << tw << " to fd " << client_fd << "\n";
		std::cout << "--------\n";
		if ((tw = send(client_fd, buffer, bytes, 0)) <= 0)
			return (ret);
		std::cout << "--------\n";
		std::cout << "Sent bytes: " << tw << " to fd " << client_fd << "\n";
		std::cout << "--------\n";
		if ((tw = send(client_fd, "\r\n", 2, 0)) <= 0)
			return (ret);
		std::cout << "--------\n";
		std::cout << "Sent bytes: " << tw << " to fd " << client_fd << "\n";
		std::cout << "--------\n";
	}
	return ret;
}

bool Server::operator ==(Server &server2)
{
	std::vector<str>	tmp, tmp2, tmp3, tmp4;
	bool				same_name = false;
	bool				same_ipport = false;

	tmp = this->getNames();
	tmp2 = server2.getNames();
	for (std::vector<str>::iterator it = tmp.begin(); it != tmp.end(); it++)
	{
		if (std::find(tmp2.begin(), tmp2.end(), *it) != tmp2.end())
		{
			same_name = true;
			break;
		}
	}
	tmp = this->getIPs();
	tmp2 = server2.getIPs();
	tmp3 = this->getPorts();
	tmp4 = server2.getPorts();
	for (unsigned int i = 0; i < tmp.size(); i++)
	{
		for (unsigned int j = 0; j < tmp2.size(); j++)
		{
			if (tmp[i] == tmp2[j] && tmp3[i] == tmp4[j])
			{
				same_ipport = true;
				break;
			}
		}
	}
	return same_name && same_ipport;
}

// new functions

/********************/
/*		Setters		*/
/********************/
// void	Server::setHeader(str header_)
// {
// 	this->header = header_;
// }

// void	Server::setBody(str body_)
// {
// 	this->body.append(body_);
// }

// void	Server::setFileFD(int fd_)
// {
// 	this->file_fd = fd_;
// }

Server::ResponseState	Server::returnIncomplete()
{
	return (Server::INCOMPLETE);
}

Server::ResponseState	Server::returnFinish()
{
	return (Server::FINISH);
}
