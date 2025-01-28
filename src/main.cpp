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

	std::vector<struct pollfd>	sock_fds;
	for (unsigned int i = 0; i < listeners.size(); i++)
	{
		struct pollfd	temp;
		temp.fd = listeners.at(i)->returnSocket(i);
		temp.events = 0;
		temp.revents = 0;
		sock_fds.push_back(temp);
	}

	// while (1)
	// {
	// 	int res = poll(&sock_fds[0], sock_fds.size(), 100);
	// 	if ()
	// }
}
