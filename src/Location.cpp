#include "Location.hpp"

Location::Location(): BlockOBJ()
{

}

Location::Location(const Location &copy): BlockOBJ(copy)
{
	(void)copy;
}

Location::~Location()
{
	for(std::vector<Location *>::iterator it = locations.begin(); it != locations.end(); it++)
    	delete *it;
}

bool Location::handleDirective(std::queue<str> opts)
{
	(void) opts;
	return false;
}

BlockOBJ *Location::handleBlock(std::queue<str> opts)
{
	if (opts.size() < 2 || opts.front() != "location")
		return NULL;
	locations.push_back(new Location());
	return locations.back();
}

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}
