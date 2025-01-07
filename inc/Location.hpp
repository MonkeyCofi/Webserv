#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <exception>
#include <queue>
#include "BlockOBJ.hpp"

typedef std::string str;

class Location: public BlockOBJ
{
	private:
		
	public:
		std::vector<Location *>	locations;
		Location();
		Location(const Location &copy);
		~Location();

		bool		handleDirective(std::queue<str> opts);
		BlockOBJ	*handleBlock(std::queue<str> opts);

		const Location &operator =(const Location &copy);
};

#endif
