#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>
# include <sys/wait.h>
# include "Server.hpp"
# include "CGIinfo.hpp"

# define READ 0
# define WRITE 1
# define CHLDPROC 0

class Cgi
{
	private:
		str					cgiPath;
		str					scriptName;
		std::vector<str>	env;
		str					path_info;
		str					query_string;
		str					content_type;
		str					content_length;
		str					method;
		str					host;
		int					pipe_fds[2];
		pid_t				cgi_fd;
		Cgi();	// make default constructor inaccessible
	public:
		~Cgi();
		Cgi(const Cgi& copy);
		Cgi	&operator=(const Cgi& copy);
		Cgi(const str script_path, Server* server);	// constructor that takes path to cgi script

		str			setupEnvAndRun(int& client_fd, Request* req, Server* serv, std::vector<struct pollfd>& pollfds,
					std::map<int, CGIinfo>& cgiProcesses);

		str    		runCGI(int& client_fd, Server* server, Request* req, 
					std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses);

		char**   	envToChar();
		str    		validScriptAccess() const;
};

#endif
