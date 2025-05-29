#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>
# include <unistd.h>
# include <sys/wait.h>
# include "Server.hpp"

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

		void		setupEnvAndRun(Request* req, std::stringstream& resp, Server* serv, 
					std::vector<struct pollfd>& pollfds, std::set<int>& cgiFds);

		void    	runCGI(std::stringstream& resp, Server* server, Request* req, 
					std::vector<struct pollfd>& pollfds, std::set<int>& cgiFds);

		char**   	envToChar();
		bool    	validScriptAccess() const;
};

#endif
