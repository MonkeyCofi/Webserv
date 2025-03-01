#include "Http.hpp"
 
const str	Http::directives[] = { "root", "index", "error_page", "client_max_body_size", "min_delete_depth", "autoindex", "" };

Http::Http(): BlockOBJ()
{

}

Http::Http(const Http &copy): BlockOBJ(copy)
{
	
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
	return servers.back();
}

std::vector<Server *> Http::getServers()
{
	return servers;
}

std::vector<Socket *>	Http::getListeners()
{
	return (this->listeners);
}

str	Http::getType()
{
	return ("http");
}

void	Http::init_listeners()
{
	std::cout << "There are " << servers.size() << " servers\n";
	try
	{
		for (unsigned int i = 0; i < servers.size(); i++)
		{
			// create a listener for each listen directive
			std::vector<str>	ports = servers.at(i)->getPorts();
			unsigned int j = 0;
			for (; j < ports.size(); j++)
			{
				Socket *temp = new Socket(*this->servers.at(i), j);
				this->listeners.push_back(temp);
			}
			std::cout << "There are " << j + 1 << " listeners for server " << i + 1 << "\n";
		}
	}
	catch (std::exception())
	{
		std::cout << "uwu\n";
	}
	catch (std::out_of_range())
	{
		std::cout << "Index is out of range\n";
	}
}

void	Http::printPortsIpsNames()
{
	for (unsigned int i = 0; i < servers.size(); i++)
	{
		//for (std::vector<str>::iterator it = this->names.begin(); it != this->names.end(); it++)
		std::vector<str>	names = this->servers.at(i)->getNames();
		std::vector<str>	ips = this->servers.at(i)->getIPs();
		std::vector<str>	ports = this->servers.at(i)->getPorts();
		for (std::vector<str>::iterator it = names.begin(); it != names.end(); it++)
			std::cout << "name: " << *it << "\n";
		for (std::vector<str>::iterator it = ips.begin(); it != ips.end(); it++)
			std::cout << "ip addr: " << *it << "\n";
		for (std::vector<str>::iterator it = ports.begin(); it != ports.end(); it++)
			std::cout << "port: " << *it << "\n";
		std::cout << "\n";
	}
}

const Http &Http::operator =(const Http &copy)
{
	(void)copy;
	return *this;
}

Http::~Http()
{
	for(std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
    	delete *it;
	for(std::vector<Socket *>::iterator it = listeners.begin(); it != listeners.end(); it++)
    	delete *it;
}
