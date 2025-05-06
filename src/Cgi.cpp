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
*/

Cgi::Cgi()
{
    this->cgiFileName = "";
    this->path_info = "";
    this->query_string = "";
    this->content_type = "";
    this->content_length = "";
    this->method = "";
    this->host = "";
    this->pipe_fds[0] = -1;
    this->pipe_fds[1] = -1;
    this->cgi_fd = -1;
}

Cgi::~Cgi()
{

}

Cgi::Cgi(const Cgi& copy)
{
    // this->env = copy.env;
    this->operator=(copy);
}

Cgi &Cgi::operator=(const Cgi& copy)
{
    this->cgiFileName = copy.cgiFileName;
    this->env = copy.env;
    this->path_info = copy.path_info;
    this->query_string = copy.query_string;
    this->content_type = copy.content_type;
    this->content_length = copy.content_length;
    this->method = copy.method;
    this->host = copy.host;
    this->pipe_fds[0] = copy.pipe_fds[0];
    this->pipe_fds[1] = copy.pipe_fds[1];
    this->cgi_fd = copy.cgi_fd;
    return (*this);
}

void    Cgi::setupEnv(Request* req)
{
    this->path_info = req->getFileURI();
    // this->query_string = // the query string will be the full path of the cgi script (i believe)
    this->method = req->getMethod();
    this->content_type = req->getContentType();
    this->host = req->getHost();
    this->content_length = req->getContentLen();
    env["PATH_INFO"] = this->path_info;
    env["METHOD"] = this->method;
    env["CONTENT_TYPE"] = this->content_type;
    env["HOST"] = this->host;
    env["CONTENT_LENGTH"] = req->getContentLen();
}

char*   Cgi::envToChar()
{
    char**  c_env = new Char*[env.size() + 1];
    env[env.size()] = 0;
    return (c_env);
}

void    Cgi::runCGI()
{
    if (pipe(this->pipe_fds) == -1)
        throw (std::runtime_error("Couldn't open pipes for CGI"));
    this->cgi_fd = fork();
    char** e = envToChar();
    if (cgi_fd == 0)    // child process
    {
        execve() // execute the cgi script, passing the 
    }
}
