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
    this->cgi_fd = -1;
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
    this->cgi_fd = copy.cgi_fd;
    return (*this);
}

void    Cgi::setupEnvAndRun(Request* req, std::stringstream& resp, Server* serv, 
        std::vector<struct pollfd>& pollfds, std::set<int>& cgiFds)
{
    const str uri = req->getFileURI();

    this->path_info = "PATH_INFO=" + str(serv->getRoot() + req->getFileURI());
    this->path_info = path_info.substr(0, path_info.find_first_of('?'));
    this->query_string = "QUERY_STRING=" + uri.substr(uri.find_first_of('?') + 1, str::npos);
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
    this->env.push_back("SCRIPT_NAME=" + this->scriptName);

    runCGI(resp, serv, req, pollfds, cgiFds);
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

bool    Cgi::validScriptAccess() const
{
    std::cout << CYAN << "Script path: " << this->cgiPath << NL;
    std::cout << YELLOW << "Script name: " << this->scriptName << NL;
    if (access(this->cgiPath.c_str(), F_OK) == 0)    // the file is found
    {
        std::cout << "Script file is found\n";
        if (access(this->cgiPath.c_str(), X_OK | R_OK) == 0)// check if the file has execution rights
        {
            std::cout << "Script file is readable and executable\n";
            return (true);
        }
    }
    std::cerr << RED"Script file is inaccessible" << NL;
    return (false);
}

// void    Cgi::runCGI(std::stringstream& resp, Server* server, Request* req)
// {
//     // if the script is inaccessible, return an error page
//     if (!validScriptAccess()) // if there is no set error page for error code (unimplemented), send default page
//     {
//         resp << "HTTP/1.1 404 Not Found\r\n";
//         resp << "Content-Length: 83\r\n";
//         resp << "\r\n\r\n";
//         resp << "<html><center><h1>404 Not Found</h1></center><hr><center>JesterServ</center></html>";
//     }
//     // if request is POST, open .tmp file and dup it with stdin
//     if (method == "POST")
//     {
//         std::cout << "Opening " << req->getTempFileName() << " to pass to CGI script\n";
//         int in_fd = open(req->getTempFileName().c_str(), O_RDONLY);
//         if (in_fd == -1)
//         {
//             // handle error
//             throw (std::exception());
//         }
//         dup2(in_fd, STDIN_FILENO);
//         close(in_fd);
//     }
//     if (pipe(this->pipe_fds) == -1)
//         throw (std::runtime_error("Couldn't open pipes for CGI"));
//     this->cgi_fd = fork();
//     if (cgi_fd == 0)    // child process
//     {
//         str script_path = this->path_info.substr(this->path_info.find("PATH_INFO=") + std::strlen("PATH_INFO="), str::npos);
//         const char* cmd = "/usr/bin/php";
//         const char *const argv[3] = {cmd, script_path.c_str(), NULL};
//         char* const* envp = envToChar();
//         close(pipe_fds[READ]);
//         dup2(pipe_fds[WRITE], STDOUT_FILENO);
//         close(pipe_fds[WRITE]);
//         if (execve(cmd, const_cast<char **>(argv), envp))
//         {
//             perror("Why");
//             delete (envp);
//             std::cerr << "Can't execute CGI script\n";
//             exit(EXIT_FAILURE);
//         }
//     }
//     else
//     {
//         close(pipe_fds[WRITE]);
//         char b[BUFFER_SIZE] = {0};
//         ssize_t r = 1;
//         int response = open("./.cgi-response", O_RDWR | O_CREAT, 0644);
//         fcntl(response, F_SETFL, fcntl(response, F_GETFL) | O_NONBLOCK);
//         fcntl(response, F_SETFD, fcntl(response, F_GETFD) | FD_CLOEXEC);
//         // while (r)
//         // {
//         //     r = read(pipe_fds[READ], b, BUFFER_SIZE);
//         //     if (write(response, b, r) == -1)
//         //         std::cerr << RED"Write error" << NL;
//         //         // error;
//         // }
//         // close(response);
//         std::ifstream test_stream;
//         test_stream.open("./.cgi-response", std::ios::in | std::ios::binary);
//         int response_fd = open("./.cgi-response", O_RDONLY);
//         if (response_fd == -1)
//             throw (std::runtime_error("Can't open CGI response FD"));
//         server->setFileFD(response_fd);
//         str line;
//         while (std::getline(test_stream, line))
//         {
//             if (line.find("HTTP/1.1") != str::npos)
//             {
//                 std::cout << RED << line << NL;
//                 server->setHeader(line);
//             }
//             else
//             {
//                 std::cout << RED << line << NL;
//                 server->setBody(line);
//             }
//         }
//         (void)r;
//         (void)b;
//     }
//     (void)resp;
// }

/// @brief runs the cgi script and adds the READend of pipe to the poll() call
/// @param resp the respose string stream
/// @param server the server object that is handling the CGI request
/// @param req the request object that requested for the CGI script
/// @param pollfds the vector of pollfds that are being poll()'ed
void    Cgi::runCGI(std::stringstream& resp, Server* server, Request* req, std::vector<struct pollfd>& pollfds, std::set<int>& cgiFds)
{
    // if the script is inaccessible, return an error page
    if (!validScriptAccess()) // if there is no set error page for error code (unimplemented), send default page
    {
        resp << "HTTP/1.1 404 Not Found\r\n";
        resp << "Content-Length: 83\r\n";
        resp << "\r\n\r\n";
        resp << "<html><center><h1>404 Not Found</h1></center><hr><center>JesterServ</center></html>";
    }
    // if request is POST, open .tmp file and dup it with stdin
    if (pipe(this->pipe_fds) == -1)
        throw (std::runtime_error("Couldn't open pipes for CGI"));
    if (method == "POST")
    {
        std::cout << "Opening " << req->getTempFileName() << " to pass to CGI script\n";
        int in_fd = open(req->getTempFileName().c_str(), O_RDONLY);
        if (in_fd == -1)
        {
            // handle error
            throw (std::exception());
        }
        dup2(in_fd, pipe_fds[READ]);
        close(in_fd);
    }
    this->cgi_fd = fork();
    if (cgi_fd == CHLDPROC)    // child process
    {
        const str script_path = this->path_info.substr(this->path_info.find("PATH_INFO=") + std::strlen("PATH_INFO="), str::npos);
        const char* cmd = "/usr/bin/php";
        const char *const argv[3] = {cmd, script_path.c_str(), NULL};
        char* const* envp = envToChar();
        // if post method, dup READend of pipe to STDIN
        if (method != "POST")
            dup2(pipe_fds[READ], STDIN_FILENO);
        close(pipe_fds[READ]);
        dup2(pipe_fds[WRITE], STDOUT_FILENO);
        close(pipe_fds[WRITE]);
        if (execve(cmd, const_cast<char **>(argv), envp))
        {
            perror("Why");
            delete (envp);
            std::cerr << "Can't execute CGI script\n";
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        close(pipe_fds[WRITE]);
        fcntl(pipe_fds[READ], F_SETFL, O_NONBLOCK); // set the read end of the pipe to non blocking
        struct pollfd read_fd;
        read_fd.events = POLLIN | POLLOUT;
        read_fd.revents = 0;
        read_fd.fd = pipe_fds[READ];
        pollfds.push_back(read_fd); // add the read end of the pipe to the pollfds
        cgiFds.insert(read_fd.fd);
        std::cout << "Pushed " << read_fd.fd << " into set of fds\n";
    }
    (void)server;
}
