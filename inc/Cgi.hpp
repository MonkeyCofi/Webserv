#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>
# include <unistd.h>
# include <sys/wait.h>
# include "Server.hpp"

# define READ 0
# define WRITE 1

class Cgi
{
	private:
		str	cgiFileName;
		std::vector<str>	env;
		// std::map<std::string, std::string>	env;
		str	path_info;
		str	query_string;
		str	content_type;
		str	content_length;
		str	method;
		str	host;
		int	pipe_fds[2];
		pid_t	cgi_fd;
	public:	
		Cgi();
		~Cgi();
		Cgi(const Cgi& copy);
		Cgi	&operator=(const Cgi& copy);

		void		setupEnvAndRun(Request* req, std::stringstream& resp, Server* serv);
		void		runCGI(std::stringstream& resp);
		char**   	envToChar();
};

#endif
