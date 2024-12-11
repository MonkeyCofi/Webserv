#include <iostream>
#include "ConfigParser.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Error\nInvalid number of arguments!\nUsage: ./webserv <configuration_file.conf>\n";
		return (1);
	}
	try
	{
		ConfigParser conf;
		str		tmp = av[1];
		Engine	eng = conf.parse(tmp);
		std::cout << "Parsing done!\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
	}
	return 0;
}
