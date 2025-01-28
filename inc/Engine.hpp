#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <exception>
#include <fstream>
#include "Http.hpp"
#include "tmpdel.hpp"

typedef std::string str;

class Engine
{
	private:
		Http		*protocol;

	public:
		Engine();
		Engine(const Engine &copy);
		~Engine();

		Http	*getProtocol();

		const Engine &operator =(const Engine &copy);
};

#endif
