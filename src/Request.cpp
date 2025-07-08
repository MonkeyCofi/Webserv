/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/04/29 10:46:34 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "ConnectionManager.hpp"

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
	this->body_boundary = "";
	this->body_boundaryEnd = "";
	this->headerReceived = false;
	this->fullyReceived = false;
	this->has_body = false;
	this->bytesReceived = 0;
	this->tempFileName = "";
	this->isCGIrequest = false;
}

Request::~Request()
{
	
}

Request::Request(const Request& obj)
{
	std::cout << "request copy constructor called\n";
	this->method = obj.method;
	this->file_URI = obj.file_URI;
	this->keepAlive = obj.keepAlive;
	this->host = obj.host;
	this->validRequest = obj.validRequest;
	this->status = obj.status;
	this->contentLength = obj.contentLength;
	this->contentType = obj.contentType;
	this->body_boundary = obj.body_boundary;
	this->fullyReceived = obj.fullyReceived;
	this->headerReceived = obj.headerReceived;
	this->request = obj.request;
	this->has_body = obj.has_body;
	this->bytesReceived = obj.bytesReceived;
	this->tempFileName = obj.tempFileName;
	this->isCGIrequest = false;
}

Request::Request(str request)
{
	*this = Request();
	// this->method = "";
	// this->file_URI = "";
	// this->host = "";
	// this->contentLength = 0;
	// this->contentType = "";
	// this->keepAlive = true;
	// this->validRequest = false;
	// this->status = "400";
	// this->body_boundary = "";
	// this->headerReceived = false;
	// this->fullyReceived = false;
	// this->has_body = false;
	parseRequest(request);
}

Request	&Request::operator=(const Request& obj)
{
	std::cout << "request copy assignment called\n";
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
	this->body_boundary = obj.body_boundary;
	this->headerReceived = obj.headerReceived;
	this->fullyReceived = obj.fullyReceived;
	this->has_body = false;
	this->isCGIrequest = false;
	return (*this);
}

const char*	Request::NoHostException::what() const throw()
{
	return ("Request doesn't contain Host header");
}

void	Request::pushRequest(std::string& req)
{
	request.append(req);
	// request.push_back(req);
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
	size_t	pos;

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
	if (this->file_URI.find("cgi-bin/") != str::npos)
		this->isCGIrequest = true;
	pos = this->file_URI.find("%20");
	while (pos != str::npos)
	{
		this->file_URI.replace(pos, 3, " ");
		pos += 3;
		pos = this->file_URI.find("%20", pos);
	}
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
	// std::cout << MAGENTA << "Header field being parsed: " << header_field << NL;
	if (header_field == "host")
		this->host = field_content;
	if (header_field == "connection" && field_content == "close")
		this->keepAlive = false;
	if (header_field == "content-type")
	{
		this->contentType = field_content;
		if (this->body_boundary.empty() == false)
			return ;
		size_t	boundary_position = field_content.find("boundary=");
		if (boundary_position != std::string::npos)
		{
			std::cout << RED << "B: " << &field_content[boundary_position] << NL;
			this->body_boundary = "--";
			this->body_boundary += field_content.substr(boundary_position + std::strlen("boundary="));
			this->body_boundaryEnd = body_boundary + "--";
		}
	}
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
	int i = 0;
	while (std::getline(reqStream, line))
	{
		if (line == "\r")
			break ;
		if (ignore)
		{
			ignore = false;
			std::cout << "Ignoring " << line << "\n";
			continue;
		}
		if (line.find_first_of(":") == str::npos || line.find_first_not_of(" \t\r") >= line.find_first_of(":"))
			return (*this);
		if (line.find("Content-Type") != std::string::npos)
			std::for_each(line.begin(), line.begin() + std::string("Content-Type").length(), Request::changeToLower);
		else
			std::for_each(line.begin(), line.end(), Request::changeToLower);
		header_field = line.substr(line.find_first_not_of(" \t\r"), line.find_first_of(":"));
		(void)i;
		if (line.find_first_not_of(" \t\r", line.find(":") + 1) == str::npos && (header_field == "host" || header_field == "content-length"))
			return (*this);
		// std::cout << "line before: " << line << "\n"; 
		line = line.substr(line.find_first_not_of(" \t\r", line.find(":") + 1));
		// std::cout << "line now: " << line << "\n";
		for (lnsp = line.length() - 1; lnsp > 0; lnsp--)
		{
			if (line[lnsp] != ' ' && line[lnsp] != '\t' && line[lnsp] != '\r')
				break;
		}
		// if (lnsp < line.length() - 1)
		// 	ignore = true;
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

bool	Request::getFullyReceived() const
{
	return fullyReceived;
}

const str& Request::getStatus() const
{
	return status;
}

bool Request::getHasBody() const
{
	return has_body;
}

bool	Request::getHeaderReceived() const
{
	return headerReceived;
}

std::fstream&	Request::getBodyFile()
{
	return (this->bodyFile);
}


const str& Request::getFileURI() const
{
	return file_URI;
}

const str& Request::getMethod() const
{
	return method;
}

const str& Request::getHost() const
{
	return host;
}

bool Request::shouldKeepAlive()
{
	return keepAlive;
}

const str&	Request::getContentType() const
{
	return contentType;
}

const str&	Request::getBoundary() const
{
	return (body_boundary);
}

size_t Request::getContentLen()
{
	return contentLength;
}

const str&	Request::getRequest() const
{
	return (this->request);
}

const str&	Request::getTempFileName() const
{
	return (this->tempFileName);
}

void	Request::setFullyReceived(const bool status)
{
	this->fullyReceived = status;
}

void	Request::clearVector()
{
	this->request.clear();
}

void	Request::setHeaderReceived(const bool status)
{
	this->headerReceived = status;
}

void	Request::setHasBody(const bool status)
{
	this->has_body = status;
}

void	Request::setTempFileName(const str file)
{
	this->tempFileName = file;
}
