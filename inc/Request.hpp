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

typedef std::string str;

class Request
{
	private:
		str		method;
		str		serveFile;
		str		contentType;
		int		fileFD;
		str		host;
		size_t	contentLength;
		bool	keepAlive;	// either keep-alive, or close
	public:
		Request();
		Request(str& request);
		~Request();
		Request(const Request& obj);
		Request		&operator=(const Request& obj);

		Request	parseRequest(str& request);
		int		getFileFD();

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
