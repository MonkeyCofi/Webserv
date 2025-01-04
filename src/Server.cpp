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

bool Server::handleDirective(std::queue<str> opts)
{
	(void) opts;
	return false;
}

BlockOBJ *Server::handleBlock(std::queue<str> opts)
{
	(void) opts;
	return nullptr;
}

const Server &Server::operator =(const Server &copy)
{
	(void)copy;
	return *this;
}
