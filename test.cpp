#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

bool	isadigit(char c)
{
	return c >= '0' && c <= '9';
}

int main(void)
{
	string test;
	cin>>test;
	if (all_of(test.begin(), test.end(), isadigit))
		cout << "YES\n";
	else
		cout << "NO\n";
	return (0);
}
