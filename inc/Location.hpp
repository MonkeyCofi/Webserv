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
		str	alias;
		const static str	directives[];

	public:
		std::vector<Location *>	locations;
		Location();
		Location(const Location &copy);
		~Location();

		bool		handleDirective(std::queue<str> opts);
		BlockOBJ	*handleBlock(std::queue<str> opts);

		void	setAlias(const str &s);
		str		getAlias() const;

		const Location &operator =(const Location &copy);
};

#endif
