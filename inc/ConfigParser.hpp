#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include "Engine.hpp"

typedef std::string str;

class ConfigParser
{
	private:
		enum e_next {
			WORD,
			WORD_SEMI,
			WORD_SEMI_NO_BRAC,
			SEMICOLON,
			OPEN_BRAC
		};

		int		inBlock;
		e_next	expected;

		bool	isWhitespace(char c);
		bool	isControl(char c);
		bool	isValidNext(str &next);
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