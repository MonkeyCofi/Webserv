#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <exception>
#include <vector>
#include "Location.hpp"

typedef std::string str;

class Server
{
	private:
		std::vector<str>		names, ips;
		std::vector<int>		ports;
		std::vector<Location>	locations;
	
	public:
		Server();
		Server(const Server &copy);
		~Server();
		const Server &operator =(const Server &copy);
};

#endif