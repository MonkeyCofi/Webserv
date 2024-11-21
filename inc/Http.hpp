#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>
#include <exception>
#include <algorithm>
#include "Server.hpp"

typedef std::string str;

class Http
{
	private:
		std::vector<Server> servers;

	public:
		Http();
		Http(const Http &copy);
		~Http();
		const Http &operator =(const Http &copy);
};

#endif