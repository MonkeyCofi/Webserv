#include "ConfigParser.hpp"

ConfigParser::ConfigParser(): inBlock(0), expected(DEFAULT)
{

}

bool ConfigParser::validFilename(str &fn)
{
	std::size_t	n;

	n = fn.length();
	if (n < 4 || fn.find(".conf") == str::npos)
		return false;
	return true;
	// return (fn[n - 5] == '.' && fn[n - 4] == 'c'
	// 	&& fn[n - 3] == 'o' && fn[n - 2] == 'n'
	// 	&& fn[n - 1] == 'f');
}

bool ConfigParser::isWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool ConfigParser::isControl(char c)
{
	return c == ';' || c == '{' || c == '}';
}

bool ConfigParser::isValidNext(str &next)
{
	if (next == "")
		return true;
	if ((expected == DEFAULT && next[0] == '{')
		|| (expected == OPEN_BRAC && (next[0] == ';' || next[0] == '}'))
		|| (expected == WORD_SEMI && (next[0] == '{' || next[0] == '}'))
		|| (!inBlock && next[0] == '}'))
		return false;
	return true;
}

void ConfigParser::skipWhitespace(str &line)
{
	unsigned int	i;

	for (i = 0; i < line.length() && isWhitespace(line[i]); i++);
	line = line.substr(i);
}

str ConfigParser::parseNext(str &line)
{
	unsigned int	i;
	str				next;

	if (isControl(line[0]))
	{
		next = line[0];
		line = line.substr(1);
		return next;
	}
	for (i = 0; i < line.length() && !isWhitespace(line[i]) && !isControl(line[i]); i++);
	next = line.substr(0, i);
	line = line.substr(i);
	return next;
}

bool ConfigParser::handleNext(str &word)
{
	inBlock += (word == "{") - (word == "}");
	if (isControl(word[0]))
		expected = DEFAULT;
	else if (expected == DEFAULT)
	{
		expected = WORD_SEMI;
		if (word == "http" || word == "server" || word == "location")
			expected = OPEN_BRAC;
	}
	return true;
}

bool ConfigParser::parseLine(str &line)
{
	str	word;

	skipWhitespace(line);
	while (line != "")
	{
		word = parseNext(line);
		if (!isValidNext(word) || !handleNext(word))
			return false;
		skipWhitespace(line);
	}
	return true;
}

Engine ConfigParser::parse(str &fn)
{
	if (!validFilename(fn))
		throw FilenameError("Invalid file name!");

	Engine			eng;
	str				line, cpy;
	int				i = 0;
	std::ifstream 	file;

	file.open(fn.c_str());
	if (!file)
		throw InvalidFileError("Could not open file!");

	while (getline(file, line))
	{
		cpy = line;
		i++;
		if (!parseLine(cpy))
			throw InvalidFileError("Syntax error in the file!\nLine: " + line);
	}
	if (inBlock)
		throw InvalidFileError("Incomplete file!\nReached EOF before end of block.");
	if (expected != DEFAULT)
		throw InvalidFileError("Incomplete file!\nReached EOF before end of directive.");
	return (eng);
}

ConfigParser::~ConfigParser()
{

}

ConfigParser::FilenameError::FilenameError(const char* message): errmsg(message) {}
ConfigParser::FilenameError::FilenameError(const std::string& message): errmsg(message) {}
const char* ConfigParser::FilenameError::what() const throw()
{
	return errmsg.c_str();
}
ConfigParser::FilenameError::~FilenameError() throw() {}
ConfigParser::InvalidFileError::InvalidFileError(const char* message): errmsg(message) {}
ConfigParser::InvalidFileError::InvalidFileError(const std::string& message): errmsg(message) {}
const char* ConfigParser::InvalidFileError::what() const throw()
{
	return errmsg.c_str();
}
ConfigParser::InvalidFileError::~InvalidFileError() throw() {}