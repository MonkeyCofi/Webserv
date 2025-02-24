#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>
# include <exception>
# include <vector>
# include <algorithm>
# include <queue>
# include "BlockOBJ.hpp"
# include "Location.hpp"

typedef std::string str;

class	Socket;

class Server: public BlockOBJ
{
	private:
		const static str		default_ip, default_port, directives[];
		std::vector<str>		names, ips, ports;
		str						ret_str;
		int						ret_code;
		std::vector<Location *>	locations;
		
		bool	validAddress(str address);
		bool	handleAddress(str address);

	public:
		Server();
		Server(const Server &copy);
		~Server();

		bool					handleDirective(std::queue<str> opts);
		BlockOBJ				*handleBlock(std::queue<str> opts);
		str						getPort(int index);
		str						getIP(int index);
		str						getType();
		void					setDefault();
		Socket*					init_listeners();
		void					printPortsIpsNames();
		std::vector<str>		getNames();
		std::vector<str>		getIPs();
		std::vector<str>		getPorts();
		std::vector<Location *>	getLocations();

		const Server	&operator =(const Server &copy);
		bool			operator ==(Server &server2);
};

#endif
