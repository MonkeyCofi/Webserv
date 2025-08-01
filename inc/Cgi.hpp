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

class ConnectionManager;

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
		ssize_t						written_bytes;
		Cgi();	// make default constructor inaccessible
	public:
		~Cgi();
		Cgi(const Cgi& copy);
		Cgi	&operator=(const Cgi& copy);
		Cgi(const std::string script_path, Server* server);	// constructor that takes path to cgi script

		std::string	setupEnvAndRun(unsigned int& i, int& client_fd, Request* req, ConnectionManager& cm,Server* serv,
			std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses);

		std::string	runCGI(unsigned int& i, int& client_fd, Server* server, Request* req, ConnectionManager& cm, 
					std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses);

		char**   	envToChar();
		std::string	validScriptAccess() const;
		int*		get_stdin();
		void		dupAndClose(int fd1, int fd2);
		void		setAndAddPollFd(unsigned int i, int fd, ConnectionManager& cm, std::vector<struct pollfd>& pollfds, int events);
		void		writeToFd(int fd, char *buf, size_t r, Request* req);
};

#endif
