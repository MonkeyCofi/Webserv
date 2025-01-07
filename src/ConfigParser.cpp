#include "ConfigParser.hpp"

const str	ConfigParser::directives[] = { "root", "listen", "index", "server_name", "error_page", "client_max_body_size", "min_delete_depth", "pid", "alias", "autoindex", "return", "try_files", "" };

ConfigParser::ConfigParser(): err_msg("Unexpected error!\n"), inBlock(0), expected(DEFAULT)
{
	// blocks.push(webserv.getProtocol());
}

bool ConfigParser::validFilename(str &fn) const
{
	std::size_t	n;

	n = fn.length();
	if (n < 4 || fn.find(".conf") == str::npos)
		return false;
	return fn.find(".conf") == fn.length() - 5;
}

bool ConfigParser::isWhitespace(char c) const
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool ConfigParser::isControl(char c) const
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
	{
		err_msg = "Syntax error: Unexpected token `";
		err_msg.append(next.substr(0, 1));
		err_msg.append("`\n");
		return false;
	}
	return true;
}

bool ConfigParser::isValidDirective(str &word)
{
	for (int i = 0; directives[i] != ""; i++)
		if (directives[i] == word)
			return true;
	err_msg = "Invalid Configuration: Unrecognized directive `";
	err_msg.append(word);
	err_msg.append("`\n");
	return false;
}

str		ConfigParser::toString(int x) const
{
	std::stringstream s;
	s << x;
	return s.str();
}

void ConfigParser::skipWhitespace(str &line) const
{
	unsigned int	i;

	for (i = 0; i < line.length() && isWhitespace(line[i]); i++);
	line = line.substr(i);
}

str ConfigParser::parseNext(str &line) const
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
	BlockOBJ	*ptr;

	inBlock += (word == "{") - (word == "}");
	if (!isControl(word[0]))
	{
		parsed_opts.push(word);
		if (expected == DEFAULT)
		{
			expected = WORD_SEMI;
			if (word == "http" || word == "server" || word == "location")
				expected = OPEN_BRAC;
			else if (!isValidDirective(word))
				return false;
		}
	}
	else
	{
		expected = DEFAULT;
		if (parsed_opts.size() > 0)
		{
			if (parsed_opts.front() == "http")
			{
				if (parsed_opts.size() > 1)
				{
					err_msg = "Incorrect arguments for block directive!\n";
					return false;
				}
				blocks.push(webserv.getProtocol());
			}
			else
			{
				if (blocks.size() == 0 && word[0] == '{')
				{
					err_msg = "Block in the wrong scope!\n";
					return false;
				}
				if (word[0] == '{')
				{
					ptr = blocks.top()->handleBlock(parsed_opts);
					if (!ptr)
					{
						err_msg = "Incorrect arguments for block directive!\n";
						return false;
					}
					blocks.push(ptr);
				}
				else if (word[0] == ';' && blocks.size() > 0)
					blocks.top()->handleDirective(parsed_opts);
			}
			while (parsed_opts.size() > 0)
				parsed_opts.pop();
		}
		if (word == "}")
			blocks.pop();
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
			throw InvalidFileError(err_msg + "Line (" + toString(i) + "): " + line);
	}
	if (inBlock)
		throw InvalidFileError("Syntax error: Reached EOF before end of block.");
	if (expected != DEFAULT)
		throw InvalidFileError("Syntax error: Reached EOF before end of directive.");
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
