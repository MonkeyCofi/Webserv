#include <iostream>
#include "ConfigParser.hpp"
#include "Socket.hpp"
#include <poll.h>

int main(int ac, char **av)
{
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
	//for (unsigned int i = 0; i < listeners.size(); i++)
	//{
	//	std::cout << "Fd: " << listeners.at(i)->returnSocket(i) << "\n";
	//}
	int test = listen(listeners.at(0)->returnSocket(0), 10);
	if (test == -1)
	{
		perror("Listen");
		return (1);
	}
	struct pollfd	sock_fds[2];
	bzero(&sock_fds, sizeof(struct pollfd) * 2);
	sock_fds[0].fd = listeners.at(0)->returnSocket(0);
	sock_fds[0].events = 0;
	sock_fds[0].revents = 0;

	sock_fds[1].fd = listeners.at(1)->returnSocket(1);
	sock_fds[1].events = 0;
	sock_fds[1].revents = 0;
	int res = poll(sock_fds, 2, 10);
	
}
