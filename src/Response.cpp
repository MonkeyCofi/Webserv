# include "Response.hpp"

Response::Response()
{

}

Response::~Response()
{

}

Response::Response(const Response& obj)
{
	(void)obj;
}

Response	&Response::operator=(const Response& obj)
{
	(void)obj;
	return (*this);
}

void	Response::parseRequest(Request& rq, Server& server)
{
	std::vector<Location *>	locations = server.getLocations();
	// parse the request status line and see whether requested URI is accessible
	for (std::vector<Location *>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		std::cout << "Location\n";
	}
}

str	Response::buildResponse(Request& rq, Server& server)
{
	str	status_line;
	str	response;

	Response::parseRequest(rq, server);
	status_line = "HTTP/1.1 "  + this->responseCode + " " + this->statusMessage + "\r\n";
	return (response);
}