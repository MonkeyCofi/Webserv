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

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <algorithm>
# include <vector>
# include <sstream>
# include <fstream>

typedef std::string str;

class Request
{
	private:
		str					host;
		str					method;
		str					status;
		str					file_URI;
		bool				has_body;
		bool				keepAlive;
		str					contentType;
		bool				validRequest;
		size_t				contentLength;
		str					body_boundary;
		bool				fullyReceived;
		bool				headerReceived;
		std::fstream		bodyFile;

		static bool	is_digit(char c);
		static void	changeToLower(char &c);
		bool		parseRequestLine(str &line);
		void		setRequestField(str &header_field, str &field_conent);

		std::vector<std::string>	request;

		Request(const Request& obj);

	public:
		Request();
		Request(str request);
		~Request();
		Request		&operator=(const Request& obj);

		Request	&parseRequest(str& request);
		void	pushRequest(std::string& req);

		std::vector<str>	fileNames;

		bool			shouldKeepAlive();
		bool			isValidRequest();
		void			setHasBody(const bool status);
		
		// getters
		bool			getFullyReceived() const;
		bool			getHasBody() const;
		bool			getHeaderReceived() const;
		str				getFileURI();
		str				getContentType();
		size_t			getContentLen();
		str				getMethod();
		str				getStatus();
		str				getHost();
		str				getBoundary() const;
		str				getRequest() const;
		std::fstream&	getBodyFile();
		// getters

		void	setFullyReceived(const bool status);
		void	setHeaderReceived(const bool status);

		class	NoHostException: public std::exception
		{
			const char*	what() const throw();
		};
};

#endif
