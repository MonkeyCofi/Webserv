#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>
# include <exception>
# include <algorithm>
# include <cstdio>
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
# include <string>
# include "BlockOBJ.hpp"
# include "Location.hpp"
# include "Request.hpp"
# include "Response.hpp"

typedef std::string str;

class	Socket;

class Server: public BlockOBJ
{
	private:
		typedef enum
		{
			INCOMPLETE,
			FINISH
		}	ResponseState;

		int						min_del_depth;
		str						root;
		bool					autoindex;
		std::map<int, Response>	response;
		const static str		default_ip, default_port, directives[];
		std::vector<str>		names, ips, ports, index;
		std::map<str, str>		error_pages;
		std::vector<Location *>	locations;
		ResponseState			responseState;

		void			handleError(str error_code, int client_fd);
		bool			validAddress(str address);
		bool			handleAddress(str address);
		str				reasonPhrase(str status);
		str				fileType(str file_name);
		str				errorPage(str status);
		str				ssizeToStr(ssize_t x);
		bool			isDirectory(const std::string& path);
		// void			directoryResponse(str path, std::stringstream &resp);
		void 			directoryResponse(Request* req, str path, int client_fd);
		// void			fileResponse(str path, bool checking_index);
		void			fileResponse(Request* req, str path, int file_fd, int client_fd);
		// unsigned int	fileSize(int fd);

	public:
		Server();
		Server(const Server &copy);
		~Server();

		size_t					sent_bytes;
		void					handleRequest(int& client_fd, Request *req, std::vector<struct pollfd>& pollfds, 
								std::map<int, int>& cgiFds);

		bool					handleDirective(std::queue<str> opts);
		BlockOBJ				*handleBlock(std::queue<str> opts);
		void					setDefault();
		Socket*					init_listeners();
		void 					handleError(str error_code);

		/********************/
		/*		getters		*/
		/********************/
		str							getPort(int index);
		str							getIP(int index);
		str							getType();
		str							getRoot() const;
		std::vector<str>			getNames();
		std::vector<str>			getIPs();
		std::vector<str>			getPorts();
		std::vector<Location *>		getLocations();
		ResponseState				getState() const;

		/********************/
		/*		setters		*/
		/********************/
		void	setHeader(str header_);
		void	setBody(str body_);
		void	setFileFD(int fd_);
		void	setState(ResponseState state);

		bool					respond(int client_fd);

		const Server	&operator =(const Server &copy);
		bool			operator ==(Server &server2);

		static ResponseState	returnIncomplete();
		static ResponseState	returnFinish();
};

#endif
