#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <exception>
#include "Http.hpp"

typedef std::string str;

class Engine
{
	private:
		Http		protocol;

	public:
		Engine();
		Engine(const Engine &copy);
		~Engine();

		Http	&getProtocol();

		const Engine &operator =(const Engine &copy);
};

#endif
