#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>
# include <exception>
# include <algorithm>
# include <unistd.h>
# include <fcntl.h>
# include <vector>
# include <queue>
# include <poll.h>
# include <signal.h>
# include <cstring>
# include <sstream>
# include <sys/socket.h>
# include <map>
# include "BlockOBJ.hpp"
# include "Location.hpp"
# include "Request.hpp"

typedef std::string str;

class	Socket;

class Server: public BlockOBJ
{
	private:
		str						root;
		str						header;
		int						file_fd;
		bool					keep_alive;
		ssize_t					total_length;
		const static str		default_ip, default_port, directives[];
		std::vector<str>		names, ips, ports;
		std::map<str, str>		error_pages;
		std::vector<Location *>	locations;

		void			handleError(str error_code, std::stringstream &resp);
		bool			validAddress(str address);
		bool			handleAddress(str address);
		str				reasonPhrase(str status);
		str				fileType(str file_name);
		str				errorPage(str status);
		str				ssizeToStr(ssize_t x);
		// unsigned int	fileSize(int fd);

	public:
		Server();
		Server(const Server &copy);
		~Server();

		void					handleRequest(Request *req);
		bool					handleDirective(std::queue<str> opts);
		BlockOBJ				*handleBlock(std::queue<str> opts);
		str						getPort(int index);
		str						getIP(int index);
		str						getType();
		void					setDefault();
		Socket*					init_listeners();
		std::vector<str>		getNames();
		std::vector<str>		getIPs();
		std::vector<str>		getPorts();
		std::vector<Location *>	getLocations();

		bool					respond(int client_fd);

		const Server	&operator =(const Server &copy);
		bool			operator ==(Server &server2);
};

#endif
