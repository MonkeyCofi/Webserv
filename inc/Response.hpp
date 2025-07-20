#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <cstdlib>

typedef std::string str;

class Response
{
	private:
		str					header, body, code, root;
		bool				chunked, header_sent, keep_alive, autoindex;
		int					fd;
		std::map<str, str>	http_codes;
		size_t				sent_bytes;
		std::vector<str>	index_files;

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
		void	setRoot(const str &str);
		str		getRoot() const;
		void	setAutoIndex(bool autoidx);
		bool	getAutoIndex() const;
		bool	headerSent() const;
		bool	keepAlive() const;
		bool 	doneSending() const;
		str		errorPage(str status) const;
		str		getBody() const;
		int		getBodyFd() const;
		str		getHeader();
		void	clear();
		size_t	getSentBytes() const;
		void	addSentBytes(size_t value);
		const std::vector<str>& getIndexFiles() const;
		void setIndexFiles(const std::vector<str>& files);

		Response	&operator =(const Response &copy);
		~Response();

		void	printResponse();
};

#endif
