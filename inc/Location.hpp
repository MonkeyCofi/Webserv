#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <exception>
#include <string>
#include <queue>
#include "BlockOBJ.hpp"

typedef std::string str;

class Location: public BlockOBJ
{
	private:
		bool				perm_redir, auto_index;
		str					root, redir_uri, save_folder;
		const static str	directives[];
		std::vector<str>	allowed_methods, indexfiles;


	public:
		Location();
		Location(const Location &copy);
		~Location();

		bool	handleDirective(std::queue<str> opts);
		bool	matchURI(str uri) const;
		str		getRoot() const;
		bool getPermRedir() const;
		bool getAutoIndex() const;
		const str& getRedirUri() const;
		const str& getSaveFolder() const;
		bool isAllowedMethod(str method) const;
		const std::vector<str>& getIndexFiles() const;
		BlockOBJ	*handleBlock(std::queue<str> opts);
		str			getType();
		void setPermRedir(bool value);
		void setAutoIndex(bool value);
		void setRoot(const str& value);
		void setRedirUri(const str& value);
		void setSaveFolder(const str& value);
		void setAllowedMethods(const std::vector<str>& methods);
		void setIndexFiles(const std::vector<str>& files);

		const Location &operator =(const Location &copy);
};

#endif
