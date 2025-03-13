#include "Server.hpp"

const str	Server::default_ip = "127.0.0.1";
const str	Server::default_port = "80";
const str	Server::directives[] = { "root", "listen", "index", "server_name", "error_page", "client_max_body_size", "min_delete_depth", "autoindex", "return", "" };

Server::Server() : BlockOBJ()
{
	header = "";
	root = "/";
	keep_alive = false;
	file_fd = -1;
}

Server::Server(const Server &copy): BlockOBJ(copy)
{
	header = "";
	root = "/";
	keep_alive = false;
	file_fd = -1;
	(void)copy;
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
	bool			parent_ret;

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

std::vector<str>	Server::getNames()
{
	return (this->names);
}

std::vector<str>	Server::getIPs()
{
	return (this->ips);
}

std::vector<str>	Server::getPorts()
{
	return (this->ports);
}

std::vector<Location *>	Server::getLocations()
{
	return locations;
}

void Server::setDefault()
{
	names.push_back("_");
}

unsigned int Server::fileSize(int fd)
{
	unsigned int	bytes, full;
	unsigned char	buffer[4096];

	full = 0;
	while ((bytes = read(fd, buffer, 4096)) > 0)
		full += bytes;
	close(fd);
	return full;
}

str	Server::reasonPhrase(str status)
{
	if (status == "200")
		return "OK";
	if (status == "404")
		return "Page Not Found";
	if (status == "414")
		return "URI Too Long";
	if (status == "501")
		return "Not Implemented";
	if (status == "505")
		return "HTTP Version Not Supported";
	return "";
}

str	Server::errorPage(str status)
{
	if (status == "400")
		return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Code 400</h1>\r\n<p>Bad Request!</p>\r\n</body>\r\n</html>\r\n";
	if (status == "404")
		return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Code 404</h1>\r\n<p>Page not found!</p>\r\n</body>\r\n</html>\r\n";
	if (status == "414")
		return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Code 414</h1>\r\n<p>URI Too Long!</p>\r\n</body>\r\n</html>\r\n";
	if (status == "501")
		return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Code 501</h1>\r\n<p>Not Implemented!</p>\r\n</body>\r\n</html>\r\n";
	if (status == "505")
		return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Code 505</h1>\r\n<p>HTTP Version Not Supported!</p>\r\n</body>\r\n</html>\r\n";
	return "<html>\r\n<head>\r\n<title>Error Page</title>\r\n</head>\r\n<body>\r\n<h1>Error Page</h1>\r\n<p>Unknown Error Code</p>\r\n</body>\r\n</html>\r\n";
}

str	Server::fileType(str file_name)
{
	str	type;

	if (file_name == "/")
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

void Server::handleError(str error_code, std::stringstream &resp)
{
	int	fd;

	resp << "HTTP/1.1 " + error_code + " " + reasonPhrase(error_code) + "\r\n";
	resp << "Connection: Keep-Alive\r\nContent-Type: text/html\r\n";
	if (error_pages.find(error_code) == error_pages.end()
		|| (fd = open(error_pages[error_code].c_str(), O_RDONLY)) == -1)
	{
		total_length = errorPage(error_code).length() - 2;
		resp << "Content-Length: " << total_length << "\r\n\r\n";
		resp << errorPage(error_code);
		file_fd = -1;
	}
	else
	{
		total_length = fileSize(fd);
		fd = open(error_pages[error_code].c_str(), O_RDONLY);
		if (fd == -1)
		{
			total_length = errorPage(error_code).length() - 2;
			resp << "Content-Length: " << total_length << "\r\n\r\n";
			resp << errorPage(error_code);
			file_fd = -1;
		}
		else
		{
			resp << "Transfer-Encoding: Chunked\r\n\r\n";//\r\nContent-Length: " << total_length << "\r\n\r\n";
			file_fd = fd;
		}
	}
	header = resp.str();
}

void Server::handleRequest(Request *req)
{
	int					fd;
	str					file;
	std::stringstream	resp;

	keep_alive = req->shouldKeepAlive();
	if (!req->isValidRequest())
	{
		handleError(req->getStatus(), resp);
		return ;
	}
	else if (req->getMethod() == "GET")
	{
		if (req->getFileURI() == "/")
			file = root + "/index.html";
		else
			file = root + req->getFileURI();
		// fd = open(file.c_str(), O_RDONLY);
		// if (fd == -1)
		// {
		// 	handleError("404", resp);
		// 	return ;
		// }
		// total_length = fileSize(fd);
		fd = open(file.c_str(), O_RDONLY);
		if (fd == -1)
		{
			handleError("404", resp);
			return ;
		}
		file_fd = fd;
		resp << "HTTP/1.1 200 OK\r\nContent-Type: " << fileType(req->getFileURI()) << "\r\n";
		resp << "Transfer-Encoding: Chunked\r\nConnection: " << (keep_alive ? "Keep-Alive" : "close") << "\r\n\r\n";
		header = resp.str();
	}
	else if (req->getMethod() == "DELETE")
	{
		if (req->getFileURI() == "/")
			file = root + "/index.html";
		else
			file = root + req->getFileURI();
		fd = open(file.c_str(), O_RDONLY);
		if (fd == -1)
		{
			handleError("404", resp);
			return ;
		}
		total_length = fileSize(fd);
		fd = open(file.c_str(), O_RDONLY);
		if (fd == -1)
		{
			handleError("404", resp);
			return ;
		}
		file_fd = fd;
		resp << "HTTP/1.1 200 OK\r\nContent-Type: " << fileType(req->getFileURI()) << "\r\n";
		resp << "Transfer-Encoding: Chunked\r\nConnection: " << (keep_alive ? "Keep-Alive" : "close") << "\r\n\r\n";
		header = resp.str();
	}
}

bool Server::respond(int client_fd)
{
	str		tmp;
	ssize_t	bytes, sb;
	char	buffer[4096];

	if (header == "")
		return true;
	send(client_fd, header.c_str(), header.length(), 0);
	header = "";
	if (file_fd != -1)
	{
		while ((bytes = read(file_fd, buffer, 4096)) > 0)
		{
			tmp = ssizeToStr(bytes) + "\r\n";
			while (send(client_fd, tmp.c_str(), tmp.length(), 0) < 0);
			while ((sb = send(client_fd, buffer, bytes, 0)) < 0);
			while (send(client_fd, "\r\n", 2, 0) < 0);
			total_length -= sb - 2;
		}
		tmp = "0\r\n\r\n";
		while (send(client_fd, tmp.c_str(), tmp.length(), 0) < 0);
		close(file_fd);
	}
	return keep_alive;
}

bool Server::operator ==(Server &server2)
{
	std::vector<str>	tmp, tmp2, tmp3, tmp4;
	bool				same_name = false;
	bool				same_ipport = false;

	tmp = this->getNames();
	tmp2 = server2.getNames();
	for(std::vector<str>::iterator it = tmp.begin(); it != tmp.end(); it++)
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
