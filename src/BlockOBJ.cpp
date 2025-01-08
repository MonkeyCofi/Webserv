#include "BlockOBJ.hpp"

BlockOBJ::BlockOBJ()
{

}

BlockOBJ::BlockOBJ(const BlockOBJ &copy)
{
	(void)copy;
}

BlockOBJ::~BlockOBJ()
{

}

bool BlockOBJ::inDirectives(const str &dir, const str *dirs) const
{
	for (int i = 0; dirs[i] != ""; i++)
		if (dir == dirs[i])
			return true;
	return false;
}

const BlockOBJ &BlockOBJ::operator=(const BlockOBJ &copy)
{
	(void)copy;
	return *this;
}
