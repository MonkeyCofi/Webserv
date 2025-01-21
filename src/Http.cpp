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
	return false;	
}
BlockOBJ *Http::handleBlock(std::queue<str> opts)
{
	if (opts.size() != 1 || opts.front() != "server")
		return NULL;
	this->servers.push_back(new Server());
	//std::cout << "Size: " << this->servers.size() << "\n";
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

str	Http::getType()
{
	return ("Http");
}

void	Http::init_listeners()
{
	std::cout << "There are " << servers.size() << " servers\n";
	try
	{
		for (unsigned int i = 0; i < servers.size(); i++)
		{
			Socket *temp = new Socket(*this->servers.at(i), i);
			this->listeners.push_back(temp);
			std::cout << "in here\n";
		}
	}
	catch (std::exception())
	{
		std::cout << "uwu\n";
	}
}

void	Http::printPortsIpsNames()
{
	for (unsigned int i = 0; i < servers.size(); i++)
	{
		//for (std::vector<str>::iterator it = this->names.begin(); it != this->names.end(); it++)
		std::vector<str>	names = this->servers.at(i)->returnNames();
		for (std::vector<str>::iterator it = names.begin(); it != names.end(); it++)
			std::cout << "name: " << *it << "\n";
		//	std::cout << "name: " << *it << "\n";
		//for (std::vector<str>::iterator it = this->ips.begin(); it != this->ips.end(); it++)
		//	std::cout << "ips: " << *it << "\n";
		//for (std::vector<str>::iterator it = this->ports.begin(); it != this->ports.end(); it++)
		//	std::cout << "ports: " << *it << "\n";
		std::cout << "\n";
	}
}
