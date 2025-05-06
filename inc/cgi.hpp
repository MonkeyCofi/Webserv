#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>
# include "Request.hpp"
# include <unistd.h>

class Cgi
{
private:
	str	cgiFileName;
	std::map<std::string, std::string>	env;
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

	void	setupEnv(Request* req);
	void	runCGI();
	char**   envToChar();
};

#endif
