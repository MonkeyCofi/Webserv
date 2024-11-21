#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <exception>
#include <algorithm>

typedef std::string str;

class Server
{
	private:
		std::vector<str>	names, ips;
		std::vector<int>	ports;
		str					root_location;
	
	public:
		Server();
		Server(const Server &copy);
		~Server();
		const Server &operator =(const Server &copy);
};

#endif