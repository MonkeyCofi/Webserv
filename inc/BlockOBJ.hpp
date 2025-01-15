#ifndef BLOCKOBJ_HPP
#define BLOCKOBJ_HPP

#include <iostream>
#include <exception>
#include <queue>

typedef std::string str;

class BlockOBJ
{
	private:
		const static str	directives[];

	public:
		BlockOBJ();
		BlockOBJ(const BlockOBJ &copy);
		virtual ~BlockOBJ();

		virtual bool		handleDirective(std::queue<str> opts)	= 0;
		virtual BlockOBJ	*handleBlock(std::queue<str> opts)		= 0;

		const BlockOBJ &operator =(const BlockOBJ &copy);
};

#endif
