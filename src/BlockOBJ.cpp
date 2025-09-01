#include "BlockOBJ.hpp"

BlockOBJ::BlockOBJ()
{

}

BlockOBJ::BlockOBJ(const BlockOBJ &copy)
{
	(void)copy;
}

bool BlockOBJ::handleDirective(std::queue<str> opts)
{
	if (opts.size() == 0)
		return false;
	return true;
}

bool BlockOBJ::inDirectives(const str &dir, const str *dirs) const
{
	for (unsigned int i = 0; dirs[i] != ""; i++)
		if (dir == dirs[i])
			return true;
	return false;
}

const BlockOBJ &BlockOBJ::operator=(const BlockOBJ &copy)
{
	(void)copy;
	return *this;
}

BlockOBJ::~BlockOBJ()
{

}
