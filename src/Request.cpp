/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehammoud <ehammoud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/25 15:35:12 by ehammoud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request()
{
	this->method = "";
	this->file_URI = "";
	this->host = "";
	this->contentLength = 0;
	this->contentType = "";
	this->keepAlive = true;
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
	this->file_URI = obj.file_URI;
	this->keepAlive = obj.keepAlive;
	this->host = obj.host;
	this->validRequest = obj.validRequest;
	this->status = obj.status;
	this->contentLength = obj.contentLength;
	this->contentType = obj.contentType;
	return (*this);
}

const char*	Request::NoHostException::what()
{
	return ("Request doesn't contain Host header");
}

void Request::changeToLower(char &c)
{
	if (c >= 'A' && c <= 'Z')
		c += 32;
}

Request	&Request::parseRequest(str& request)
{
	std::istringstream	reqStream(request);
	str					headerFieldContent;
	str					line;
	str		 			str_case;

	while (std::getline(reqStream, line) && line.find_first_not_of(" \t\n\r") == str::npos)
		;
	
	this->method = 
	while (std::getline(reqStream, line)) // \n will be stripped from the end of the string so only look for \r
	{
		if (line == "\r")
		{
			std::cout << "Reached end of request header\n";
			break ;
		}
		str_case = returnHeader(line.substr(0, line.find_first_of(":")));
		
		headerFieldContent = line.substr(line.find(":") + 1);
		if (headerFieldContent.at(0) == ' ')
			headerFieldContent.erase(headerFieldContent.begin());
		std::for_each(headerFieldContent.begin(), headerFieldContent.end(), Request::changeToLower);
		setRequestFields(str_case, headerFieldContent);
	}
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

str Request::getFileURI()
{
	return file_URI;
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

str Request::getContentType()
{
	return contentType;
}

size_t Request::getContentLen()
{
	return contentLength;
}
