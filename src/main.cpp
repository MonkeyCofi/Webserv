#include <iostream>
#include "ConfigParser.hpp"
#include <poll.h>
#include <signal.h>
#include <fstream>
#include <fcntl.h>
#include "Request.hpp"

int main(int ac, char **av)
{
	signal(SIGPIPE, SIG_IGN);
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
			throw std::runtime_error("e4");
		std::cout << "Server Ready!\n";
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
