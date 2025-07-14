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
	this->header = "";
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
	this->cgi_fd = -1;
	this->received_body_bytes = 0;
	this->partial_request = false;
}

Request::~Request()
{
	
}

Request::Request(const Request& obj)
{
	*this = obj;
}

Request::Request(str request)
{
	*this = Request();
	parseRequest(request);
}

Request	&Request::operator=(const Request& obj)
{
	if (&obj == this)
		return (*this);
	this->header = obj.header;
	this->request = obj.request;
	this->bytesReceived = obj.bytesReceived;
	this->tempFileName = obj.tempFileName;
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
	this->has_body = obj.has_body;
	this->isCGIrequest = obj.isCGIrequest;
	this->cgi_fd = obj.cgi_fd;
	this->received_body_bytes = obj.received_body_bytes;
	this->partial_request = obj.partial_request;
	return (*this);
}

const char*	Request::NoHostException::what() const throw()
{
	return ("Request doesn't contain Host header");
}

int	Request::pushRequest(str &req)
{
	size_t	pos;

	this->header += req;
	if (this->header.length() > MAX_HEADER_SIZE)
		return -1;
	pos = this->header.find("\r\n\r\n");
	if (pos == str::npos)
		return 0;
	if (pos != this->header.length() - 4)
	{
		req = this->header.substr(pos + 4);
		this->header = this->header.substr(0, pos);
		this->parseRequest(this->header);
		if (this->method == "GET" || this->method == "DELETE")
			return -1;
		this->received_body_bytes += req.size();
		this->headerReceived = true;
		this->request += req;
		return 1;
	}
	this->parseRequest(this->header);
	this->headerReceived = true;
	return 1;
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
	if (this->file_URI.find("cgi-bin/") != str::npos && this->file_URI.find("cgi-bin/") != this->file_URI.length() - 8)
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
			continue;
		}
		// if (line.find_first_of(":") == str::npos || line.find_first_not_of(" \t\r") >= line.find_first_of(":"))
		// 	return (*this);
		// if (line.find("Content-Type") != std::string::npos)
		// {
		// 	for (str::iterator it = line.begin(); it < line.begin() + 12)
		// 		*it = std::tolower(*it);
		// }
		// else
		// {
		// 	for (str::iterator it = line.begin(); it < line.end())
		// 		*it = std::tolower(*it);
		// }
		header_field = line.substr(line.find_first_not_of(" \t\r"), line.find_first_of(":"));
		for (str::iterator it = header_field.begin(); it != header_field.end(); it++)
			*it = std::tolower(*it);
		(void)i;
		if (line.find_first_not_of(" \t\r", line.find(":") + 1) == str::npos && (header_field == "host" || header_field == "content-length"))
			return (*this);
		line = line.substr(line.find_first_not_of(" \t\r", line.find(":") + 1));
		for (lnsp = line.length() - 1; lnsp > 0; lnsp--)
		{
			if (line[lnsp] != ' ' && line[lnsp] != '\t' && line[lnsp] != '\r')
				break;
		}
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

bool	Request::isCGI() const
{
	return (this->isCGIrequest);
}

void	Request::setCGIfd(int fd)
{
	this->cgi_fd = fd;
}

size_t	Request::getReceivedBytes() const
{
	return (this->received_body_bytes);
}

int	Request::getCGIfd() const
{
	return (this->cgi_fd);
}

void	Request::setPartialRequest(bool cond)
{
	this->partial_request = cond;
}

bool	Request::isPartial() const
{
	return (this->partial_request);
}
