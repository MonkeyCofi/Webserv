#include "Http.hpp"

const str	Http::directives[] = { "root", "index", "error_page", "client_max_body_size", "min_delete_depth", "autoindex", "" };

Http::Http(): BlockOBJ()
{

}

Http::Http(const Http &copy): BlockOBJ(copy)
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
	if (opts.size() != 1 || opts.front() != "server")
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
