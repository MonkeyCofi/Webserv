#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <exception>
#include "Engine.hpp"

typedef std::string str;

class ConfigParser
{
	private:
		bool	validFilename(const str &fn);

	public:
		ConfigParser();
		ConfigParser(const str &fn);
		ConfigParser(const ConfigParser &copy);
		~ConfigParser();
		const ConfigParser &operator =(const ConfigParser &copy);

		Engine	parse(const str &fn);

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