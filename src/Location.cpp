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

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}
