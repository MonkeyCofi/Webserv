#include "Cgi.hpp"

/*
	* Check if the request URI contains the word cgi or the folder: cgi-bin.
	* if request method is GET, then the query for the GET request is stored in an environment variable called QUERY_STRING
	* if request method is POST, then the request body is passed to the standard input of the script
	* if request contains additional path after the cgi filename, then that path is stored in a variable called PATH_INFO. however,
	in this case, we just use the full path as the PATH_INFO
	* once the environment is setup, create a pipe variable that will represent
	the stdin and stdout of the CGI program
	* write the request body into stdin
	* the stdout will be the response of the cgi
	* by default, request.cgi_fd is instantiated to -1
	* if a script is gonna be executed, set cgi_fd to that child process's fd
	* when in POLLIN event, if cgi_fd for this request is not -1, it means there is a currently executing cgi script
*/

Cgi::Cgi()
{
	this->cgiPath = "";
	this->path_info = "";
	this->query_string = "";
	this->content_type = "";
	this->content_length = "";
	this->method = "";
	this->host = "";
	this->pipe_fds[0] = -1;
	this->pipe_fds[1] = -1;
	this->stdin_fds[0] = -1;
	this->stdin_fds[1] = -1;
	this->cgi_fd = -1;
	this->written_bytes = 0;
}

Cgi::~Cgi()
{

}

Cgi::Cgi(const Cgi& copy)
{
	// this->env = copy.env;
	this->operator=(copy);
}

Cgi::Cgi(const str script_path, Server* server)
{
	this->cgiPath = server->getRoot() + script_path;
	this->cgiPath = this->cgiPath.substr(0, this->cgiPath.find_first_of('?'));
	this->scriptName = this->cgiPath.substr(this->cgiPath.find_last_of('/') + 1);
	this->path_info = "";
	this->query_string = "";
	this->content_type = "";
	this->content_length = "";
	this->method = "";
	this->host = "";
	this->pipe_fds[0] = -1;
	this->pipe_fds[1] = -1;
	this->stdin_fds[0] = -1;
	this->stdin_fds[1] = -1;
	this->cgi_fd = -1;
	this->written_bytes = 0;
}

Cgi &Cgi::operator=(const Cgi& copy)
{
	this->cgiPath = copy.cgiPath;
	this->env = copy.env;
	this->path_info = copy.path_info;
	this->query_string = copy.query_string;
	this->content_type = copy.content_type;
	this->content_length = copy.content_length;
	this->method = copy.method;
	this->host = copy.host;
	this->pipe_fds[0] = copy.pipe_fds[0];
	this->pipe_fds[1] = copy.pipe_fds[1];
	this->stdin_fds[0] = copy.stdin_fds[0];
	this->stdin_fds[1] = copy.stdin_fds[1];
	this->cgi_fd = copy.cgi_fd;
	this->written_bytes = copy.written_bytes;
	return (*this);
}

void	Cgi::writeToFd(int fd, char *buf, size_t r, Request* req)
{
	std::cout << "writing " << buf << " to stdin fd\n";
	ssize_t	written = write(fd, buf, r);
	this->written_bytes += written;
	std::cout << "Wrote " << this->written_bytes << " so far\n";
	if (this->written_bytes == (ssize_t)req->getContentLen()) // all bytes written: close write pipe
	{
		std::cout << "Body has been fully written\n";
		close(this->stdin_fds[WRITE]);
	}
	else if (written > (ssize_t)req->getContentLen())	// technically should never happen because it is checked when receiving the request
		;
}

str	Cgi::setupEnvAndRun(int& client_fd, Request* req, Server* serv, 
		std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses)
{
	const str& uri = req->getFileURI();
	std::ostringstream  length;

	this->path_info = "PATH_INFO=" + str(serv->getRoot() + uri);
	this->path_info = path_info.substr(0, path_info.find_first_of('?'));
	this->query_string = "QUERY_STRING=" + uri.substr(uri.find_first_of('?') + 1, str::npos);
	this->method = "METHOD=" + req->getMethod();
	this->content_type = "CONTENT_TYPE=" + req->getContentType();
	this->host = "HTTP_HOST=" + req->getHost();
	length << req->getContentLen();
	this->content_length = "CONTENT_LENGTH=" + length.str();   // incomplete
	this->env.push_back(path_info);
	this->env.push_back(query_string);
	this->env.push_back(method);
	this->env.push_back(content_type);
	this->env.push_back(host);
	this->env.push_back(content_length);
	this->env.push_back("SCRIPT_NAME=" + this->scriptName);

	return (runCGI(client_fd, serv, req, pollfds, cgiProcesses));
}

char**   Cgi::envToChar()
{
	char**  envp = new char*[env.size() + 1];

	size_t  i = 0;
	for (; i < this->env.size(); i++)
		envp[i] = const_cast<char *>(this->env[i].c_str()); // take constness away from string
	envp[i] = NULL;
	return (envp);
}

std::string	Cgi::validScriptAccess() const
{
	if (access(this->cgiPath.c_str(), F_OK | R_OK) == 0)    // the file is found
	{
		std::cout << "Script file is found\n";
		if (access(this->cgiPath.c_str(), X_OK) == 0)// check if the file has execution rights
		{
			std::cout << "Script file is readable and executable\n";
			return ("OK");
		}
		std::cout << "returning 500\n";
		return ("500");
	}
	return ("404");
}

void	Cgi::dupAndClose(int fd1, int fd2)
{
	dup2(fd1, fd2);
	close(fd1);
}

void	Cgi::setAndAddPollFd(int fd, std::vector<struct pollfd>& pollfds, int events)
{
	struct pollfd	pfd;

	pfd.fd = fd;
	fcntl(pfd.fd, F_SETFD, FD_CLOEXEC);
	fcntl(pfd.fd, F_SETFL, O_NONBLOCK);
	pfd.events = events;
	pfd.revents = 0;
	pollfds.push_back(pfd);
}

std::string	Cgi::runCGI(int& client_fd, Server* server, 
		Request* req, std::vector<struct pollfd>& pollfds, std::map<int, CGIinfo>& cgiProcesses)
{
	std::string access_status = validScriptAccess();
	(void)req;
	if (access_status != "OK") // if there is no set error page for error code (unimplemented), send default page
	{
		// if script is not accessible, respond with error page
		return (access_status);
	}
	if (pipe(this->pipe_fds) == -1)	// if pipe fails, internal server error
	{
		std::cout << "returning 500\n";
		return ("500");
	}
	if (pipe(this->stdin_fds) == -1)
		return ("500");
	if (req->getMethod() == "POST")
	{
		setAndAddPollFd(this->stdin_fds[WRITE], pollfds, POLLIN);
		std::cerr << "opened stdin pipes for CGI POST request\n";
		const char*	leftovers = req->getLeftOvers();
		if (leftovers != NULL)
		{
			writeToFd(this->stdin_fds[WRITE], const_cast<char *>(leftovers), req->getLeftOverSize(), req);
			std::cout << "Wrote the leftovers and will now delete the leftovers\n";
			delete req->getLeftOvers();	// delete the leftovers and set it to NULL;
			req->setLeftOvers(NULL, 0);
		}
	}
	this->cgi_fd = fork();
	if (this->cgi_fd == CHLDPROC)    // child process
	{
		const str script_path = this->path_info.substr(this->path_info.find("PATH_INFO=") + std::strlen("PATH_INFO="), str::npos);
		const char* cmd = "/usr/bin/php";
		const char *const argv[3] = {cmd, script_path.c_str(), NULL};
		char* const* envp = envToChar();

		if (this->stdin_fds[WRITE] != -1)
			close(this->stdin_fds[WRITE]);
		close(this->pipe_fds[READ]);
		dupAndClose(this->stdin_fds[READ], STDIN_FILENO);
		dupAndClose(this->pipe_fds[WRITE], STDOUT_FILENO);

		execve(cmd, const_cast<char **>(argv), envp);
		std::cerr << "Unable to execute cmd\n";
		perror("Why");
		delete [] (envp);
		std::cerr << "Can't execute CGI script\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		if (this->stdin_fds[READ] != -1)
			close(this->stdin_fds[READ]);
		close(pipe_fds[WRITE]);
		setAndAddPollFd(pipe_fds[READ], pollfds, POLLIN);
		cgiProcesses[pipe_fds[READ]] = CGIinfo(client_fd, this->cgi_fd);
	}
	return ("200");
	(void)server;
}

int*		Cgi::get_stdin()
{
	return (this->stdin_fds);
}
