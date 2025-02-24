#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>
#include <exception>
#include <vector>
#include <queue>
#include "BlockOBJ.hpp"
#include "Server.hpp"
#include "Socket.hpp"

typedef std::string str;

class Http: public BlockOBJ
{
	private:
		const static str		directives[];
		std::vector<Socket *>	listeners;
		std::vector<Server *>	servers;

	public:
		Http();
		Http(const Http &copy);
		~Http();

		bool					handleDirective(std::queue<str> opts);
		BlockOBJ				*handleBlock(std::queue<str> opts);
		std::vector<Server *>	getServers();
		std::vector<Socket *>	getListeners();
		str						getType();
		void					printPortsIpsNames();
		void					init_listeners();

		const Http &operator =(const Http &copy);
};

#endif
