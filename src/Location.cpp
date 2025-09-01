#include "Location.hpp"

const str	Location::directives[] = { "root", "index", "client_max_body_size", "min_delete_depth", "autoindex", "return", "redir", "save_folder", "allowed_methods", "cgi_pass", "" };

Location::Location(): BlockOBJ()
{
	perm_redir = false;
	auto_index = false;
	cgi = false;
	root_found = false;
	autoindex_found = false;
	root = "";
	redir_uri = "";
	save_folder = "";
	match_uri = "";
}

Location::Location(const Location &copy): BlockOBJ(copy), root(copy.getRoot())
{
	perm_redir = copy.perm_redir;
	auto_index = copy.auto_index;
	cgi = copy.cgi;
	root_found = copy.root_found;
	autoindex_found = copy.autoindex_found;
	root = copy.root;
	redir_uri = copy.redir_uri;
	save_folder = copy.save_folder;
}

Location::~Location()
{
	
}

bool Location::handleDirective(std::queue<str> opts)
{
	bool			parent_ret;

	if (opts.size() == 0 || !inDirectives(opts.front(), directives))
		return false;
	parent_ret = BlockOBJ::handleDirective(opts);
	if (opts.front() == "root" && opts.size() == 2)
	{
		root_found = true;
		opts.pop();
		// check valid uri
		this->root = opts.front();
	}
	else if (opts.front() == "redir" && opts.size() == 3)
	{
		opts.pop();
		this->perm_redir = (opts.front() == "true");
		if (opts.front() != "true" && opts.front() != "false")
			return false;
		opts.pop();
		// check valid uri
		this->redir_uri = opts.front();
	}
	else if (opts.front() == "autoindex" && opts.size() == 2)
	{
		opts.pop();
		autoindex_found = true;
		this->auto_index = (opts.front() == "on");
		if (opts.front() != "on" && opts.front() != "off")
			return false;
	}
	else if (opts.front() == "index" && opts.size() > 1)
	{
		opts.pop();
		while(opts.size() > 0)
		{
			// check valid uri?
			indexfiles.push_back(opts.front());
			opts.pop();
		}
	}
	else if (opts.front() == "allowed_methods" && opts.size() > 1)
	{
		opts.pop();
		while(opts.size() > 0)
		{
			if (opts.front() != "GET" && opts.front() != "POST" && opts.front() != "DELETE")
				return false;
			allowed_methods.push_back(opts.front());
			opts.pop();
		}
	}
	else if (opts.front() == "save_folder" && opts.size() == 2)
	{
		opts.pop();
		// check valid uri
		this->save_folder = opts.front();
	}
	else if (opts.front() == "cgi_pass" && opts.size() == 1)
	{
		opts.pop();
		this->cgi = true;
	}
	else
		return parent_ret;
	return true;
}

bool	Location::getRootFound() const
{
	std::cout << std::boolalpha << "returning " << root_found << "\n";
	return (this->root_found);
}

bool	Location::getAutoindexFound() const
{
	return (this->autoindex_found);
}

str Location::getRoot() const
{
	return root;
}

const std::string&	Location::getMatchUri() const
{
	return (this->match_uri);
}

bool Location::getPermRedir() const
{
	return perm_redir;
}

bool Location::getAutoIndex() const
{
	return auto_index;
}

bool Location::isCGI() const
{
	return cgi;
}

const str& Location::getRedirUri() const
{
	return redir_uri;
}

const str& Location::getSaveFolder() const
{
	return save_folder;
}

bool Location::isAllowedMethod(str method) const
{
	return (std::find(this->allowed_methods.begin(), this->allowed_methods.end(), method) != this->allowed_methods.end());
}

const std::vector<str>& Location::getIndexFiles() const
{
	return indexfiles;
}

bool Location::matchURI(str uri) const
{
	if (uri.find(this->match_uri) != 0)	// the uri doesn't start with the match uri
		return (false);
	if (uri.length() == this->match_uri.length())
		return (true);
	if (this->match_uri.at(this->match_uri.length() - 1) == '/')
		return (true);
		
	return (false);
}

std::vector<str> &Location::getAllowedMethods()
{
	return this->allowed_methods;
}

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}

BlockOBJ	*Location::handleBlock(std::queue<str> opts)
{
	(void)opts;
	std::cout << "\033[31mHANDLE BLOCK\033[0m\n";
	return NULL;
}

str			Location::getType()
{
	return "location";
}

void Location::setPermRedir(bool value) {
	perm_redir = value;
}

void Location::setAutoIndex(bool value) {
	auto_index = value;
}

void Location::setRoot(const str& value)
{
	std::cout << "old root: " << this->root << "\n";
	std::cout << "setting it to: " << value << "\n";
	root = value;
}

void Location::setRedirUri(const str& value) {
	redir_uri = value;
}

void Location::setSaveFolder(const str& value) {
	save_folder = value;
}

void Location::setAllowedMethods(const std::vector<str>& methods) {
	allowed_methods = methods;
}

void Location::setIndexFiles(const std::vector<str>& files) {
	indexfiles = files;
}

void	Location::setMatchUri(const std::string& uri)
{
	this->match_uri = uri;
}

void	Location::printLocation() const
{
	std::cout << "Permanent redirect: " << std::boolalpha << this->perm_redir << "\n";
	std::cout << "autoindex: " << std::boolalpha << auto_index << "\n";
	std::cout << "root: " << root << "\n";
	std::cout << "redirect url: " << redir_uri << "\n";
	std::cout << "save folder: " << save_folder << "\n";
	std::cout << "allowed methods: ";
	for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
	{
		std::cout << *it << " ";
	}
	std::cout << "\n";
	std::cout << "index files: ";
	for (std::vector<std::string>::const_iterator it = indexfiles.begin(); it != indexfiles.end(); it++)
	{
		std::cout << *it << " ";
	}
}
