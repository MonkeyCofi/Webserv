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
# include <dirent.h>
# include <cstring>
# include <sys/stat.h>
# include <ctime>
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
		int						file_fd;
		ssize_t					total_length;
		str						root, header, body;
		bool					keep_alive, autoindex;
		const static str		default_ip, default_port, directives[];
		std::vector<str>		names, ips, ports;
		std::map<str, str>		error_pages;
		std::map<str, str>		http_codes;
		std::vector<Location *>	locations;

		void			handleError(str error_code, std::stringstream &resp);
		bool			validAddress(str address);
		bool			handleAddress(str address);
		str				reasonPhrase(str status);
		str				fileType(str file_name);
		str				errorPage(str status);
		str				ssizeToStr(ssize_t x);
		bool			isDirectory(const std::string& path);
		void			directoryResponse(Request *req, str path, std::stringstream &resp);
		void			fileResponse(Request *req, str path, std::stringstream &resp, bool checking_index);
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
		str						getRoot() const;
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
