#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include "Request.hpp"
# include "Server.hpp"

typedef std::string str;

// this class should have access to both the http block and the request
class	Response
{
	private:
		str	statusLine;
		str	contentLength;
		str	contentType;
		str	responseCode;
		str	statusMessage;
	public:
		Response::Response();
		~Response();
		Response(const Response& obj);
		Response	&operator=(const Response& obj);

		void	parseRequest(Request& obj, Server& server);
		str		buildResponse(Request& rq, Server& server);	// this will take fields from the private variables and build a response string accordingly
};

#endif