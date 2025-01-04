#include "Location.hpp"

Location::Location()
{

}

Location::Location(const Location &copy)
{
	(void)copy;
}

Location::~Location()
{

}

bool Location::handleDirective(std::queue<str> opts)
{
	(void) opts;
	return false;
}

BlockOBJ *Location::handleBlock(std::queue<str> opts)
{
	(void) opts;
	return nullptr;
}

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}
