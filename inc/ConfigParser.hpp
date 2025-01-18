#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include <stack>
#include <queue>
#include <string>
#include <cctype>
#include "Engine.hpp"

typedef std::string str;

class ConfigParser
{
	private:
		enum e_next
		{
			DEFAULT,
			WORD_SEMI,
			OPEN_BRAC
		};

		const static str		directives[];
		Engine					webserv;
		str						err_msg;
		int						inBlock;
		e_next					expected;
		std::queue<str>			parsed_opts;
		std::stack<BlockOBJ *>	blocks;

		str		toString(int x)				const;
		bool	isWhitespace(char c)		const;
		bool	isControl(char c)			const;
		bool	isValidNext(str &next);
		bool	isValidDirective(str &word);
		void	skipWhitespace(str &line)	const;
		bool	validFilename(str &fn)		const;
		str		parseNext(str &line)		const;
		bool	parseLine(str &line, Engine &engine);
		bool	handleNext(str &word, Engine &engine);

	public:
		ConfigParser();
		~ConfigParser();

		Engine	parse(str &fn);		

		class FilenameError: public std::exception
		{
			private:
				str errmsg;
			public:
				FilenameError(const char* message);
				FilenameError(const std::string& message);
				~FilenameError() throw();

				const char* what() const throw();
		};
		class InvalidFileError: public std::exception
		{
			private:
				str errmsg;
			public:
				InvalidFileError(const char* message);
				InvalidFileError(const std::string& message);
				~InvalidFileError() throw();

				const char* what() const throw();
		};
};

#endif
