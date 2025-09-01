#include "Cgi.hpp"
#include "ConnectionManager.hpp"
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
	std::memset(&this->write_fd, 0, sizeof(struct pollfd));
	std::memset(&this->read_fd, 0, sizeof(struct pollfd));
}

Cgi::~Cgi()
{
	if (this->stdin_fds[0] != -1)
	{
		close(this->stdin_fds[0]);
		this->stdin_fds[0] = -1;
	}
	if (this->stdin_fds[1] != -1)
	{
		close(this->stdin_fds[1]);
		this->stdin_fds[1] = -1;
	}
	if (this->pipe_fds[0] != -1)
	{
		close(this->pipe_fds[0]);
		this->pipe_fds[0] = -1;
	}
	if (this->pipe_fds[1] != -1)
	{
		close(this->pipe_fds[1]);
		this->pipe_fds[1] = -1;
	}
}

Cgi::Cgi(const Cgi& copy)
{
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
	std::memset(&this->write_fd, 0, sizeof(struct pollfd));
	std::memset(&this->read_fd, 0, sizeof(struct pollfd));
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
	this->write_fd = copy.write_fd;
	this->read_fd = copy.read_fd;
	return (*this);
}

bool	Cgi::writeToFd(int fd, const char *buf, size_t r, Request* req)
{
	ssize_t	written = write(fd, buf, r);
	this->written_bytes += written;
	if (this->written_bytes == (ssize_t)req->getContentLen()) // all bytes written: close write pipe
	{
		if (this->stdin_fds[WRITE] != -1)
		{
			close(this->stdin_fds[WRITE]);
			this->stdin_fds[WRITE] = -1;
		}
		return (true);
	}
	return (false);
}

str	Cgi::setupEnvAndRun(unsigned int& i, int& client_fd, Request* req, ConnectionManager& cm, Server* serv)
{
	const str& uri = req->getFileURI();
	std::ostringstream  length;

	this->path_info = "PATH_INFO=" + str(serv->getRoot() + uri);
	this->path_info = path_info.substr(0, path_info.find_first_of('?'));
	this->query_string = "QUERY_STRING=" + uri.substr(uri.find_first_of('?') + 1, str::npos);
	this->method = "REQUEST_METHOD=" + req->getMethod();
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

	return (runCGI(i, client_fd, serv, req, cm));
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
	if (access(this->cgiPath.c_str(), F_OK) == 0)    // the file is found
	{
		if (access(this->cgiPath.c_str(), R_OK | X_OK) == 0)// check if the file has execution rights
			return ("OK");
		return ("500");
	}
	return ("404");
}

void	Cgi::dupAndClose(int fd1, int fd2)
{
	dup2(fd1, fd2);
	close(fd1);
}

void	Cgi::setAndAddPollFd(unsigned int i, int fd, ConnectionManager& cm, int events)
{
	struct pollfd	pfd;

	pfd.fd = fd;
	fcntl(pfd.fd, F_SETFD, FD_CLOEXEC);
	fcntl(pfd.fd, F_SETFL, O_NONBLOCK);
	pfd.events = events;
	pfd.revents = 0;
	cm.addFdtoPoll(i, pfd);
}

std::string	Cgi::runCGI(unsigned int& i, int& client_fd, Server* server, Request* req, ConnectionManager& cm)
{
	std::string access_status = validScriptAccess();
	std::map<int, CGIinfo>& cgiProcesses = cm.getCgiProcesses();

	if (access_status != "OK") // if there is no set error page for error code (unimplemented), send default page
		return (access_status);
	if (pipe(this->pipe_fds) == -1)	// if pipe fails, internal server error
		return ("500");
	if (pipe(this->stdin_fds) == -1)
		return ("500");
	if (req->getMethod() == "POST")
	{
		setAndAddPollFd(i, this->stdin_fds[WRITE], cm, POLLIN);	// add the write end of the pipe to the pollfds
		const char*	leftovers = req->getLeftOvers();
		if (leftovers != NULL)
		{
			if (writeToFd(this->stdin_fds[WRITE], leftovers, req->getLeftOverSize(), req) == true)
			{
				if (req->shouldKeepAlive() == false)
					cm.closeSocketNoRef(i);
			}	// save the pollfd in the pollfds and search for it later in the vector to remove
			req->deleteLeftOvers(); // delete req->getLeftOvers();	// delete the leftovers and set it to NULL;
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
		{
			close(this->stdin_fds[WRITE]);
			this->stdin_fds[WRITE] = -1;
		}
		close(this->pipe_fds[READ]);
		this->pipe_fds[READ] = -1;
		dupAndClose(this->stdin_fds[READ], STDIN_FILENO);
		this->stdin_fds[READ] = -1;
		dupAndClose(this->pipe_fds[WRITE], STDOUT_FILENO);
		this->pipe_fds[WRITE] = -1;

		execve(cmd, const_cast<char **>(argv), envp);
		std::cerr << "Unable to execute cmd\n";
		perror("CGI");
		delete [] (envp);
		std::cerr << "Can't execute CGI script\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		if (this->stdin_fds[READ] != -1)
		{
			close(this->stdin_fds[READ]);
			this->stdin_fds[READ] = -1;
		}
		if (this->pipe_fds[WRITE] != -1)
		{
			close(pipe_fds[WRITE]);
			this->pipe_fds[WRITE] = -1;
		}
		setAndAddPollFd(i, pipe_fds[READ], cm, POLLIN);
		cgiProcesses[pipe_fds[READ]] = CGIinfo(client_fd, this->cgi_fd);	// the read end of the pipe is the key
	}
	if (req->getMethod() == "DELETE")
		return ("204");
	return ("200");
	(void)server;
}

int*		Cgi::get_stdin()
{
	return (this->stdin_fds);
}
