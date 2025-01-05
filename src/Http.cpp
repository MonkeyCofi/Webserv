#include "Http.hpp"

Http::Http()
{

}

Http::Http(const Http &copy)
{
	(void)copy;
}

bool Http::handleDirective(std::queue<str> opts)
{
	(void) opts;
	return true;	
}
BlockOBJ *Http::handleBlock(std::queue<str> opts)
{
	if (opts.front() != "server")
		return NULL;
	servers.push_back(new Server());
	return servers.back();
}

Http::~Http()
{
	for(std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
    	delete *it;
}

const Http &Http::operator =(const Http &copy)
{
	(void)copy;
	return *this;
}
