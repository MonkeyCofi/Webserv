#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include <string>
#include <cctype>
#include "Engine.hpp"

typedef std::string str;

class ConfigParser
{
	private:
		enum e_next {
			DEFAULT,
			WORD_SEMI,
			OPEN_BRAC
		};

		const static str	directives[];
		str		err_msg;
		int		inBlock;
		e_next	expected;

		str		toString(int x);
		bool	isWhitespace(char c);
		bool	isControl(char c);
		bool	isValidNext(str &next);
		bool	isValidDirective(str &word);
		void	skipWhitespace(str &line);
		bool	validFilename(str &fn);
		bool	handleNext(str &word);
		bool	parseLine(str &line);
		str		parseNext(str &line);

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