#include <iostream>
#include "ConfigParser.hpp"
#include "Socket.hpp"
#include <poll.h>

int main(int ac, char **av)
{
	unlink("0.0.0.0");
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
			std::cout << sock_fds.at(i).revents << "\n";
			if (sock_fds.at(i).revents & POLLIN)
			{
				struct sockaddr_in	client;
				socklen_t	len = sizeof(client);
				int acc_sock = accept(sock_fds.at(i).fd, (sockaddr *)&client, &len);
				char buf[1024];
				recv(acc_sock, buf, sizeof(buf), 0);
				std::cout << buf << "\n";
				close(acc_sock);
			}
			else if (sock_fds.at(i).revents & POLLOUT)
				std::cout << "Fd " << sock_fds.at(i).fd << " is ready for output\n";
		}
	 }
}
