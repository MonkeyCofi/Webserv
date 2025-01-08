#include "Location.hpp"

const str	Location::directives[] = { "root", "index", "error_page", "client_max_body_size", "min_delete_depth", "alias", "autoindex", "return", "try_files", "" };

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
	if (opts.size() == 0 || !inDirectives(opts.front(), directives))
		return false;
	if (opts.front() == "alias")
	{
		opts.pop();
		alias = opts.front();
	}
	else if (opts.front() == "alias")
	{
		opts.pop();
		alias = opts.front();
	}
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
