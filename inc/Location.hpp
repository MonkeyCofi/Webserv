#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <exception>
#include "BlockOBJ.hpp"

typedef std::string str;

class Location: public BlockOBJ
{
	private:

	public:
		Location();
		Location(const Location &copy);
		~Location();

		bool	handleDirective(std::vector<str> opts);
		bool	handleBlock(std::queue<str> opts);

		const Location &operator =(const Location &copy);
};

#endif