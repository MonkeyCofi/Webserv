#include "Server.hpp"

Server::Server()
{

}

Server::Server(const Server &copy)
{
	(void)copy;
}

Server::~Server()
{

}

const Server &Server::operator =(const Server &copy)
{
	(void)copy;
	return *this;
}
