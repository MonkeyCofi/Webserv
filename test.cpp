#include <iostream>
#include <algorithm>
#include <unistd.h>

bool test(char c1, char c2)
{
	return (tolower(c1) == tolower(c2));
}

int main(int ac, char **av, char **envp)
{
	// execve("/bin/bash", av, envp);
	std::string test_str = "HTTPS/1.1 200 OK\r\ncontent-type: text/plain\r\ncontent-length: 30";
	std::string s = "CONTENT-TYPE";
	std::string::iterator it = std::search(test_str.begin(), test_str.end(), s.begin(), s.end(), test);
	if (it != test_str.end())
	{
		size_t start_pos = std::distance(test_str.begin(), it) + std::strlen("content-type:");
		size_t	end_pos = test_str.find("\r\n", start_pos);
		std::string value = test_str.substr(start_pos, end_pos - (start_pos));
		std::cout << "str:" << value << "\n";
	}
	else
		std::cout << "no instance of " << s << " in test string\n";
}
