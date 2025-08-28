/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/08/02 15:03:28 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <algorithm>
# include <vector>
# include <sstream>
# include <fstream>

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN "\033[36m"
# define RESET "\033[0m"
# define NL "\033[0m\n"

# define MAX_HEADER_SIZE 5000

typedef std::string str;

class	ConnectionManager;
class	Cgi;

class Request
{
	private:
		std::string			host;
		std::string			method;
		std::string			destination_URI;
		std::string			status;
		std::string			file_URI;
		std::string			header;
		std::string			contentType;
		std::string			body_boundary;
		std::string			fileType;
		std::fstream		bodyFile;
		size_t				contentLength;
		size_t				received_body_bytes;
		bool				keepAlive;
		bool				isCGIrequest;
		bool				has_body;
		bool				is_chunked;
		bool				validRequest;
		bool				fullyReceived;
		bool				headerReceived;
		bool				partial_request;
		Cgi*				cgi;
		bool				cgi_started;
		int					bodyFd;
		char				*left_overs;
		size_t				left_over_size;

		bool		parseRequestLine(str &line);
		void		setRequestField(str &header_field, std::string &field_conent);
		
		Request(const Request& obj);
	public:
		Request();
		Request(std::string request);
		~Request();
		Request		&operator=(const Request& obj);

		size_t				bytesReceived;
		std::string					body_boundaryEnd;

		Request	&parseRequest(std::string& request);
		int	pushRequest(std::string& req);
		int	pushBody(char *buffer, size_t size);

		bool			shouldKeepAlive();
		bool			isValidRequest();
		void			setHasBody(const bool status);
		
		/********************/
		/*		getters		*/
		/********************/
		const std::string&		getFileURI() const;
		const std::string&		getContentType() const;
		const std::string&		getMethod() const;
		const std::string&		getStatus() const;
		const std::string&		getHost() const;
		const std::string&		getBoundary() const;
		const std::string&		getDestURI() const;
		const std::string&		getHeader() const;
		int						getBodyFd() const;
		size_t					getContentLen();
		size_t					getReceivedBytes() const;
		bool					getFullyReceived() const;
		bool					getHasBody() const;
		bool					getHeaderReceived() const;
		bool					isCGI() const;
		int						getCGIfd() const;

		/********************/
		/*		setters		*/
		/********************/
		void	setDestURI(const str &dest);
		void	setFullyReceived(const bool status);
		void	setHeaderReceived(const bool status);
		void	setCgi(Cgi* _cgi);
		bool	getCGIstarted() const;
		void	setCGIstarted();
		void	setPartialRequest(bool cond);
		bool	isPartial() const;
		bool	isCompleteRequest() const;
		bool	isChunked() const;
		void	setStatus(const str &status);
		void	setValid(bool valid);
		void	setLeftOvers(char* buf, size_t r);
		char*	getLeftOvers() const;
		void	addReceivedBytes(size_t received);
		size_t	getLeftOverSize() const;
		Cgi*	getCgiObj();
		
		void	deleteLeftOvers();

		class	NoHostException: public std::exception
		{
			const char*	what() const throw();
		};
};

#endif
