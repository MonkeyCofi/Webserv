/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/04/17 21:46:21 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <algorithm>
# include <sstream>

typedef std::string str;

class Request
{
	private:
		str		host;
		str		method;
		str		status;
		str		file_URI;
		bool	keepAlive;
		str		contentType;
		bool	validRequest;
		size_t	contentLength;
		str		body_boundary;

		static bool	is_digit(char c);
		static void	changeToLower(char &c);
		bool		parseRequestLine(str &line);
		void		setRequestField(str &header_field, str &field_conent);

		Request();
		Request(const Request& obj);

	public:
		Request(str request);
		~Request();
		Request		&operator=(const Request& obj);

		Request	&parseRequest(str& request);

		bool	shouldKeepAlive();
		bool	isValidRequest();
		str		getFileURI();
		str		getContentType();
		size_t	getContentLen();
		str		getMethod();
		str		getStatus();
		str		getHost();
		str		getBoundary() const;

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
