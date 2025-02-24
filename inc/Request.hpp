/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppolinta <ppolinta@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/31 17:11:35 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/16 18:52:24 by ppolinta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <fcntl.h>
# include <vector>
# include <sstream>
# include <cstdlib>

typedef std::string str;

class Request
{
	private:
		str					method;
		str					file_URI;
		str					contentType;
		std::vector<str>	accept_types;
		int					fileFD;
		str					hostIP;
		size_t				contentLength;
		bool				keepAlive;	// either keep-alive, or close

		typedef enum e_header
		{
			none = -1,
			host,
			accept,
			content_length,
			content_type,
			connection
		}	t_header;
		
		void			parseStatusLine(str& request);
		t_header		returnHeader(str line) const;
		
		void			setRequestFields(t_header str_case, str& headerFieldContent);
	public:
		Request();
		~Request();
		Request(const Request& obj);
		Request		&operator=(const Request& obj);

		Request	parseRequest(str& request);
		int		getFileFD() const;

		class	NoHostException
		{
			const char*	what();
		};
};

#endif
