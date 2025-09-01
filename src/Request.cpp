/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:25:05 by pipolint          #+#    #+#             */
/*   Updated: 2025/09/01 12:03:57 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "ConnectionManager.hpp"
#include "Cgi.hpp"
#include "Location.hpp"

Request::Request()
{
	this->header = "";
	this->method = "";
	this->file_URI = "";
	this->host = "";
	this->contentLength = 0;
	this->contentType = "";
	this->is_chunked = false;
	this->keepAlive = true;
	this->validRequest = false;
	this->status = "400";
	this->body_boundary = "";
	this->body_boundaryEnd = "";
	this->headerReceived = false;
	this->fullyReceived = false;
	this->has_body = false;
	this->bytesReceived = 0;
	this->isCGIrequest = false;
	this->received_body_bytes = 0;
	this->partial_request = false;
	this->cgi_started = false;
	this->cgi = NULL;
	this->bodyFd = -1;
	this->left_overs = NULL;
	this->left_over_size = 0;

	// file upload members
	this->nameTemp = NULL;
	this->partial_buffer = NULL;
	this->first_chunk = true;
	this->partial_buffer_size = 0;
	this->partial_size = 0;
	this->partial_index = 0;
	this->upload_file_fd = -1;
}

Request::~Request()
{
	if (this->cgi)
	{
		delete this->cgi;
		this->cgi = NULL;
	}
	if (this->left_overs)
	{
		delete[] this->left_overs;
		this->left_overs = NULL;
	}
	if (this->nameTemp)
	{
		delete[] this->nameTemp;
		this->nameTemp = NULL;
	}
	if (this->partial_buffer)
	{
		delete[] this->partial_buffer;
		this->partial_buffer = NULL;
	}
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
	this->bytesReceived = obj.bytesReceived;
	this->method = obj.method;
	this->is_chunked = obj.is_chunked;
	this->bodyFd = obj.bodyFd;
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
	this->received_body_bytes = obj.received_body_bytes;
	this->partial_request = obj.partial_request;
	this->cgi_started = obj.cgi_started;
	this->cgi = NULL;
	this->left_overs = NULL;
	this->left_over_size = 0;

	// file upload members
	this->nameTemp = NULL;
	this->partial_buffer = NULL;
	this->first_chunk = false;
	this->partial_buffer_size = 0;
	this->partial_size = 0;
	this->partial_index = 0;
	this->upload_file_fd = -1;
	return (*this);
}

const char*	Request::NoHostException::what() const throw()
{
	return ("Request doesn't contain Host header");
}

int	Request::pushRequest(str &req)
{
	// -1: invalid
	//  0: header incomplete
	//  1: header complete
	size_t	pos;

	this->header += req;
	if (this->header.length() > MAX_HEADER_SIZE)
		return 400;
	pos = this->header.find("\r\n\r\n");

	// Header incomplete
	if (pos == str::npos)
		return 0;

	// Header read fully
	if (pos != this->header.length() - 4)
	{
		// There are left-overs!
		req = this->header.substr(pos + 4);
		this->header = this->header.substr(0, pos);
		this->parseRequest(this->header);
		if (!this->isCGI() && this->isChunked())
			return 501;
		if (this->method == "GET" || this->method == "DELETE")
			return 400;
		this->headerReceived = true;
		return 1;
	}
	// No left-overs!
	this->parseRequest(this->header);
	this->headerReceived = true;
	return 1;
}



// int	Request::pushBody(char *buffer, size_t size)
// {
// 	str		line, tmp;
// 	bool	reading_content;

// 	if (size + this->received_body_bytes > this->contentLength)
// 		return 400;
// 	line = str(buffer);
// 	for (int i = 0; i < line.length(); i++)
// 		line[i] = std::tolower(line[i]);
// 	if (line.find("content-type: ") != str::npos)
// 	{
// 		// text/plain -> .txt
// 		// image/x-icon -> .ico
// 		// text/$$ -> .$$
// 		// image/$$ -> .$$
// 		// sound/$$ -> .$$
// 		// video/$$ -> .$$
// 		tmp = line.substr(line.find("content-type: ") + 14);
// 		tmp = tmp.substr(0, tmp.find("\r\n"));
// 		if (tmp.find("text/plain") != str::npos)
// 			this->fileType = ".txt";
// 		else if (tmp.find("image/x-icon") != str::npos)
// 			this->fileType = ".ico";
// 		else if (tmp.find("image/") != str::npos)
// 			this->fileType = "." + tmp.substr(tmp.find("image/") + 6, tmp.find_first_of(" \t"));
// 		else if (tmp.find("sound/") != str::npos)
// 			this->fileType = "." + tmp.substr(tmp.find("sound/") + 6, tmp.find_first_of(" \t"));
// 		else if (tmp.find("video/") != str::npos)
// 			this->fileType = "." + tmp.substr(tmp.find("video/") + 6, tmp.find_first_of(" \t"));
// 		else if (tmp.find("text/") != str::npos)
// 			this->fileType = "." + tmp.substr(tmp.find("text/") + 5, tmp.find_first_of(" \t"));
// 		reading_content = true;
// 	}
// 	if (line.find("content-disposition: ") != str::npos)
// 		reading_content = true;
// 	if (reading_content == true)
// 	{
// 		std::cout << "putting data to file\n";
// 		if (this->bodyFd == -1)
// 		{
// 			char	tempnam[255] = "upload_XXXXXXXXXX";
// 			for (int i = 0; i < 255 - 17 && i < this->fileType.length(); i++)
// 				tempnam[17 + i] = this->fileType[i];
// 			this->bodyFd = mkstemp(tempnam);
// 			fcntl(this->bodyFd, F_SETFL, O_NONBLOCK);
// 			fcntl(this->bodyFd, F_SETFD, FD_CLOEXEC);
// 		}
// 		if (size == 0)
// 		{
// 			if (prevBoundPos)
// 				newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
// 			break ;
// 		}
// 		total += r;
// 		k = 0;
// 		while (prevBoundPos && k < r && k + prevBoundPos < boundary.length() && buffer[k] == boundary[k + prevBoundPos])
// 			k++;
// 		if (k + prevBoundPos == boundary.length())
// 		{
// 			// handle found boundary
// 			break ;
// 		}
// 		else if (prevBoundPos)
// 		{
// 			newFile.write(boundary.substr(0, prevBoundPos).c_str(), prevBoundPos);
// 		}
// 		prevBoundPos = 0;
// 		i = 0;
// 		for (; i < r; i++)
// 		{
// 			j = 0;
// 			while (j < boundary.length() && i + j < r && buffer[i + j] == boundary[j])
// 				j++;
// 			if (j == boundary.length())	// this means some characters matched the boundary at the end of buffer
// 			{
// 				if (i > 0)
// 				{
// 					newFile.write(&buffer[0], i);
// 				}
// 				sheet = false;
// 				break ;
// 			}
// 			if (i + j == r)	// this means some characters matched the boundary at the end of buffer
// 			{
// 				// tempBdr = buffer[j - i];	// create a temp string that contains the boundary characters found so far
// 				prevBoundPos = j;
// 				break;
// 			}
// 		}
// 		if (i && sheet)
// 			newFile.write(&buffer[0], i);
// 		newFile.close();
// 		tempFile.seekg(i);
// 		reading_content = false;
// 		(void)boundaryPosInFile;
// 	}
// }

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
	if (header_field == "transfer-encoding" && field_content == "chunked")
		this->is_chunked = true;
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
	std::cout << BLUE << this->header << NL;
	return (*this);
}

bool	mustCreateFile(Request* req, char* buffer, size_t size)
{
	(void)size;
	if (std::strstr(buffer, "filename=\"") != NULL)
		return true;
	std::string content_type = req->getContentType();
	if (content_type.find("application/") != std::string::npos ||
		content_type.find("image/") != std::string::npos ||
		content_type.find("video/") != std::string::npos ||
		content_type.find("audio/") != std::string::npos ||
		content_type.find("text/") != std::string::npos ||
		content_type.find("font/") != std::string::npos)
	{
		return true;
	}
	return false;
}

const char*	sizestrstr(const char* haystack, const char* needle, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		size_t j = 0;
		while (haystack[i + j] == needle[j])
			j++;
		if (needle[j] == '\0')
			return (haystack + i);
	}
	return (NULL);
}

size_t	partial_boundary_index(const char *haystack, const char *needle, size_t size, size_t& partial_size)
{
	// check at the end of the haystack up until the "\r\n" as that indicates the end of the binary data
	size_t i = size - 1;
	size_t j = 0;
	(void)needle;
	while (i > 0 && haystack[i - 1] != '\r' && haystack[i] != '\n')
		i--;
	i += haystack[i] == '\n';
	while (i < size && needle[j] && haystack[i] == needle[j])
	{
		i++;
		j++;
	}
	if (j == 0)
	{
		// std::cout << "No partial boundary\n";
		return (0);
	}
	partial_size = j;
	// std::cout << YELLOW << "PARTIAL BOUNDARY FOUND" << NL;
	return (i);
}

char	*copy_partial(char *buffer, size_t size)
{
	char* partial = new char[size + 1];
	partial[size] = 0;
	std::memcpy(partial, buffer, size);
	return (partial);
}

int	Request::fileUpload(Location* location, char *buffer, size_t size)
{
	const char* binary_start = std::strstr(buffer, "\r\n\r\n");
	std::string	ending_boundary = this->getBoundary() + "--";
	const char* boundary = ending_boundary.c_str();
	const std::string save_folder = location ? location->getSaveFolder() : "./";
	std::string	upload_location = save_folder != "" \
		? (save_folder.at(save_folder.size() - 1) == '/' ? save_folder : save_folder + '/') \
		: "./";

	// if a partial part of the boundary was found, compare it with the beginning of the current buffer
	if (partial_index != 0)
	{
		// while the new buffer beginning is equal to boundary[size]
		size_t i = 0;
		while (boundary[partial_size] != '\0' && i < size && buffer[i] == boundary[partial_size + i])
			i++;
		if (partial_size + i != std::strlen(boundary))	// if the boundary was indeed not found in the new buffer
			write(this->upload_file_fd, partial_buffer, partial_buffer_size);
		else
		{
			// write the old buffer without the partial boundary
			size_t write_size = partial_buffer_size - partial_size - 2;
			write(this->upload_file_fd, partial_buffer, write_size);
			close(this->upload_file_fd);
			this->upload_file_fd = -1;
			delete [] partial_buffer;
			return (1);
		}
		delete [] partial_buffer;
		partial_buffer = NULL;
		partial_buffer_size = 0;
		partial_index = 0;
	}

	// if the buffer contains the filename, then create a file of that type
	const char* filename_pos = sizestrstr(buffer, "filename=\"", size);
	if (filename_pos)
		std::cout << BLUE << filename_pos << NL;
	// std::cout << "in file upload\n";
	if (first_chunk && filename_pos != NULL && mustCreateFile(this, buffer, size))
	{
		std::cout << "Creating upload file\n";
		std::string file_format = std::string(filename_pos + 10);
		file_format = file_format.substr(0, file_format.find_first_of("\""));
		file_format = file_format.substr(file_format.find_last_of('.') + 1);
		std::cout << "File format: " << file_format << "\n";
		// std::string filename = "./fileuploadXXXXX." + file_format;
		std::string filename = upload_location + "upload_XXXXXX." + file_format;
		this->nameTemp = new char[filename.size() + 1];
		this->nameTemp[filename.size()] = 0;
		strcpy(this->nameTemp, filename.c_str());
		std::cout << this->nameTemp << "\n";
		// this->fd = mkstemp(this->nameTemp);
		std::cout << "suffix size: " << file_format.size() + 1 << "\n";
		this->upload_file_fd = mkstemps(this->nameTemp, file_format.size() + 1);
		if (upload_file_fd == -1)
		{
			perror("mkstemp");
			delete [] this->nameTemp;
			std::cout << "Failed to create upload file\n";
			return (-1);
		}
		fcntl(upload_file_fd, F_SETFL, O_NONBLOCK);
		std::cerr << "created file: " << this->nameTemp << "\n";
		if (binary_start != NULL)	// if this is the first time writing to the file and the there is binary data present
		{
			binary_start += 4;
			size_t binary_size = size - (binary_start - buffer);
			std::cout << "Writing " << binary_size << " bytes of binary data\n";
			first_chunk = false;
			const char* boundary_position = sizestrstr(binary_start, boundary, binary_size);
			if (boundary_position != NULL)
			{
				binary_size = boundary_position - binary_start - 2;
				first_chunk = true;
			}
			write(this->upload_file_fd, binary_start, binary_size);
		}
	}
	else
	{
		// first will be true if there was no data
		
		// check if the boundary is partially found
		// if so, store the position, t, it was found at and start comparing with received buffer's beginning from boundary[t]
		partial_index = partial_boundary_index(buffer, boundary, size, partial_size);
		if (partial_index != 0)	// save the string where the partial boundary was found and write it to the file if the boundary hasn't been found
		{
			partial_buffer = copy_partial(buffer, size);
			partial_buffer_size = size;
			return (0);
		}
		// if any of the partial + the next received characters are equal to boundary, then write up until the partial position
		const char* bpos = sizestrstr(buffer, boundary, size);
		if (bpos)
		{
			std::cout << "Found boundary\n";
			size_t write_size = size - std::strlen(bpos) - 2;
			std::cout << "size: " << size << " write_size: " << write_size << "\n";
			write(upload_file_fd, buffer, write_size);
			close(this->upload_file_fd);
			this->upload_file_fd = -1;
			first_chunk = true;
		}
		else
		{
			// std::cout << "just writing everything\n";
			write(this->upload_file_fd, buffer, size);
		}
	}

	// else
	// {
	// 	// check if the boundary is partially found
	// 	// if so, store the position, t, it was found at and start comparing with received buffer's beginning from boundary[t]
	// 	const char* bpos = sizestrstr(buffer, boundary, size);
	// 	partial_index = partial_boundary_index(buffer, boundary, size, partial_size);
	// 	if (partial_index != 0)	// save the string where the partial boundary was found and write it to the file if the boundary hasn't been found
	// 	{
	// 		partial_buffer = copy_partial(buffer, size);
	// 		partial_buffer_size = size;
	// 		return (0);
	// 	}
	// 	// if any of the partial + the next received characters are equal to boundary, then write up until the partial position
	// 	if (bpos)
	// 	{
	// 		std::cout << "Found boundary\n";
	// 		size_t write_size = size - std::strlen(bpos) - 2;
	// 		std::cout << "size: " << size << " write_size: " << write_size << "\n";
	// 		write(upload_file_fd, buffer, write_size);
	// 		close(this->fd);
	// 		this->fd = -1;
	// 		first = true;
	// 	}
	// 	else
	// 	{
	// 		std::cout << "just writing everything\n";
	// 		write(this->fd, buffer, size);
	// 	}
	// }
	// std::cout << "Received bytes: " << this->getReceivedBytes() << " content length: " << this->getContentLen() << "\n";
	if (this->getReceivedBytes() == this->getContentLen())
	{
		std::cout << "Received content length bytes\n";
		return (1);
	}
	if (this->getReceivedBytes() > this->getContentLen())
		return (-1);
	return (sizestrstr(buffer, boundary, size) != NULL);
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
	return (headerReceived);
}

bool	Request::isChunked() const
{
	return (this->is_chunked);
}

void Request::setStatus(const str &status)
{
	this->status = status;
}

void Request::setValid(bool valid)
{
	this->validRequest = valid;
}

int	Request::getBodyFd() const
{
	return (this->bodyFd);
}

const str& Request::getFileURI() const
{
	return file_URI;
}

const str& Request::getDestURI() const
{
	return destination_URI;
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

void	Request::setFullyReceived(const bool status)
{
	this->fullyReceived = status;
}

void	Request::setHeaderReceived(const bool status)
{
	this->headerReceived = status;
}

void	Request::setHasBody(const bool status)
{
	this->has_body = status;
}

bool	Request::isCGI() const
{
	return (this->isCGIrequest);
}

size_t	Request::getReceivedBytes() const
{
	return (this->received_body_bytes);
}

void	Request::setPartialRequest(bool cond)
{
	this->partial_request = cond;
}

bool	Request::isPartial() const
{
	return (this->partial_request);
}

bool	Request::isCompleteRequest() const
{
	return ((this->headerReceived && this->contentLength == this->received_body_bytes) || (this->headerReceived && !this->contentLength));
}

bool	Request::getCGIstarted() const
{
	return (this->cgi_started);
}

void	Request::setCGIstarted()
{
	this->cgi_started = true;
}

void	Request::setCgi(Cgi* _cgi)
{
	this->cgi = _cgi;
}

void Request::setDestURI(const str &dest)
{
	this->destination_URI = dest;
}

void	Request::setLeftOvers(char* buf, size_t r)
{
	if (buf == NULL)
	{
		this->left_overs = NULL;
		return ;
	}
	std::cout << BLUE << "Buf to copy: " << buf << NL;
	this->left_overs = new char[r + 1]();
	for (size_t i = 0; i < r; i++)
		this->left_overs[i] = buf[i];
	this->left_over_size = r;
}

char*	Request::getLeftOvers() const
{
	return (this->left_overs);
}

size_t	Request::getLeftOverSize() const
{
	return (this->left_over_size);
}

void	Request::deleteLeftOvers()
{
	if (this->left_overs)
	{
		delete [] this->left_overs;
		this->left_overs = NULL;
	}
}

void	Request::addReceivedBytes(size_t received)
{
	this->received_body_bytes += received;
}

Cgi*	Request::getCgiObj()
{
	return (this->cgi);
}

const std::string&	Request::getHeader() const
{
	return (this->header);
}
