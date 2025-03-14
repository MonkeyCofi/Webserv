#include <iostream>
#include "ConfigParser.hpp"
#include <poll.h>
#include <signal.h>
#include <fstream>
#include <fcntl.h>
#include "Request.hpp"

// bool	g_quit = false;

// void	sigint_handle(int signal)
// {
// 	if (signal == SIGINT)
// 		g_quit = true;
// }

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
	size_t	bytes = 1;
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

// void	parse_request(Request& request)
// {
// 	char	buffer[4096];

// 	str	http_header = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:close\r\n";
// 	send(request.getFileFD(), http_header.c_str(), http_header.length(), 0);
// 	send(request.getFileFD(), "\r\n", 2, 0);
// 	size_t	bytes = 1;
// 	while ((bytes = read(request.getFileFD(), buffer, 1)) > 0)
// 	{
// 		//std::cout << buffer << "\n";
// 		if (buffer[0] != '\n')
// 			send(request.getFileFD(), buffer, 1, 0);
// 		else
// 			send(request.getFileFD(), "\r\n", 2, 0);
// 	}
// 	// close(client);
// }

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
			throw std::runtime_error();
		std::cout << "Parsing done!\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
		if (eng)
			delete eng;
		exit(1);
	}
	try
	{
		eng->start();
		// p->printPortsIpsNames();
	}
	catch (Socket::BindException())
	{
		if (eng)
			delete eng;
	}
	catch(std::out_of_range())
	{
		if (eng)
			delete eng;
	}
	delete eng;
}
	// std::vector<Socket *>	listeners = p->getListeners();

	// std::vector<struct pollfd>	sock_fds;
	// for (unsigned int i = 0; i < listeners.size(); i++)
	// {
	// 	struct pollfd	temp;
	// 	// fcntl(temp.fd, )
	// 	temp.fd = listeners.at(i)->returnSocket(i);
	// 	fcntl(temp.fd, F_SETFL, fcntl(temp.fd, F_GETFL) | O_NONBLOCK);
	// 	fcntl(temp.fd, F_SETFD, fcntl(temp.fd, F_GETFD) | FD_CLOEXEC);
	// 	temp.events = POLLIN | POLL_PRI;
	// 	temp.revents = 0;
	// 	sock_fds.push_back(temp);
	// }
	// std::vector<std::string>	reqs;
	// reqs.push_back("");
	// signal(SIGINT, sigint_handle);
	// // while (g_quit != true)
	// // {
	// //  	int res = poll(&sock_fds[0], sock_fds.size(), 500);
	// // 	std::cout << "Number of sockets " << sock_fds.size() << "\n";
	// // 	if (res < 0)
	// // 	{
	// // 		std::cout << "TEST\n";
	// // 		perror("poll");
	// // 		exit(EXIT_FAILURE);
	// // 	}
	// // 	if (res == 0)
	// // 		continue ;
	// // 	if (sock_fds.at(0).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
	// // 	{
	// // 		struct pollfd		cli;
	// // 		struct sockaddr_in	client;
	// // 		socklen_t			len;
	// // 		int					acc_sock;

	// // 		len = sizeof(client);
	// // 		acc_sock = accept(sock_fds.at(0).fd, (sockaddr *)&client, &len);
	// // 		if (errno == EWOULDBLOCK)
	// // 		{
	// // 			std::cout << "Nothing to accept\n";
	// // 			continue ;
	// // 		}
	// // 		std::cout << "Received a request from a new client\n";
	// // 		cli.fd = acc_sock;
	// // 		fcntl(cli.fd, F_SETFL, fcntl(cli.fd, F_GETFL) | O_NONBLOCK);
	// // 		fcntl(cli.fd, F_SETFD, fcntl(cli.fd, F_GETFD) | FD_CLOEXEC);
	// // 		cli.events = POLLIN | POLLOUT;
	// // 		cli.revents = 0;
	// // 		sock_fds.push_back(cli);
	// // 		reqs.push_back("");
	// // 	}
	// // 	// std::cout << "Number of sockets in poll(): " << sock_fds.size() << "\n";
	// // 	for (unsigned int i = 1; i < sock_fds.size(); i++)
	// // 	{
	// // 		if (sock_fds.at(i).revents == 0)
	// // 			continue ;
	// // 		if (sock_fds.at(i).revents & POLLHUP)
	// // 		{
	// // 			std::cout << "Hangup\n";
	// // 			// if (i < listeners.size())
	// // 			// {
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				reqs.erase(reqs.begin() + i);
	// // 				i--;
	// // 			// }
	// // 			continue ;
	// // 		}
	// // 		if (sock_fds.at(i).revents & POLLERR)
	// // 		{
	// // 			std::cout << "Error\n";
	// // 			// if (i < listeners.size())
	// // 			// {
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				reqs.erase(reqs.begin() + i);
	// // 				i--;
	// // 			// }
	// // 			continue ;
	// // 		}
	// // 		if (sock_fds.at(i).revents & POLLNVAL)
	// // 		{
	// // 			std::cout << "INVALID\n";
	// // 			// if (i < listeners.size())
	// // 			// {
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				reqs.erase(reqs.begin() + i);
	// // 				i--;
	// // 			// }
	// // 			continue ;
	// // 		}
	// // 		if (sock_fds.at(i).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
	// // 		{
	// // 			// std::cout << "POLLIN(TAN)\n";
	// // 			char buf[2048];
	// // 			//memset(buf, 0, sizeof(buf));
	// // 			size_t	bytes = recv(sock_fds.at(i).fd, buf, sizeof(buf), 0);
	// // 			if (bytes > 0)
	// // 			{
	// // 				std::cout << "Reading...\n";
	// // 				reqs.at(i).append(buf, bytes);
	// // 				if (reqs.at(i).find("\r\n\r\n") != std::string::npos)
	// // 				{
	// // 					std::cout << reqs.at(i) << "\n";
	// // 					parse_request(reqs.at(i), i, sock_fds);
	// // 					close(sock_fds.at(i).fd);
	// // 					sock_fds.erase(sock_fds.begin() + i);
	// // 					reqs.erase(reqs.begin() + i);
	// // 					i--;
	// // 					// parse_request(reqs.at(i), sock_fds.at(i).fd);
	// // 				}
	// // 			}
	// // 			else if (bytes == 0)
	// // 			{
	// // 				std::cout << "Read till the end of the request\n";
	// // 				// parse_request(reqs.at(i), sock_fds.at(i).fd);
	// // 				parse_request(reqs.at(i), i, sock_fds);
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				reqs.erase(reqs.begin() + i);
	// // 				i--;
	// // 			}
	// // 			else if (errno == EAGAIN)
	// // 			{
	// // 				std::cout << "EAGAIN. wtf is that\n";
	// // 				continue ;
	// // 			}
	// // 			else if (bytes < 0)
	// // 				break;
	// // 		}
	// // 		if (sock_fds.at(i).revents & POLLOUT)
	// // 		{
	// // 			// close(sock_fds.at(i).fd);
	// // 			// sock_fds.erase(sock_fds.begin() + i);
	// // 			// reqs.erase(reqs.begin() + i);
	// // 			// i--;
	// // 		}
	// // 	}
	// // }
	// int j = 0;
	// while (g_quit != true)
	// {
	//  	int res = poll(&sock_fds[0], sock_fds.size(), 500);
	// 	// std::cout << "Number of sockets " << sock_fds.size() << "\n";
	// 	if (res < 0)
	// 	{
	// 		std::cout << "TEST\n";
	// 		perror("poll");
	// 		exit(EXIT_FAILURE);
	// 	}
	// 	if (res == 0)
	// 		continue ;
	// 	if (sock_fds.at(0).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
	// 	{
	// 		struct pollfd		cli;
	// 		struct sockaddr_in	client;
	// 		socklen_t			len;
	// 		int					acc_sock;

	// 		len = sizeof(client);
	// 		acc_sock = accept(sock_fds.at(0).fd, (sockaddr *)&client, &len);
	// 		if (errno == EWOULDBLOCK)
	// 		{
	// 			std::cout << "Nothing to accept\n";
	// 			continue ;
	// 		}
	// 		std::cout << "Received a request from a new client\n";
	// 		cli.fd = acc_sock;
	// 		fcntl(cli.fd, F_SETFL, fcntl(cli.fd, F_GETFL) | O_NONBLOCK);
	// 		fcntl(cli.fd, F_SETFD, fcntl(cli.fd, F_GETFD) | FD_CLOEXEC);
	// 		cli.events = POLLIN | POLLOUT;
	// 		cli.revents = 0;
	// 		sock_fds.push_back(cli);
	// 		reqs.push_back("");
	// 		std::cout << "Number of sockets in poll(): " << sock_fds.size() << "\n";
	// 	}
	// 	for (unsigned int i = 1; i < sock_fds.size(); i++)
	// 	{
	// 		if (sock_fds.at(i).revents == 0)
	// 			continue ;
	// 		if (sock_fds.at(i).revents & POLLHUP)
	// 		{
	// 			std::cout << "Hangup\n";
	// 			// if (i < listeners.size())
	// 			// {
	// 				close(sock_fds.at(i).fd);
	// 				sock_fds.erase(sock_fds.begin() + i);
	// 				reqs.erase(reqs.begin() + i);
	// 				i--;
	// 			// }
	// 			continue ;
	// 		}
	// 		if (sock_fds.at(i).revents & POLLERR)
	// 		{
	// 			std::cout << "Error\n";
	// 			// if (i < listeners.size())
	// 			// {
	// 				close(sock_fds.at(i).fd);
	// 				sock_fds.erase(sock_fds.begin() + i);
	// 				reqs.erase(reqs.begin() + i);
	// 				i--;
	// 			// }
	// 			continue ;
	// 		}
	// 		if (sock_fds.at(i).revents & POLLNVAL)
	// 		{
	// 			std::cout << "INVALID\n";
	// 			// if (i < listeners.size())
	// 			// {
	// 				close(sock_fds.at(i).fd);
	// 				sock_fds.erase(sock_fds.begin() + i);
	// 				reqs.erase(reqs.begin() + i);
	// 				i--;
	// 			// }
	// 			continue ;
	// 		}
	// 		if (sock_fds.at(i).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
	// 		{
	// 			j = 0;
	// 			std::cout << "POLLIN(TAN)\n";
	// 			char buf[2048];
	// 			//memset(buf, 0, sizeof(buf));
	// 			size_t	bytes = recv(sock_fds.at(i).fd, buf, sizeof(buf), 0);
	// 			(void)bytes;
	// 		}
	// 		if (sock_fds.at(i).revents & POLLOUT)
	// 		{
	// 			if (j == 0)
	// 			{
	// 				j = 1;
	// 				std::cout << "POLLOUT(TAN)\n";
	// 			}
	// 		}
	// 	}
	// }
	// // while (g_quit != true)
	// // {
	// //  	int res = poll(&sock_fds[0], sock_fds.size(), 500);
	// // 	if (res == -1)
	// // 	{
	// // 		perror("poll");
	// // 		exit(EXIT_FAILURE);
	// // 	}
	// // 	// std::cout << "Number of sockets in poll(): " << sock_fds.size() << "\n";
	// // 	for (unsigned int i = 0; i < sock_fds.size(); i++)
	// // 	{
	// // 		if (sock_fds.at(i).revents & POLLOUT)
	// // 		{
	// // 			std::cout << "PULLOUT\n";
	// // 			if (i < listeners.size())
	// // 			{
	// // 				struct pollfd		cli;
	// // 				struct sockaddr_in	client;
	// // 				socklen_t			len;
	// // 				int					acc_sock;

	// // 				len = sizeof(client);
	// // 				acc_sock = accept(sock_fds.at(i).fd, (sockaddr *)&client, &len);
	// // 				if (errno == EWOULDBLOCK)
	// // 				{
	// // 					std::cout << "Nothing to accept\n";
	// // 					continue ;
	// // 				}
	// // 				std::cout << "received a request from a client\n";
	// // 				cli.fd = acc_sock;
	// // 				fcntl(cli.fd, F_SETFL, fcntl(cli.fd, F_GETFL) | O_NONBLOCK);
	// // 				fcntl(cli.fd, F_SETFD, fcntl(cli.fd, F_GETFD) | FD_CLOEXEC);
	// // 				cli.events = POLLIN | POLLOUT;
	// // 				cli.revents = 0;
	// // 				sock_fds.push_back(cli);
	// // 			}
	// // 		}
	// // 		else if (sock_fds.at(i).revents & POLLHUP)
	// // 		{
	// // 			std::cout << "Hangup\n";
	// // 			if (i < listeners.size())
	// // 			{
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				i--;
	// // 			}
	// // 			continue ;
	// // 		}
	// // 		else if (sock_fds.at(i).revents & POLLERR)
	// // 		{
	// // 			std::cout << "Error\n";
	// // 			if (i < listeners.size())
	// // 			{
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				i--;
	// // 			}
	// // 			continue ;
	// // 		}
	// // 		else if (sock_fds.at(i).revents & POLLIN)	// a client in the poll; handle request; remove client if Connection: keep-alive is not present in request
	// // 		{
	// // 			std::cout << "POLLIN(TAN)\n";
	// // 			char buf[2048];
	// // 			//memset(buf, 0, sizeof(buf));
	// // 			size_t	bytes = recv(sock_fds.at(i).fd, buf, sizeof(buf), 0);
	// // 			if (bytes > 0)
	// // 			{
	// // 				std::cout << "reading\n";
	// // 				req.append(buf, bytes);
	// // 				if (req.find("\r\n\r\n") != std::string::npos)
	// // 				{
	// // 					std::cout << req << "\n";
	// // 					parse_request(req, sock_fds.at(i).fd);
	// // 				}
	// // 			}
	// // 			else if (bytes == 0)
	// // 			{
	// // 				std::cout << "read till the end of the request\n";
	// // 				parse_request(req, sock_fds.at(i).fd);
	// // 				close(sock_fds.at(i).fd);
	// // 				sock_fds.erase(sock_fds.begin() + i);
	// // 				i--;
	// // 			}
	// // 			else if (errno == EAGAIN)
	// // 			{
	// // 				std::cout << "EAGAIN. wtf is that\n";
	// // 				continue ;
	// // 			}
	// // 		}
	// // 	}
	// // }
	// if (g_quit == true)
	// {
	// 	// close all sockets
	// 	for (unsigned int i = 0; i < sock_fds.size(); i++)
	// 	{
	// 		close(sock_fds.at(i).fd);
	// 		sock_fds.pop_back();
	// 		reqs.pop_back();
	// 	}
	// 	for (unsigned int i = 0; i < listeners.size(); i++)
	// 	{
	// 		delete listeners[i];
	// 		listeners.pop_back();
	// 	}
	// }
	// std::cout << (g_quit == true ? "True" : "False") << "\n";

