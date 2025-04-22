#include "BlockOBJ.hpp"

BlockOBJ::BlockOBJ()
{

}

BlockOBJ::BlockOBJ(const BlockOBJ &copy)
{
	(void)copy;
}

bool BlockOBJ::handleDirective(std::queue<str> opts)
{
	if (opts.size() == 0)
		return false;
	if (opts.front() == "root" && opts.size() == 2)
	{
		opts.pop();
		root_dir = opts.front();
	}
	else if (opts.front() == "index" && opts.size() > 1)
	{
		opts.pop();
		while (opts.size() > 0)
		{
			if (opts.front().length() == 0)
				return false;
			index_pages.push_back(opts.front());
			opts.pop();
		}
	}
	else if (opts.front() == "error_page" && opts.size() > 2)
	{
		opts.pop();
		while (opts.size() > 1)
		{
			if (opts.front().length() == 0)
				return false;
			if (opts.size() > 1)
			{
				for (unsigned int i = 0; i < opts.front().length(); i++)
				{
					if (opts.front()[i] < '0' || opts.front()[i] > '9')
						return false;
				}
			}
			error_pages[opts.front()] = opts.back();
			opts.pop();
		}
	}
	else if (opts.front() == "client_max_body_size" && opts.size() == 2)
	{
		opts.pop();
		for (unsigned int i = 0; i < opts.front().length(); i++)
		{
			if (opts.front()[i] < '0' || opts.front()[i] > '9')
				return false;
		}
		client_max_body = atoi(opts.front().c_str());
	}
	else if (opts.front() == "min_delete_depth" && opts.size() == 2)
	{
		opts.pop();
		for (unsigned int i = 0; i < opts.front().length(); i++)
		{
			if (opts.front()[i] < '0' || opts.front()[i] > '9')
				return false;
		}
		min_del_depth = atoi(opts.front().c_str());
	}
	else if (opts.front() == "autoindex" && opts.size() == 2)
	{
		opts.pop();
		if (opts.front() != "on" && opts.front() != "off")
			return false;
		else if (opts.front() == "on")
			autoindex = true;
		else
			autoindex = false;
	}
	else
		return false;
	return true;
}

bool BlockOBJ::inDirectives(const str &dir, const str *dirs) const
{
	for (unsigned int i = 0; dirs[i] != ""; i++)
		if (dir == dirs[i])
			return true;
	return false;
}

const BlockOBJ &BlockOBJ::operator=(const BlockOBJ &copy)
{
	(void)copy;
	return *this;
}

BlockOBJ::~BlockOBJ()
{

}
