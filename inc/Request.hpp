/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/01 16:13:41 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <algorithm>

typedef std::string str;

class Request
{
	private:
		str		host;
		str		method;
		int		status;
		str		serveFile;
		str		contentType;
		size_t	contentLength;
		bool	keepAlive;
		bool	validRequest;

	public:
		Request();
		Request(str& request);
		~Request();
		Request(const Request& obj);
		Request		&operator=(const Request& obj);

		Request	parseRequest(str& request);

		bool	isValidRequest();
		str		getServeFile();
		str		getMethod();
		str		getHost();
		bool	shouldKeepAlive();
		str		getContentType();
		size_t	getContentLength();

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
