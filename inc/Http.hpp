#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>
#include <exception>
#include <vector>
#include "BlockOBJ.hpp"
#include "Server.hpp"

typedef std::string str;

class Http: public BlockOBJ
{
	private:
		std::vector<Server> servers;

	public:
		Http();
		Http(const Http &copy);
		~Http();

		bool	handleDirective(std::vector<str> opts);
		bool	handleBlock(std::queue<str> opts);

		const Http &operator =(const Http &copy);
};

#endif