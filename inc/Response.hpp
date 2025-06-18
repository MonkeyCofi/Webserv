#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sstream>

typedef std::string str;

class Response
{
	private:
		std::map<str, str>	http_codes;
		str					header, body, code;
		bool				chunked, header_sent, keep_alive;
		int					fd;

	public:
		Response();
		Response(str code);
		Response(Response &copy);

		void	setHeaderField(str field, str value);
		void	setHeaderField(str field, int value);
		void	setHeaderField(str field, ssize_t value);
		void	setBodyFd(int fd);
		void	setBody(str body, str type);
		void	setCode(str code);
		void	setHeaderSent(bool sent);
		void	setKeepAlive(bool keepAlive);
		bool	isChunked() const;
		bool	headerSent() const;
		bool	keepAlive() const;
		str		errorPage(str status) const;
		str		getBody() const;
		int		getBodyFd() const;
		void	clear();

		Response	&operator =(Response &copy);
		~Response();
}

#endif