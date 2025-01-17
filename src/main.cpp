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
	Socket	*ptr;
	std::cout << "no. of servers: " << p->servers.size() << "\n";
	p->servers.at(0)->printPortsIpsNames();
	try
	{
		Socket socket(*p->servers.at(0));
		ptr = &socket;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Bind failed\n";
	}
	
	return 0;
}
