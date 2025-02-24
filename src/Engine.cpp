#include "Engine.hpp"

Engine::Engine(): protocol(NULL), cm(NULL)
{
	protocol = new Http();
}

Engine::Engine(const Engine &copy): cm(NULL)
{
	if (!copy.protocol)
		protocol = new Http();
	else
		protocol = new Http(*copy.protocol);
}

Http *Engine::getProtocol()
{
	return protocol;
}

void	Engine::start()
{
	if (!protocol)
		throw std::exception();
	if (!cm)
		cm = new ConnectionManager(protocol);
	cm->startConnections();
}

const Engine &Engine::operator =(const Engine &copy)
{
	if (protocol)
		delete protocol;
	if (cm)
		delete cm;
	if (!copy.protocol)
		protocol = new Http();
	else
		protocol = new Http(*copy.protocol);
	cm = NULL;
	return *this;
}

Engine::~Engine()
{
	if (protocol)
		delete protocol;
	if (cm)
		delete cm;
}
