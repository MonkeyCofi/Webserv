#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <exception>
#include <fstream>
#include "ConnectionManager.hpp"
#include "Http.hpp"

typedef std::string str;

class Engine
{
	private:
		Http				*protocol;
		ConnectionManager	*cm;

	public:
		Engine();
		Engine(const Engine &copy);
		~Engine();

		Http	*getProtocol();
		void	start();

		const Engine &operator =(const Engine &copy);
};

#endif
