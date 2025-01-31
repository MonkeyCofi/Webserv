/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/01/31 20:07:43 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>

typedef std::string str;

class Request
{
	private:
		str		method;
		str		serveFile;
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

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
