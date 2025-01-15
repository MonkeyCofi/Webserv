#include <iostream>
#include "BlockOBJ.hpp"
#include "Http.hpp"
#include "Server.hpp"
#include "Location.hpp"

std::string	getType(BlockOBJ *obj)
{
	if (dynamic_cast<Http *>(obj) != NULL)
		return "Http";
	if (dynamic_cast<Server *>(obj) != NULL)
		return "Server";
	if (dynamic_cast<Location *>(obj) != NULL)
		return "Location";
	return "???";
}

void	printengine(BlockOBJ *obj, int lvl)
{
	std::string type;
	for(int i=0;i < lvl;i++)
		std::cout << "\t";
	type = getType(obj);
	std::cout<<type<<":\n";
	if (type == "Http")
	{
		for(unsigned int i=0;i<((Http *)obj)->servers.size();i++)
			printengine(((Http *)obj)->servers[i], lvl + 1);
	}
	if (type == "Server")
	{
		for(unsigned int i=0;i<((Server *)obj)->locations.size();i++)
			printengine(((Server *)obj)->locations[i], lvl + 1);
	}
	if (type == "Location")
	{
		for(unsigned int i=0;i<((Location *)obj)->locations.size();i++)
			printengine(((Location *)obj)->locations[i], lvl + 1);

	}
}
