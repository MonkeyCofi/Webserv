/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/01 16:13:41 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <fcntl.h>

typedef std::string str;

class Response
{
	private:
		
	public:
		Response();
		Response(str& request);
		~Response();
		Response(const Response& obj);
		Response		&operator=(const Response& obj);
// req.fileFD = open(req.serveFile.c_str(), O_RDWR);
		Response	parseResponse(str& request);
		int		getFileFD();

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
