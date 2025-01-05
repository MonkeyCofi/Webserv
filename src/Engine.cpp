#include "Engine.hpp"

Engine::Engine(): protocol(NULL)
{
	protocol = new Http();
}

Engine::Engine(const Engine &copy)
{
	(void)copy;
}

Http *Engine::getProtocol()
{
	return protocol;
}

Engine::~Engine()
{
	delete protocol;
}

const Engine &Engine::operator =(const Engine &copy)
{
	(void)copy;
	return *this;
}
