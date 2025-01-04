#include "Http.hpp"

Http::Http()
{

}

Http::Http(const Http &copy)
{
	(void)copy;
}

bool	Http::handleDirective(std::queue<str> opts)
{
	
}
bool	Http::handleBlock(std::queue<str> opts)
{
	if (opts.front() != "server")
		return NULL;
	servers.push(new Server());
	return servers.back();
}

Http::~Http()
{

}

const Http &Http::operator =(const Http &copy)
{
	(void)copy;
	return *this;
}
