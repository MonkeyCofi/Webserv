#include <iostream>
#include "ConfigParser.hpp"
#include "Socket.hpp"
#include <poll.h>
#include <fstream>
#include <fcntl.h>

void	parse_request(str& request, int client)
{
	str	status_line = request.substr(0, request.find_first_of("\r\n"));
	str	method = status_line.substr(0, status_line.find_first_of(' '));
	str	http_header = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:close\r\n";
	char	buffer[4096];

	int	index;
	index = open("index.html", O_RDONLY);
	if (index == -1)
	{
		std::cout << "Couldn't open index.html\n";
		send(client, "HTTP/1.1 404 Not Found\r\n\r\n", sizeof("HTTP/1.1 404 Not Found\r\n\r\n"), 0);
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "Successfully opened index.html\n";
	send(client, http_header.c_str(), http_header.length(), 0);
	send(client, "\r\n", 2, 0);
	ssize_t	bytes = 1;
	while ((bytes = read(index, buffer, 1)) > 0)
	{
		std::cout << buffer << "\n";
		if (buffer[0] != '\n')
			send(client, buffer, 1, 0);
		else
			send(client, "\r\n", 2, 0);
	}
	close(client);
}

//void	parse_request(str& request, int client)
//{
//	str	status_line = request.substr(0, request.find_first_of("\r\n"));
//	str	method = status_line.substr(0, status_line.find_first_of(' '));
	
//}

// once request is parsed
// send a response according to the request
// for now, serve a webpage 

int main(int ac, char **av)
{
	unlink("localhost");
	if (ac != 2)
	{
		std::cerr << "Error\nInvalid number of arguments!\nUsage: ./webserv <configuration_file.conf>\n";
		return (1);
	}
	Engine	eng;
	try
	{
		ConfigParser conf;
		str		tmp = av[1];
		eng = conf.parse(tmp);
		std::cout << "Parsing done!\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
	}

	Http	*p = eng.getProtocol();
	try
	{
		p->init_listeners();	// initialize every listener for every certificate block
		p->printPortsIpsNames();
	}
	catch (Socket::BindException())
	{
		;
	}
	catch(std::out_of_range())
	{
		;
	}
	std::vector<Socket *>	listeners = p->returnSockets();

	std::vector<struct pollfd>	sock_fds;
	for (unsigned int i = 0; i < listeners.size(); i++)
	{
		struct pollfd	temp;
		// fcntl(temp.fd, )
		temp.fd = listeners.at(i)->returnSocket(i);
		temp.events = POLLIN;
		temp.revents = 0;
		sock_fds.push_back(temp);
	}

	 while (1)
	 {
	 	int res = poll(&sock_fds[0], sock_fds.size(), 100);
		if (res == -1)
		{
			perror("poll");
			exit(EXIT_FAILURE);
		}
		for (unsigned int i = 0; i < listeners.size(); i++)
		{
			// std::cout << sock_fds.at(i).revents << "\n";
			if (sock_fds.at(i).revents & POLLIN)
			{
				// Request read_request();
				struct sockaddr_in	client;
				socklen_t	len = sizeof(client);
				int acc_sock = accept(sock_fds.at(i).fd, (sockaddr *)&client, &len);
				char buf[1024];
				recv(acc_sock, buf, sizeof(buf), 0);

				std::string	req(buf);
				std::cout << req;
				parse_request(req, acc_sock);
				// close(acc_sock);

				// after accepting, parse the request and serve according to request
				// parse_request(Request &req);
			}
			else if (sock_fds.at(i).revents & POLLOUT)
				std::cout << "Fd " << sock_fds.at(i).fd << " is ready for output\n";
		}
	 }
}
