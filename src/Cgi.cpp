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

void    Cgi::setupEnvAndRun(Request* req, std::stringstream& resp, Server* serv)
{
    this->path_info = "PATH_INFO=" + str(serv->getRoot() + req->getFileURI());
    this->path_info = path_info.substr(0, path_info.find_first_of('?'));
    this->query_string = "QUERY_STRING=" + req->getFileURI().substr(req->getFileURI().find_first_of('?'), str::npos);
    this->method = "METHOD=" + req->getMethod();
    this->content_type = "CONTENT_TYPE=" + req->getContentType();
    this->host = "HTTP_HOST=" + req->getHost();
    this->content_length = "CONTENT_LENGTH=";   // incomplete

    this->env.push_back(path_info);
    this->env.push_back(query_string);
    this->env.push_back(method);
    this->env.push_back(content_type);
    this->env.push_back(host);
    this->env.push_back(content_length);
    runCGI(resp);
}

char**   Cgi::envToChar()
{
    char**  envp = new char*[env.size() + 1];
    size_t  i = 0;
    for (; i < this->env.size(); i++)
        envp[i] = const_cast<char *>(this->env[i].c_str());
    envp[i] = NULL;
    return (envp);
}

void    Cgi::runCGI(std::stringstream& resp)
{
    int status;
    int old_out = dup(STDOUT_FILENO);
    if (pipe(this->pipe_fds) == -1)
        throw (std::runtime_error("Couldn't open pipes for CGI"));
    this->cgi_fd = fork();
    if (cgi_fd == 0)    // child process
    {
        str script_path = this->path_info.substr(this->path_info.find("PATH_INFO=") + std::strlen("PATH_INFO="), str::npos);
        const char* cmd = "/usr/bin/python3";
        const char *const argv[3] = {cmd, script_path.c_str(), NULL};
        char* const* envp = envToChar();
        close(pipe_fds[READ]);
        dup2(STDOUT_FILENO, pipe_fds[WRITE]);
        close(pipe_fds[WRITE]);
        if (execve(cmd, const_cast<char **>(argv), envp))
        {
            perror("Why");
            delete (envp);
            throw (std::runtime_error("Cant't execute CGI script")); // execute the cgi script, passing the script path as the argument
        }
    }
    else
    {
        wait(&status);
        if (WIFEXITED(status))
            std::cout << "Child process exited with status: " << WEXITSTATUS(status) << "\n";
        else
            std::cout << "Child process did not exit\n";
        close(pipe_fds[WRITE]);
        dup2(pipe_fds[READ], STDOUT_FILENO);
        close(pipe_fds[READ]);
        resp << std::cout;
        dup2(old_out, STDOUT_FILENO);
        std::cout << CYAN "resp: " << resp.str() << NL;
    }
    (void)resp;
    (void)old_out;
}
