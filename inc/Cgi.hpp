#pragma once
#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>
# include <sys/wait.h>
# include <vector>
# include "Server.hpp"
# include "CGIinfo.hpp"

# define READ 0
# define WRITE 1
# define CHLDPROC 0

class Cgi
{
	private:
		std::string					cgiPath;
		std::string					scriptName;
		std::vector<std::string>	env;
		std::string					path_info;
		std::string					query_string;
		std::string					content_type;
		std::string					content_length;
		std::string					method;
		std::string					host;
		int							pipe_fds[2];
		int							stdin_fds[2];
		pid_t						cgi_fd;
		Cgi();	// make default constructor inaccessible
	public:
		~Cgi();
		Cgi(const Cgi& copy);
		Cgi	&operator=(const Cgi& copy);
		Cgi(const std::string script_path, Server* server);	// constructor that takes path to cgi script

		std::string	setupEnvAndRun(int& client_fd, Request* req, Server* serv, std::vector<struct pollfd>& pollfds,
					std::map<int, CGIinfo>& cgiProcesses);

		std::string	runCGI(int& client_fd, Server* server, Request* req, 
					std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses);

		char**   	envToChar();
		std::string	validScriptAccess() const;
		int*		get_stdin();
		void		dupAndClose(int fd1, int fd2);
		void		setAndAddPollFd(int fd, std::vector<struct pollfd>& pollfds, int events);
};

#endif
