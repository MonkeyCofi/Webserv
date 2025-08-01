# ifndef CGIINFO_HPP
# define CGIINFO_HPP

# include <unistd.h>
# include <iostream>
# include "Response.hpp"

typedef std::string str;

class CGIinfo
{
	private:
		int		client_fd;
		str		response_str;
		bool	response_complete;
		pid_t	child_pid;
		str     header;
		bool	parsed;
	public:
		CGIinfo();
		CGIinfo(int _client, pid_t _child);
		CGIinfo(const CGIinfo& obj);
		~CGIinfo();

		CGIinfo&    operator=(const CGIinfo& obj);
		void        concatBuffer(std::string str);
		pid_t       getPid() const;
		int         getClientFd() const;
		bool        isComplete() const;
		void        completeResponse();
		std::string getBuffer() const;
		void        printInfo() const;
		Response    parseCgiResponse();

		static bool		charEq(const char& c1, const char& c2);
		size_t			nameFound(str& to_search, str search_val);
		str				getValue(str& main_str, str key, size_t key_start);
		bool			getParsed() const;
		void    		setParsed(bool parsed);
};

# endif
