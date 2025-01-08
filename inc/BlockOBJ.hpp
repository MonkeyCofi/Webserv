#ifndef BLOCKOBJ_HPP
#define BLOCKOBJ_HPP

#include <iostream>
#include <exception>
#include <queue>
#include <map>

typedef std::string str;

class BlockOBJ
{
	protected:
		int			min_del_depth;
		str			root_dir;
		bool		autoindex;
		long long	clienx_max_body;
		std::map<std::string, std::string>	error_pages;

	public:
		BlockOBJ();
		BlockOBJ(const BlockOBJ &copy);
		virtual ~BlockOBJ();

		virtual bool		handleDirective(std::queue<str> opts)	= 0;
		virtual BlockOBJ	*handleBlock(std::queue<str> opts)		= 0;

		bool	inDirectives(const str &dir, const str *dirs)	const;

		const BlockOBJ &operator =(const BlockOBJ &copy);
};

#endif
