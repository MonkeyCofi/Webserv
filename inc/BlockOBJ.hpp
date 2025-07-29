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

# define BUFFER_SIZE 4096	// 4kb buffer size; need to fix case where buffer size is a small number such as 32
class BlockOBJ
{
	protected:
		bool		inDirectives(const str &dir, const str *dirs)	const;

	public:
		BlockOBJ();
		BlockOBJ(const BlockOBJ &copy);
		virtual ~BlockOBJ();

		virtual bool		handleDirective(std::queue<str> opts);
		virtual BlockOBJ	*handleBlock(std::queue<str> opts) = 0;
		virtual str			getType() = 0;

		const BlockOBJ &operator =(const BlockOBJ &copy);
};

#endif
