#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <exception>

typedef std::string str;

class Location
{
	private:

	public:
		Location();
		Location(const Location &copy);
		~Location();
		const Location &operator =(const Location &copy);
};

#endif