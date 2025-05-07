#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>
#include <exception>
#include <vector>
#include <queue>
#include "BlockOBJ.hpp"
#include "Server.hpp"

typedef std::string str;

class Http: public BlockOBJ
{
	private:
		const static str		directives[];
		std::vector<Server *>	servers;

	public:
		Http();
		Http(const Http &copy);
		~Http();

		bool					handleDirective(std::queue<str> opts);
		BlockOBJ				*handleBlock(std::queue<str> opts);
		std::vector<Server *>	getServers();
		str						getType();

		const Http &operator =(const Http &copy);
};

#endif
