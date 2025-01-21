#ifndef BLOCKOBJ_HPP
#define BLOCKOBJ_HPP

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <cstdlib>

typedef std::string str;

class BlockOBJ
{
	protected:
		int			min_del_depth, client_max_body;
		str			root_dir;
		bool		autoindex;
		std::vector<std::string>			index_pages;
		std::map<std::string, std::string>	error_pages;

	public:
		BlockOBJ();
		BlockOBJ(const BlockOBJ &copy);
		virtual ~BlockOBJ();

		virtual bool		handleDirective(std::queue<str> opts);
		virtual BlockOBJ	*handleBlock(std::queue<str> opts) = 0;

		bool	inDirectives(const str &dir, const str *dirs)	const;

		const BlockOBJ &operator =(const BlockOBJ &copy);
		virtual str	getType();
};

#endif
