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


// int main(void)
// {
// 	unsigned int total_length = 0, bytes = 0;
// 	unsigned char buffer[4096];
// 	vector<unsigned char> file;
// 	string			str;
// 	stringstream	resp;
// 	int fd = open("favicon.ico", O_RDONLY);
// 	resp << "HTTP/1.1 200 OK\r\nContent-Type: " << "text/html" << "\r\n";
// 	total_length = 0;
// 	while ((bytes = read(fd, buffer, 4096)) > 0)
// 	{
// 		total_length += bytes;
// 		file.insert(file.end(), buffer, buffer + bytes);
// 	}
// 	close(fd);
// 	resp << "Content-Length: " << total_length << "\r\n";
// 	std::cout << resp.str();
// 	write(1, &file[0], total_length);
// 	return (0);
// }

int main(void)
{
	try
	{
		std::vector<int> v;
		std::cout << v.at(3) << "\n";
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}