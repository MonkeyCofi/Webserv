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
	p->init_listeners();
	p->printPortsIpsNames();
	(void)p;
	//Socket socket(*p->servers.at(0));
	//if (listen(socket.returnSocket(0), 5) == -1)
	//	perror("Listen");
	//else
	//	std::cout << "Currently listening\n";
	//// now that the socket is listening, create an accept socket
	//struct sockaddr_in	cl = socket.returnClient();
	//socklen_t	len = sizeof(socket.returnClient());
	
	//char	address[INET_ADDRSTRLEN];
	//if (inet_ntop(AF_INET, &cl.sin_addr, address, len) == NULL)
	//	std::cout << "Couldn't get presentation of IP\n";
	//std::cout << "Listening on address: " << address << "\n";
	//int acc_socket = accept(socket.returnSocket(0), (struct sockaddr *)&cl, &len);
	//char buf[1024];
	//recv(acc_socket, buf, sizeof(buf), 0);
	//std::cout << buf << "\n";
	//unlink("0.0.0.0");
	//return 0;
}
