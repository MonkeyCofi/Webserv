#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <map>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <unistd.h>
using namespace std;


int main(void)
{
	unsigned int total_length = 0, bytes = 0;
	unsigned char buffer[4096];
	vector<unsigned char> file;
	string			str;
	stringstream	resp;
	int fd = open("favicon.ico", O_RDONLY);
	resp << "HTTP/1.1 200 OK\r\nContent-Type: " << "text/html" << "\r\n";
	total_length = 0;
	while ((bytes = read(fd, buffer, 4096)) > 0)
	{
		total_length += bytes;
		file.insert(file.end(), buffer, buffer + bytes);
	}
	close(fd);
	resp << "Content-Length: " << total_length << "\r\n";
	std::cout << resp.str();
	write(1, &file[0], total_length);
	// string test = "013214 1234";
	// if (all_of(test.begin(), test.end(), isadigit))
	// 	cout << "YES\n";
	// else
	// 	cout << "NO\n";
	// int x= 5;
	// int y = 2;
	// int z = 3;
	// vector <map<string, int *> > servers_per_ippp;
	// servers_per_ippp.push_back(map<string, int *>());
	// servers_per_ippp.push_back(map<string, int *>());
	// servers_per_ippp.at(0)["x"] = &x;
	// servers_per_ippp.at(0)["y"] = &y;
	// servers_per_ippp.at(1)["z"] = &z;
	// if (servers_per_ippp.at(0).find("x") != servers_per_ippp.at(0).end())
	// 	std::cout << "YES\n";
	// else
	// 	std::cout << "NO\n";
	// if (servers_per_ippp.at(0).find("y") != servers_per_ippp.at(0).end())
	// 	std::cout << "YES\n";
	// else
	// 	std::cout << "NO\n";
	// if (servers_per_ippp.at(0).find("z") != servers_per_ippp.at(0).end())
	// 	std::cout << "YES\n";
	// else
	// 	std::cout << "NO\n";
	return (0);
}
