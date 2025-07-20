#include "Response.hpp"

Response::Response(): root("/"), header(""), body(""), code("200"), chunked(false), header_sent(false), keep_alive(false), fd(-1)
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

Response::Response(const Response &copy)
{
	this->root = copy.root;
	this->header = copy.header;
	this->body = copy.body;
	this->code = copy.code;
	this->chunked = copy.chunked;
	this->header_sent = copy.header_sent;
	this->fd = copy.fd;
	this->keep_alive = copy.keep_alive;
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

Response::Response(str code): root("/"), header(""), body(""), code(code), chunked(false), header_sent(false), keep_alive(false), fd(-1)
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

void Response::setHeaderField(str field, str value)
{
	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
	this->header += field + ": " + value + "\r\n";
}

void Response::setHeaderField(str field, ssize_t value)
{
	std::stringstream	ss;

	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
	ss << field << ": " << value << "\r\n";
	this->header += ss.str();
}

void Response::setBodyFd(int fd)
{
	this->fd = fd;
	this->chunked = true;
	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
	this->setHeaderField("Transfer-Encoding", "Chunked");
}

void Response::setKeepAlive(bool keepAlive)
{
	this->keep_alive = keepAlive;
	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
	this->setHeaderField("Connection", (keepAlive? "Keep-Alive" : "close"));
}

void Response::setBody(str body, str type)
{
	if (type == "")
		type = "text/plain";
	this->chunked = false;
	this->body = body;
	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
	this->setHeaderField("Content-Type", type);
	this->setHeaderField("Content-Length", body.length() - 2);
}

void Response::setCode(str code)
{
	this->code = code;
	this->header = "HTTP/1.1 " + this->code;
	if (http_codes.find(this->code) != http_codes.end())
		this->header += " " + http_codes[this->code];
	this->header += "\r\n";
}

str	Response::errorPage(str code) const
{
	if (http_codes.find(code) == http_codes.end())
		return "<html><head><title>Error Page</title></head><body><h1>Error Page</h1><p>Unknown Error Code</p></body></html>\r\n";
	return "<html><head><title>Error Page</title></head><body><h1>Error Code " + code + "</h1><p>" + http_codes.at(code) + "!</p></body></html>\r\n";
}

void Response::setHeaderSent(bool sent)
{
	this->header_sent = sent;
}

bool Response::isChunked() const
{
	return chunked;
}

bool Response::doneSending() const
{
	std::cout << std::boolalpha << "header sent: " << header_sent << "\nChunked: " << chunked 
		<< "\nfd: " << fd << "\n";
	return (header_sent && !chunked) || (chunked && fd == -1);
}

bool Response::keepAlive() const
{
	return keep_alive;
}

bool Response::headerSent() const
{
	return header_sent;
}

str Response::getHeader()
{
	if (this->header == "")
	{
		this->header = "HTTP/1.1 " + this->code;
		if (http_codes.find(this->code) != http_codes.end())
			this->header += " " + http_codes[this->code];
		this->header += "\r\n";
	}
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

void Response::clear()
{
	this->fd = -1;
	this->body = "";
	this->code = "";
	this->header = "";
	this->chunked = false;
	this->header_sent = false;
	this->keep_alive = false;
}

Response &Response::operator =(const Response &copy)
{
	this->header = copy.header;
	this->body = copy.body;
	this->code = copy.code;
	this->chunked = copy.chunked;
	this->header_sent = copy.header_sent;
	this->fd = copy.fd;
	this->keep_alive = copy.keep_alive;
	return (*this);
}

void Response::setRoot(const str &str)
{
	this->root = str;
}

str Response::getRoot() const
{
	return this->root;
}

void Response::setAutoIndex(bool autoidx)
{
	this->autoindex = autoidx;
}

bool Response::getAutoIndex() const
{
	return this->autoindex;
}

Response::~Response()
{

}

void	Response::printResponse()
{
	std::cout << "Header: \n";
	std::cout << this->header << "\n";
	std::cout << "Body: \n";
	std::cout << this->body << "\n";
}

size_t	Response::getSentBytes() const
{
	return (this->sent_bytes);
}

void	Response::addSentBytes(size_t value)
{
	this->sent_bytes += value;
}

const std::vector<str>& Response::getIndexFiles() const {
	return this->index_files;
}

void Response::setIndexFiles(const std::vector<str>& files) {
	this->index_files = files;
}
