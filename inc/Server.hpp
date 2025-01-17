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
		const static str	directives[];
		std::vector<str>	names, ips, ports;
		str					ret_str;
		int					ret_code;
		//Socket				serv;
		

		bool	handleAddress(str address);

	public:
		std::vector<Location *>	locations;
		Server();
		Server(const Server &copy);
		~Server();

		bool		handleDirective(std::queue<str> opts);
		BlockOBJ	*handleBlock(std::queue<str> opts);

		const Server &operator =(const Server &copy);
		str	returnPort(int index);

		void	printPortsIpsNames();
		str		getType();
};

#endif
