#include "Response.hpp"

Response::Response(): code("200"), chunked(false), header_sent(false), fd(-1)
{
	http_codes["200"] = "OK";
	http_codes["201"] = "Created";
	http_codes["202"] = "Accepted";
	http_codes["204"] = "No Content";
	http_codes["301"] = "Redirect";
	http_codes["302"] = "Found";
	http_codes["304"] = "Not Modified";
	http_codes["403"] = "Forbidden";
	http_codes["404"] = "Page Not Found";
	http_codes["414"] = "URI Too Long";
	http_codes["500"] = "Internal Server Error";
	http_codes["501"] = "Not Implemented";
	http_codes["505"] = "HTTP Version Not Supported";
}

Response::Response(Response &copy)
{
	this->header = copy.header;
	this->body = copy.body;
	this->code = copy.code;
	this->chunked = copy.chunked;
	this->header_sent = copy.header_sent;
	this->fd = copy.fd;
	http_codes["200"] = "OK";
	http_codes["201"] = "Created";
	http_codes["202"] = "Accepted";
	http_codes["204"] = "No Content";
	http_codes["301"] = "Redirect";
	http_codes["302"] = "Found";
	http_codes["304"] = "Not Modified";
	http_codes["403"] = "Forbidden";
	http_codes["404"] = "Page Not Found";
	http_codes["414"] = "URI Too Long";
	http_codes["500"] = "Internal Server Error";
	http_codes["501"] = "Not Implemented";
	http_codes["505"] = "HTTP Version Not Supported";
	(void) copy;
}

Response::Response(str code): code(code), chunked(false), header_sent(false), fd(-1)
{
	http_codes["200"] = "OK";
	http_codes["201"] = "Created";
	http_codes["202"] = "Accepted";
	http_codes["204"] = "No Content";
	http_codes["301"] = "Redirect";
	http_codes["302"] = "Found";
	http_codes["304"] = "Not Modified";
	http_codes["403"] = "Forbidden";
	http_codes["404"] = "Page Not Found";
	http_codes["414"] = "URI Too Long";
	http_codes["500"] = "Internal Server Error";
	http_codes["501"] = "Not Implemented";
	http_codes["505"] = "HTTP Version Not Supported";
	(void) code;
}

void Response::setHeaderField(str field, str value)
{
	if (this->header == "")
		this->header = this->code +  " " + http_codes[this->code] + "\r\n";
	this->header += field + ": " + value + "\r\n";
}

void Response::setHeaderField(str field, int value)
{
	std::stringstream	ss;

	if (this->header == "")
		this->header = this->code +  " " + http_codes[this->code] + "\r\n";
	ss << field << ": " << value << "\r\n";
	this->header += ss.str();
}

void Response::setHeaderField(str field, ssize_t value)
{
	std::stringstream	ss;

	if (this->header == "")
		this->header = this->code +  " " + http_codes[this->code] + "\r\n";
	ss << field << ": " << value << "\r\n";
	this->header += ss.str();
}

void Response::setBodyFd(int fd)
{
	this->fd = fd;
}

void Response::setBody(str body)
{
	this->body = body;
}

void Response::setHeaderSent(bool sent)
{
	this->header_sent = sent;
}

bool Response::isChunked() const
{
	return chunked;
}

bool Response::headerSent() const
{
	return header_sent;
}

str Response::getHeader() const
{
	return header + "\r\n";
}

str Response::getBody() const
{
	return body;
}

int Response::getBodyFd() const
{
	return fd;
}

Response &Response::operator =(Response &copy)
{
	(void) copy;
}

~Response()
{

}
