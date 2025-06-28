#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sstream>
#include <map>
#include <iostream>
#include <cstdlib>

typedef std::string str;

class Response
{
	private:
		str					header, body, code;
		bool				chunked, header_sent, keep_alive;
		int					fd;
		std::map<str, str>	http_codes;

	public:
		Response();
		Response(str code);
		Response(const Response &copy);

		void	setHeaderField(str field, str value);
		void	setHeaderField(str field, ssize_t value);
		void	setBodyFd(int fd);
		void	setBody(str body, str type);
		void	setCode(str code);
		void	setHeaderSent(bool sent);
		void	setKeepAlive(bool keepAlive);
		bool	isChunked() const;
		bool	headerSent() const;
		bool	keepAlive() const;
		bool 	doneSending() const;
		str		errorPage(str status) const;
		str		getBody() const;
		int		getBodyFd() const;
		str		getHeader();
		void	clear();

		Response	&operator =(const Response &copy);
		~Response();

		void	printResponse();
};

#endif
