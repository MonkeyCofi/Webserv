/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/02 12:08:17 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request()
{
	this->method = "";
	this->serveFile = "";
	this->host = "";
	this->keepAlive = false;
	this->validRequest = false;
	this->status = 400;
}

//Request::Request(str& request)
//{
	
//}

Request::~Request()
{
	
}

Request::Request(const Request& obj)
{
	(void)obj;
}

Request	&Request::operator=(const Request& obj)
{
	if (&obj == this)
		return (*this);
	this->method = obj.method;
	this->serveFile = obj.serveFile;
	this->keepAlive = obj.keepAlive;
	this->host = obj.host;
	this->validRequest = obj.validRequest;
	this->status = obj.status;
	return (*this);
}

const char*	Request::NoHostException::what()
{
	return ("Request doesn't contain Host header");
}

Request	Request::parseRequest(str& request)
{
	const str	status_line;

	if (request.find("\r\n") == str::npos || request.find("HTTP/1.1") == str::npos || request.find("Host: ") == str::npos)
		return (*this);
	status_line = request.substr(0, request.find_first_of("\r\n"));
	if (std::count(status_line.begin(), status_line.end(), ' ') < 2)
		return (*this);
	this->method = status_line.substr(0, status_line.find_first_of(' '));
	this->serveFile = status_line.substr(status_line.find_first_of(' ') + 1);
	if (serveFile.substr(status_line.find_first_of(' ') + 1) != "HTTP/1.1")
		return (*this);
	this->serveFile = serveFile.substr(0, file_name.find_first_of(' '));
	this->host = request.substr(request.find("Host: ") + 1, request.find("\r\n"));
	if (this->host.length() < 1)
		return (*this);
	// std::cout << request << "\n";
	// if (file_name.at(0) == '/' && file_name.length() == 1)
	// 	file_name = "root";
	// else
	// {
	// 	std::string::iterator	it = file_name.begin();
	// 	file_name.erase(it);
	// }
	// std::cout << "The file to get is " << file_name << "\n";
	status = 200;
	validRequest = true;
	return (*this);
}

bool Request::isValidRequest()
{
	return validRequest;
}

int Request::getStatus()
{
	return status;
}

str Request::getServeFile()
{
	return serveFile;
}

str Request::getMethod()
{
	return method;
}

str Request::getHost()
{
	return host;
}

bool Request::shouldKeepAlive()
{
	return keepAlive;
}
