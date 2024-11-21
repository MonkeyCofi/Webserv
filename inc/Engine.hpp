#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <iostream>
#include <exception>
#include "Http.hpp"

typedef std::string str;

class Engine
{
	private:
		enum e_loglevel {
			Debug,
			Info,
			Notice,
			Warn,
			Error,
			Crit
		};

		str			username, groupname;
		int			workers;
		Http		protocol;
		str			logfile;
		e_loglevel	loglvl;
		t_pid		pid;

	public:
		Engine();
		Engine(const Engine &copy);
		~Engine();
		const Engine &operator =(const Engine &copy);
};

#endif