#include <iostream>
#include "ConfigParser.hpp"
#include "Socket.hpp"

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
	for (unsigned int i = 0; i < listeners.size(); i++)
	{
		std::cout << "Fd: " << listeners.at(i)->returnSocket(i) << "\n";
	}
	//for (std::vector<Socket *>::iterator it = listeners.begin(); it != listeners.end(); it++)
	//{
	//	std::cout << "Fd: " << it->returnSocket()
	//}
}
