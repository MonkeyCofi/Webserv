/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/04/29 10:46:17 by ppolinta         ###   ########.fr       */
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

class Request
{
	private:
		str					host;
		str					method;
		str					status;
		str					file_URI;
		str					header;
		bool				has_body;
		bool				keepAlive;
		bool				isCGIrequest;
		str					contentType;
		bool				validRequest;
		size_t				contentLength;
		bool				headerReceived;
		str					body_boundary;
		bool				fullyReceived;
		str					tempFileName;
		std::fstream		bodyFile;
		str					request;
		
		static void	changeToLower(char &c);
		bool		parseRequestLine(str &line);
		void		setRequestField(str &header_field, str &field_conent);
		
		Request(const Request& obj);
	public:
		Request();
		Request(str request);
		~Request();
		Request		&operator=(const Request& obj);

		size_t				bytesReceived;
		str					body_boundaryEnd;

		Request	&parseRequest(str& request);
		int	pushRequest(std::string& req);

		bool			shouldKeepAlive();
		bool			isValidRequest();
		void			setHasBody(const bool status);
		
		/********************/
		/*		getters		*/
		/********************/
		bool			getFullyReceived() const;
		bool			getHasBody() const;
		bool			getHeaderReceived() const;
		size_t			getContentLen();
		const str&		getFileURI() const;
		const str&		getContentType() const;
		const str&		getMethod() const;
		const str&		getStatus() const;
		const str&		getHost() const;
		const str&		getBoundary() const;
		const str&		getRequest() const;
		const str&		getTempFileName() const;
		std::fstream&	getBodyFile();

		/********************/
		/*		setters		*/
		/********************/
		void	setFullyReceived(const bool status);
		void	setHeaderReceived(const bool status);
		void	setTempFileName(const str file);

		void	clearVector();

		class	NoHostException: public std::exception
		{
			const char*	what() const throw();
		};
};

#endif
