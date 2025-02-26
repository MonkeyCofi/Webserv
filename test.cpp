#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <map>
using namespace std;

bool	isadigit(char c)
{
	return c >= '0' && c <= '9';
}

int main(void)
{
	string test = "013214 1234";
	if (all_of(test.begin(), test.end(), isadigit))
		cout << "YES\n";
	else
		cout << "NO\n";
	int x= 5;
	int y = 2;
	int z = 3;
	vector <map<string, int *> > servers_per_ippp;
	servers_per_ippp.push_back(map<string, int *>());
	servers_per_ippp.push_back(map<string, int *>());
	servers_per_ippp.at(0)["x"] = &x;
	servers_per_ippp.at(0)["y"] = &y;
	servers_per_ippp.at(1)["z"] = &z;
	if (servers_per_ippp.at(0).find("x") != servers_per_ippp.at(0).end())
		std::cout << "YES\n";
	else
		std::cout << "NO\n";
	if (servers_per_ippp.at(0).find("y") != servers_per_ippp.at(0).end())
		std::cout << "YES\n";
	else
		std::cout << "NO\n";
	if (servers_per_ippp.at(0).find("z") != servers_per_ippp.at(0).end())
		std::cout << "YES\n";
	else
		std::cout << "NO\n";
	return (0);
}
