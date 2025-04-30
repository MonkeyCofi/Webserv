#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <map>

class cgi
{
private:
	std::map<std::string, std::string>	env;
public:
	cgi();
	~cgi();
	cgi();
	~cgi();

	void	setupEnv();
};

#endif