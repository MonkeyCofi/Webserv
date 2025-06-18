#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sstream>

typedef std::string str;

class Response
{
	private:
		std::map<str, str>	http_codes;
		str					header, body, code;
		bool				chunked, header_sent;
		int					fd;

	public:
		Response();
		Response(str code);
		Response(Response &copy);

		void	setHeaderField(str field, str value);
		void	setHeaderField(str field, int value);
		void	setHeaderField(str field, ssize_t value);
		void	setBodyFd(int fd);
		void	setBody(str body);
		void	setHeaderSent(bool sent);
		bool	isChunked() const;
		bool	headerSent() const;
		str		getBody() const;
		int		getBodyFd() const;

		Response	&operator =(Response &copy);
		~Response();
}

#endif