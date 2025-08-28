/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehammoud <ehammoud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/16 16:48:00 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/05 12:35:07 by ehammoud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONMANAGER_HPP
# define CONNECTIONMANAGER_HPP

# include <netdb.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <iostream>
# include <fstream>
# include <exception>
# include <algorithm>
# include <cerrno>
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <poll.h>
# include <signal.h>
# include <fcntl.h>
# include <cstring>
# include <map>
# include <set>
# include <cstdio>

# include "BlockOBJ.hpp"
# include "Http.hpp"
# include "Server.hpp"
# include "Request.hpp"

// # include "Cgi.hpp"
# include "CGIinfo.hpp"

// # define BUFFER_SIZE 4096	// 4 kb

/* Colors */
# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN "\033[36m"
# define RESET "\033[0m"
# define NL "\033[0m\n"
# define FLUSH "\033[0m" << std::endl;
/* Colors */

# define WRITE 1
# define READ 0

typedef std::string str;

class Cgi;

class ConnectionManager
{
	private:
		static unsigned int	number;
		typedef enum
		{
			IGNORE = -1,
			INVALID,
			INCOMPLETE,
			HEADER_FINISHED,
			FINISH
		}	State;
	
		unsigned int							main_listeners;
		std::vector<Server *>					defaults;
		std::vector<Server *>					handlers;
		std::vector<str>						reqs;
		int										fd;
		std::vector<struct pollfd>				sock_fds;
		std::vector<str>						tempFileNames;
		std::vector<std::map<str, Server *>	>	servers_per_ippp;
		std::map<int, Request *>				requests;
		std::map<int, CGIinfo>	 				cgiProcesses;
		std::string								request_header;
		std::fstream							request_body;
		int										cgi_pipes[2];
		
		ConnectionManager();
		ConnectionManager(const ConnectionManager &obj);
		ConnectionManager	&operator=(const ConnectionManager &obj);
		
		State		receiveRequest(int client_fd, Request* req, unsigned int& index, State& state);
		int			setupSocket(str ip, str port);
		void		addServerToMap(std::map<str, Server *>	&map, Server &server);
		void		addSocket(str ip, str port);
		void		newClient(int i, struct pollfd sock);
		void		printError(int revents);

		void		passRequestToServer(unsigned int& i, Request **req);

		void		deleteTempFiles();

		void		handlePollin(unsigned int& i, State& state, std::map<int, Request *>& requests);
		void		handlePollout(State& state, unsigned int& i, std::map<int, Request *> &requests);

		bool		handleCGIPollout(unsigned int& i);
		void		handleCGIread(unsigned int& i);
		void		handleCgiPollhup(unsigned int& i);

		void		reapProcesses();

		void		deleteRequest(unsigned int i);

	public:
		ConnectionManager(Http *protocol);
		void		closeSocket(unsigned int& index);
		void		closeSocketNoRef(unsigned int index);

		int			fileUpload(Request* req, Location* location, char *buffer, size_t size);


		void	startConnections();
		std::vector<struct pollfd>& getPollFds();
		std::map<int, CGIinfo>& getCgiProcesses();
		void debugVectorSizes(const std::string& location);
		void	addFdtoPoll(unsigned int i, struct pollfd fd);

		~ConnectionManager();

		class	bindException: public std::exception
		{
			public:
				const char* what() const throw();
		};
};

#endif
