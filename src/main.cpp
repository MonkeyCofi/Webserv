#include <iostream>
#include "ConfigParser.hpp"
#include "Socket.hpp"
#include <poll.h>
#include <signal.h>
#include <fstream>
#include <fcntl.h>
#include "Request.hpp"

void	parse_request(str& request, int id, std::vector<struct pollfd> &pauldexter)//, std::vector<std::string> &reqs)
{
	char	buffer[4096];

	str	status_line = request.substr(0, request.find_first_of("\r\n"));
	str	method = status_line.substr(0, status_line.find_first_of(' '));
	str	http_header;

	if (request.find("text/html") == str::npos)
	{
		//if (request.find("text"))
		if (request.find("text/css") == str::npos)
			http_header = "HTTP/1.1 200 OK\r\nContent-Type:image/jpeg\r\nConnection:close\r\n";
		else
			http_header = "HTTP/1.1 200 OK\r\nContent-Type:text/css\r\nConnection:close\r\n";
	}
	else
		http_header = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:close\r\n";
	str	get_file = "none";
	if (method == "GET")
	{
		get_file = status_line.substr(status_line.find_first_of(' ') + 1);
		get_file = get_file.substr(0, get_file.find_first_of(' '));
		if (get_file.at(0) == '/' && get_file.length() == 1)
			get_file = "none";
		else
		{
			std::string::iterator	it = get_file.begin();
			get_file.erase(it);
		}
		std::cout << "The file to get is " << (get_file == "none" ? "index" : get_file) << "\n";
	}
	int	index;
	index = open(get_file == "none" ? "index.html" : get_file.c_str(), O_RDONLY);
	if (index == -1)
	{
		std::cout << "Couldn't open " << (get_file == "none" ? "index.html" : get_file) << "\n";
		send(pauldexter.at(id).fd, "HTTP/1.1 403 Not Found\r\n\r\n", sizeof("HTTP/1.1 403 Not Found\r\n\r\n"), 0);
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "Successfully opened " << (get_file == "none" ? "index.html" : get_file) << "\n";
	send(pauldexter.at(id).fd, http_header.c_str(), http_header.length(), 0);
	send(pauldexter.at(id).fd, "\r\n", 2, 0);
	ssize_t	bytes = 1;
	while ((bytes = read(index, buffer, 1)) > 0)
	{
		send(pauldexter.at(id).fd, buffer, 1, 0);
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			std::cout << "send issue\n";
	}
	// close(pauldexter.at(id).fd);
	// pauldexter.erase(pauldexter.begin() + id);
	// reqs.erase(reqs.begin() + id);
	// (void)reqs;
}

int main(int ac, char **av)
{
	unlink("localhost");
	if (ac != 2)
	{
		std::cerr << "Error\nInvalid number of arguments!\nUsage: ./webserv <configuration_file.conf>\n";
		return (1);
	}
	Engine	*eng = NULL;
	try
	{
		ConfigParser conf;
		str		tmp = av[1];
		eng = conf.parse(tmp);
		if (!eng)
			throw std::exception();
		std::cout << "Parsing done!\n";
		eng->start();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
		if (eng)
			delete eng;
		exit(1);
	}
	delete eng;
}
