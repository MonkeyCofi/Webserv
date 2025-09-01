#include <iostream>
#include "ConfigParser.hpp"
#include <poll.h>
#include <signal.h>
#include <fstream>
#include <fcntl.h>
#include "Request.hpp"

int main(int ac, char **av)
{
	bool	defaultConf = false;
	int		exit_status = EXIT_SUCCESS;

	signal(SIGPIPE, SIG_IGN);
	unlink("localhost");
	if (ac == 1)
		defaultConf = true;
	else if (ac != 2)
	{
		std::cerr << "Error\nInvalid number of arguments!\nUsage: ./webserv <configuration_file.conf>\n";
		return (1);
	}
	Engine	*eng = NULL;
	try
	{
		ConfigParser conf;
		str	tmp = (defaultConf == false ? av[1] : "");
		conf.parse(tmp, defaultConf, &eng);
		if (!eng)
			throw std::runtime_error("failed to boot up server");
		std::cout << "Server Ready!\n";
		eng->start();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Caught standard exception " << e.what() << "\n";
		exit_status = EXIT_FAILURE;
	}
	if (eng)
		delete eng;
	exit(exit_status);
}
