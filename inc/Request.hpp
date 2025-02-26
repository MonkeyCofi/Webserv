/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehammoud <ehammoud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/25 17:06:55 by ehammoud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <algorithm>
# include <sstream>
# include <cctype>

typedef std::string str;

class Request
{
	private:
		str		host;
		str		method;
		int		status;
		str		file_URI;
		bool	keepAlive;
		str		contentType;
		bool	validRequest;
		size_t	contentLength;

		static void	changeToLower(char &c);
		bool		parseRequestLine(str &line);
		void		setRequestField(str &header_field, str &field_conent);

	public:
		Request();
		Request(str& request);
		~Request();
		Request(const Request& obj);
		Request		&operator=(const Request& obj);

		Request	&parseRequest(str& request);

		bool	shouldKeepAlive();
		bool	isValidRequest();
		str		getFileURI();
		str		getContentType();
		size_t	getContentLen();
		str		getMethod();
		int		getStatus();
		str		getHost();

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
