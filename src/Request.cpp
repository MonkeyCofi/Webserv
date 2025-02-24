/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/24 20:46:42 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request()
{
	this->method = "none";
	this->file_URI = "none";
	this->hostIP = "none";
	this->contentLength = 0;
	this->keepAlive = false;
	this->URI_fd = -1;
	//throw(NoHostException());
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
	this->contentLength = obj.contentLength;
	this->keepAlive = obj.keepAlive;
	this->URI_fd = obj.URI_fd;
	return (*this);
}

const char*	Request::NoHostException::what()
{
	return ("Request doesn't contain Host header");
}

Request::t_header	Request::returnHeader(str line) const
{
	if (line == "Accept") return (accept);
	if (line == "Content-length") return (content_length);
	if (line == "Content-type") return (content_type);
	if (line == "Host") return (host);
	if (line == "Connection") return (connection);
	return (none);
}

void	Request::parseStatusLine(str& status_line)
{
	this->method = status_line.substr(0, status_line.find_first_of(' '));
	status_line = status_line.substr(status_line.find_first_of(' ') + 1, std::string::npos);
	this->file_URI = status_line.substr(0, status_line.find_first_of(' '));
}

void	Request::setRequestFields(t_header str_case, str& headerFieldContent)
{
	switch (str_case)
	{
		case(accept):
		{
			std::istringstream	acc_types(headerFieldContent);
			str					type;

			while (std::getline(acc_types, type, ','))
			{
				this->accept_types.push_back(type);
				std::cout << "accept type: " << type << "\n";
			}
			return ;	
		}
		case (content_length):
			this->contentLength = atoi(headerFieldContent.c_str());
			return ;
		case (content_type):
			this->contentType = headerFieldContent;
			return ;
		case (host):
			this->hostIP = headerFieldContent;
			return ;
		case (connection):
			this->keepAlive = (headerFieldContent == "keep-alive" ? true : false);
			return ;
		case (none):
			return ;
	}
}

void	Request::changeToLower(char& c)
{
	if (c >= 'A' && c <= 'Z')
		c += 32;
}

void	Request::parseRequest(str& request)
{
	std::istringstream	reqStream(request);
	str					line;
	str					headerFieldContent;
	t_header 			str_case;
	
	std::getline(reqStream, line);
	parseStatusLine(line);
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
		std::for_each(headerFieldContent.begin(), headerFieldContent.end(), changeToLower);
		setRequestFields(str_case, headerFieldContent);
	}
}

int	Request::getFileFD() const
{
	return (this->fileFD);
}
