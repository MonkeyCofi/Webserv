#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <exception>
# include <vector>
# include <queue>
# include "BlockOBJ.hpp"
# include "Location.hpp"

typedef std::string str;

class	Socket;

class Server: public BlockOBJ
{
	private:
		const static str		directives[];
		std::vector<str>		names, ips, ports;
		str						ret_str;
		int						ret_code;
		std::vector<Location *>	locations;
		//Socket				serv;
		

		bool	handleAddress(str address);

	public:
		Server();
		Server(const Server &copy);
		~Server();

		bool		handleDirective(std::queue<str> opts);
		BlockOBJ	*handleBlock(std::queue<str> opts);

		str	returnPort(int index);
		str	returnIP(int index);
		str		getType();
		Socket*	init_listeners();
		void	printPortsIpsNames();
		std::vector<str>	returnNames();
		std::vector<str>	returnIPs();
		std::vector<str>	returnPorts();
		std::vector<Location *>	getLocations();

		const Server &operator =(const Server &copy);
};

#endif
