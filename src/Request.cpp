/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehammoud <ehammoud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/25 18:03:45 by ehammoud         ###   ########.fr       */
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
	this->status = "400";
}

Request::~Request()
{
	
}

Request::Request(const Request& obj)
{
	this->method = obj.method;
	this->file_URI = obj.file_URI;
	this->keepAlive = obj.keepAlive;
	this->host = obj.host;
	this->validRequest = obj.validRequest;
	this->status = obj.status;
	this->contentLength = obj.contentLength;
	this->contentType = obj.contentType;
}

Request::Request(str request)
{
	this->method = "";
	this->file_URI = "";
	this->host = "";
	this->contentLength = 0;
	this->contentType = "";
	this->keepAlive = true;
	this->validRequest = false;
	this->status = "400";
	parseRequest(request);
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

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

bool Request::parseRequestLine(str &line)
{
	if (std::count(line.begin(), line.end(), ' ') != 2 || line.length() < 14)
		return false;
	if (line.find_first_of(" ") > 6)
	{
		this->status = "501";
		return false;
	}
	this->method = line.substr(0, line.find_first_of(" "));
	if (method != "GET" && method != "POST" && method != "DELETE")
		return false;
	line = line.substr(line.find_first_of(" ") + 1);
	if (line.find_first_of(" ") > 256 || line.find_first_of(" ") == 0)
	{
		this->status = "414";
		return false;
	}
	this->file_URI = line.substr(0, line.find_first_of(" "));
	if (file_URI.at(0) != '/' || this->file_URI.find_first_of("\t\r") != str::npos)
		return false;
	line = line.substr(line.find_first_of(" ") + 1);
	if (line != "HTTP/1.1\r")
	{
		this->status = "505";
		return false;
	}
	return true;
}

void Request::setRequestField(str &header_field, str &field_content)
{
	if (header_field == "host")
		this->host = field_content;
	if (header_field == "connection" && field_content == "close")
		this->keepAlive = false;
	if (header_field == "content-type")
		this->contentType = field_content;
	if (header_field == "content-length")
		this->contentLength = std::atoi(field_content.c_str());
}

Request	&Request::parseRequest(str& request)
{
	std::stringstream	reqStream(request);
	str		 			header_field;
	str					field_value;
	str					line;
	bool				ignore;
	unsigned int		lnsp;

	std::getline(reqStream, line);
	if (!parseRequestLine(line))
		return (*this);
	ignore = false;
	while (std::getline(reqStream, line))
	{
		if (line == "\r")
			break ;
		if (ignore)
		{
			ignore = false;
			continue;
		}
		if (line.find_first_of(":") == str::npos || line.find_first_not_of(" \t\r") >= line.find_first_of(":"))
			return (*this);
		std::for_each(line.begin(), line.end(), Request::changeToLower);
		header_field = line.substr(line.find_first_not_of(" \t\r"), line.find_first_of(":"));
		if (line.find_first_not_of(" \t\r", line.find(":") + 1) == str::npos && (header_field == "host" || header_field == "content-length"))
			return (*this);
		line = line.substr(line.find_first_not_of(" \t\r", line.find(":") + 1));
		for (lnsp = line.length() - 1; lnsp > 0; lnsp--)
		{
			if (line[lnsp] != ' ' && line[lnsp] != '\t' && line[lnsp] != '\r')
				break;
		}
		if (lnsp < line.length() - 1)
			ignore = true;
		field_value = line.substr(0, lnsp + 1);
		if (field_value.find_first_not_of("0123456789") != str::npos && header_field == "content-length")
			return (*this);
		setRequestField(header_field, field_value);
	}
	status = "200";
	validRequest = true;
	return (*this);
}

bool Request::isValidRequest()
{
	return validRequest;
}

str Request::getStatus()
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
