#include "Engine.hpp"

Engine::Engine()
{

}

Engine::Engine(const Engine &copy)
{
	(void)copy;
}

Http &Engine::getProtocol()
{
	return protocol;
}

Engine::~Engine()
{

}

const Engine &Engine::operator =(const Engine &copy)
{
	(void)copy;
	return *this;
}
