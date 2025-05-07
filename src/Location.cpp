#include "Location.hpp"

const str	Location::directives[] = { "root", "index", "client_max_body_size", "min_delete_depth", "alias", "autoindex", "return", "" };

Location::Location(): BlockOBJ(), alias()
{

}

Location::Location(const Location &copy): BlockOBJ(copy), alias(copy.getAlias())
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
	bool			parent_ret;

	if (opts.size() == 0 || !inDirectives(opts.front(), directives))
		return false;
	parent_ret = BlockOBJ::handleDirective(opts);
	if (opts.front() == "alias" && opts.size() == 2)
	{
		opts.pop();
		alias = opts.front();
	}
	else if (opts.front() == "return" && opts.size() >= 2 && opts.size() <= 3)
	{
		opts.pop();
		ret_code = atoi(opts.front().c_str());
		opts.pop();
		if (opts.size() > 0)
			ret_str = opts.front();
	}
	else
		return parent_ret;
	return true;
}

BlockOBJ *Location::handleBlock(std::queue<str> opts)
{
	if (opts.size() < 2 || opts.front() != "location")
		return NULL;
	locations.push_back(new Location());
	return locations.back();
}

void Location::setAlias(const str &s)
{
	alias = s;
}

str Location::getAlias() const
{
	return alias;
}

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}

str	Location::getType()
{
	return ("location");
}

std::vector<Location *>	Location::getLocations()
{
	return locations;
}
