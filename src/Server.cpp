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
		// min_del_depth = std::stoi(opts.front());
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

void Server::handleError(str error_code)
{
	int	fd = open(error_pages[error_code].c_str(), O_RDONLY);

	this->response.clear();
	if (error_pages.find(error_code) == error_pages.end() || fd == -1)
	{
		if (fd != -1)
			close(fd);
		this->response.setCode(error_code);
		this->response.setBody(this->response.errorPage(error_code), "text/html");
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	// else
		// this->response.setBodyFd(fd);
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

void Server::directoryResponse(Request* req, str path)
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
			fileResponse(req, path + index.at(i), file_fd);
			return ;
		}
		else if (i != index.size() - 1)
			continue ;
		else if (!autoindex)
		{
			handleError("403");
			return ;
		}
	}
	dir = opendir(full_path.c_str());
    if (dir == NULL)
	{
		handleError("404");
		return ;
	}
	body = "<html>\r\n<head>\r\n<title>Index of " + path + "</title>\r\n</head>\r\n<body>\r\n<h1>Index of " + path + "</h1>\r\n<hr>\r\n";
	body += "<a href=\"../\">..</a> <br>\r\n";
    while ((item = readdir(dir)) != NULL)
	{
		filename = str(item->d_name);
		if (filename != "." && filename != "..")
		{
			if (isDirectory(full_path + filename))
				filename += "/";
			body += "<a href=\"./" + filename + "\">" + filename + "</a> <br>\r\n";
		}
	}
	body += "<hr></body>\r\n</html>\r\n";
	this->response.setCode((redir ? "301" : "200"));
	this->response.setKeepAlive(req->shouldKeepAlive());
	this->response.setBody(body, "text/html");
	if (redir)
		this->response.setHeaderField("Location", path);
    closedir(dir);
}

void Server::fileResponse(Request* req, str path, int file_fd)
{
	str	status = "200";

	if (file_fd != -1 && path != "/index.html")
		status = "302";
	else
	{
		path = root + path;
		// std::cout << "Serving " << path << "\n";
		file_fd = open(path.c_str(), O_RDONLY);
	}
	if (file_fd == -1)
	{
		handleError("404");
		return ;
	}
	this->response.setCode(status);
	this->response.setKeepAlive(req->shouldKeepAlive());
	this->response.setHeaderField("Content-Type", fileType(path));
	this->response.setBodyFd(file_fd);
	if (status == "302")
		this->response.setHeaderField("Location", path);
}

void Server::handleRequest(int& client_fd, Request *req, 
	std::vector<struct pollfd>& pollfds, std::map<int, int>& cgiFds)
{
	str					file;

	response.clear();
	// std::cout << RED << ((this->response.keepAlive()) == true ? "Keep connection alive" : "End connection") << NL;
	if (!req->isValidRequest())
	{
		handleError(req->getStatus());
		return ;
	}
	else if (req->getMethod() == "GET")
	{
		file = req->getFileURI();
		// pass the pollfds to the CGI handler
		if (req->getFileURI().find("cgi") != str::npos)
		{
			Cgi	cgi(req->getFileURI(), this);
			cgi.setupEnvAndRun(client_fd, req, this, pollfds, cgiFds);
		}
		else if (file.at(file.length() - 1) == '/' || isDirectory(root + file))
			directoryResponse(req, file);
		else
			fileResponse(req, file, -1);
	}
	else if (req->getMethod() == "POST")
	{
		if (req->getFileURI().find("cgi") != str::npos)
		{
			Cgi	cgi(req->getFileURI(), this);
			cgi.setupEnvAndRun(client_fd, req, this, pollfds, cgiFds);
		}
		else
		{
			if (req->getContentLen() == 0)
				return ;
			this->response.setCode("200");
			this->response.setBody("<html><h1>POSTED</h1></html>\r\n", "text/html");
		}
	}
	else if (req->getMethod() == "DELETE")
	{
		// count = 0;
		// for (unsigned int i = 0; i < req->getFileURI(); i++)
		// 	count += (req->getFileURI());
		// if (count < min_del_depth)
		// {
		// 	handleError()
		// }
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

bool Server::respond(int client_fd)
{
	bool	ret;
	int		file_fd;
	str		tmp, header;
	ssize_t	bytes;
	char	buffer[BUFFER_SIZE + 1];

	if (this->response.doneSending())
		return this->response.keepAlive();
	else
		this->response.printResponse();
	file_fd = this->response.getBodyFd();
	header = this->response.getHeader();
	if (!this->response.isChunked())
		header += this->response.getBody();
	std::cout << MAGENTA << "CLIENT FD MF: " << client_fd << " PLEASE WORK!\n" << RESET;
	if (!this->response.headerSent() && send(client_fd, header.c_str(), header.length(), 0) <= 0)
		return this->response.keepAlive();
	this->response.setHeaderSent(true);
	ret = this->response.keepAlive();
	if (!this->response.isChunked())
		std::cout << BLUE << "Done responding!\n" << RESET;
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
			this->response.setBodyFd(-1);
			tmp = "0\r\n\r\n";
			if (send(client_fd, tmp.c_str(), tmp.length(), 0) <= 0)	// send the ending byte to the client
			{
				std::cout << "Pierce is wrong, I am wrong as well\n";
				return (this->response.keepAlive());
			}
	
			return (this->response.keepAlive());
		}
		// this->response.setBody(buffer, "");
		std::cout << "buffer: " << buffer << "\n";
		std::cout << "Bytes: " << bytes << "\n";
		// std::cout << "Sending " << buffer << "\n";
		tmp = ssizeToStr(bytes) + "\r\n";
		// std::cout << tmp << " bytes\n";
		std::cout << "--------------================\n";
		std::cout << tmp << "\n";
		if (send(client_fd, tmp.c_str(), tmp.length(), 0) <= 0)
		{
			std::cout << "123WHAFSJDKLASJKLFAJSDKLFALSKDF\n";
			return (ret);
		}

		std::cout << "--------------================\n";
		std::cout << buffer << ", " << bytes << "\n";
		if (send(client_fd, buffer, bytes, 0) <= 0)
		{
			std::cout << "456WHAFSJDKLASJKLFAJSDKLFALSKDF\n";
			return (ret);
		}

		if (send(client_fd, "\r\n", 2, 0) <= 0)
		{
			std::cout << "789WHAFSJDKLASJKLFAJSDKLFALSKDF\n";
			return (ret);
		}

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
