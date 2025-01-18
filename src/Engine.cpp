#include "Engine.hpp"

Engine::Engine(): protocol(NULL)
{
	protocol = new Http();
}

Engine::Engine(const Engine &copy)
{
	(void)copy;
	this->protocol = copy.protocol;
}

Http *Engine::getProtocol()
{
	return protocol;
}

Engine::~Engine()
{
	//printengine(protocol, 0);
	//delete protocol;
}

const Engine &Engine::operator =(const Engine &copy)
{
	(void)copy;
	this->protocol = copy.protocol;
	return *this;
}
