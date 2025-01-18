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
	p->servers.at(0)->printPortsIpsNames();
	Socket socket(*p->servers.at(0));
	if (listen(socket.returnSocket(0), 5) == -1)
		perror("Listen");
	else
		std::cout << "Currently listening\n";
	// now that the socket is listening, create an accept socket
	struct sockaddr_in	cl = socket.returnClient();
	socklen_t	len = sizeof(socket.returnClient());
	int acc_socket = accept(socket.returnSocket(0), (struct sockaddr *)&cl, &len);
	(void)acc_socket;
	return 0;
}
