/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/02 12:08:17 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response()
{
	this->method = "none";
	this->serveFile = "none";
	this->host = "none";
	this->contentLength = 0;
	this->keepAlive = false;
	//throw(NoHostException());
}

//Response::Response(str& request)
//{
	
//}

Response::~Response()
{
	
}

Response::Response(const Response& obj)
{
	(void)obj;
}

Response	&Response::operator=(const Response& obj)
{
	if (&obj == this)
		return (*this);
	this->method = obj.method;
	this->serveFile = obj.serveFile;
	this->contentLength = obj.contentLength;
	this->keepAlive = obj.keepAlive;
	return (*this);
}

const char*	Response::NoHostException::what()
{
	return ("Response doesn't contain Host header");
}

Response	Response::parseResponse(str& request)
{
	Response		req;
	str			file_name;
	const str	status_line = request.substr(0, request.find_first_of("\r\n"));
	const str	method = status_line.substr(0, status_line.find_first_of(' '));

	file_name = status_line.substr(status_line.find_first_of(' ') + 1);\
	file_name = file_name.substr(0, file_name.find_first_of(' '));
	std::cout << request << "\n";
	if (file_name.at(0) == '/' && file_name.length() == 1)
		file_name = "root";
	else
	{
		std::string::iterator	it = file_name.begin();
		file_name.erase(it);
	}
	std::cout << "The file to get is " << file_name << "\n";
	req.serveFile = file_name;
	req.fileFD = open(req.serveFile.c_str(), O_RDWR);
	
	
	
	return (req);
}

int	Response::getFileFD()
{
	return (this->fileFD);
}
