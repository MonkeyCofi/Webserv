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

void    Cgi::setupEnvAndRun(Request* req, Server* serv)
{
    std::cout << "Root for server: " << serv->getRoot() << "\n";
    std::cout << CYAN << "Setting up cgi environment\n";
    this->path_info = serv->getRoot() + req->getFileURI();
    this->path_info = this->path_info.substr(0, this->path_info.find_first_of('?'));
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
    runCGI();
}

char**   Cgi::envToChar()
{
    std::vector<str>    env;
    std::vector<char*>  envp;
    
    env.push_back("PATH_INFO=" + this->path_info);
    env.push_back("METHOD=" + this->method);
    env.push_back("CONTENT_TYPE=" + this->content_type);
    env.push_back("CONTENT_LENGTH=" + this->content_length);
    env.push_back("HOST=" + this->host);
    for (std::vector<str>::iterator it = env.begin(); it != env.end(); it++)
    {
        envp.push_back(const_cast<char*>((*it).c_str()));
    }
    envp.push_back(NULL);
    for (std::vector<char*>::iterator it = envp.begin(); it != envp.end(); it++)
    {
        std::cout << RED << (*it) << NL;
    }
    return (&envp[0]);
}

void    Cgi::runCGI()
{
    int status;
    int old_out = dup(STDOUT_FILENO);
    if (pipe(this->pipe_fds) == -1)
        throw (std::runtime_error("Couldn't open pipes for CGI"));
    this->cgi_fd = fork();
    // char** e = envToChar();
    if (cgi_fd == 0)    // child process
    {
        // dup2(STDOUT_FILENO, pipe_fds[1]);
        // str script_path = this->path_info.substr(path_info.at(0) == '/', path_info.find_last_of("/"));
        str script_path = this->path_info;
        const char *const argv[2] = {script_path.c_str(), NULL};
        std::cout << "Trying to execute " << script_path.c_str() << "\n";
        char* const* envp = envToChar();
        for (int i = 0; envp[i]; i++)
            std::cout << YELLOW << envp[i] << NL;
        std::cout << "In child process\n";
        if (execve("/bin/python3", const_cast<char **>(argv), envp))
        {
            perror("Why");
            throw (std::runtime_error("Cant't execute CGI script")); // execute the cgi script, passing the script path as the argument
        }
    }
    else
    {
        wait(&status);
        dup2(pipe_fds[0], STDOUT_FILENO);
        std::cout.flush();
        dup2(old_out, STDOUT_FILENO);
        // for (int i = 0; e[i]; i++)
        // {
        //     std::cout << e[i] << "\n";
        // }
        // std::cout << "In parent process\n";
    }
}
