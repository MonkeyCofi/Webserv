#include "Location.hpp"

const str	Location::directives[] = { "root", "index", "client_max_body_size", "min_delete_depth", "autoindex", "return", "redir", "save_folder", "" };

Location::Location(): BlockOBJ()
{
	perm_redir = false;
	auto_index = false;
	root = "";
	redir_uri = "";
	save_folder = "";
}

Location::Location(const Location &copy): BlockOBJ(copy), root(copy.getRoot())
{
	perm_redir = copy.perm_redir;
	auto_index = copy.auto_index;
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
	else
		return parent_ret;
	return true;
}

// BlockOBJ *Location::handleBlock(std::queue<str> opts)
// {
// 	if (opts.size() < 2 || opts.front() != "location")
// 		return NULL;
// 	locations.push_back(new Location());
// 	return locations.back();
// }

str Location::getRoot() const
{
	return root;
}

bool Location::getPermRedir() const {
	return perm_redir;
}

bool Location::getAutoIndex() const {
	return auto_index;
}

const str& Location::getRedirUri() const {
	return redir_uri;
}

const str& Location::getSaveFolder() const {
	return save_folder;
}

bool Location::isAllowedMethod(str method) const
{
	return (std::find(this->allowed_methods.begin(), this->allowed_methods.end(), method) != this->allowed_methods.end());
}

const std::vector<str>& Location::getIndexFiles() const {
	return indexfiles;
}

bool Location::matchURI(str uri) const
{
	return (this->root.find(uri) == 0);
}

const Location &Location::operator =(const Location &copy)
{
	(void)copy;
	return *this;
}

BlockOBJ	*handleBlock(std::queue<str> opts)
{
	(void)opts;
	return NULL;
}

str			getType()
{
	return "location";
}

void Location::setPermRedir(bool value) {
	perm_redir = value;
}

void Location::setAutoIndex(bool value) {
	auto_index = value;
}

void Location::setRoot(const str& value) {
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
