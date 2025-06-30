#include "CGIinfo.hpp"

CGIinfo::CGIinfo()
{
    this->response_complete = false;
    this->response_str.clear();
    this->client_fd = -1;
    this->child_pid = -1;
}

CGIinfo::~CGIinfo()
{
    ;
}

CGIinfo::CGIinfo(int _client, pid_t _child)
{
    this->client_fd = _client;
    this->child_pid = _child;
    this->response_str.clear();
    this->response_complete = false;
}

CGIinfo &CGIinfo::operator=(const CGIinfo& obj)
{
    this->child_pid = obj.child_pid;
    this->client_fd = obj.client_fd;
    this->response_complete = obj.response_complete;
    this->response_str = obj.response_str;
    return (*this);
}

CGIinfo::CGIinfo(const CGIinfo& obj)
{
    this->child_pid = obj.child_pid;
    this->client_fd = obj.client_fd;
    this->response_complete = obj.response_complete;
    this->response_str.clear();
    this->response_str = obj.response_str;
}

void    CGIinfo::concatBuffer(std::string str)
{
    this->response_str.append(str);
}

pid_t   CGIinfo::getPid() const
{
    return (this->child_pid);
}

int CGIinfo::getClientFd() const
{
    return (this->client_fd);
}

void    CGIinfo::completeResponse()
{
    this->response_complete = true;
}

bool    CGIinfo::isComplete() const
{
    return (this->response_complete);
}

std::string CGIinfo::getBuffer() const
{
    return (this->response_str);
}

void    CGIinfo::printInfo() const
{
    std::cout << "Process pid: " << this->child_pid << "\n" << "Client fd: " << this->client_fd << "\n"
        << "Response complete: " << this->response_complete << "\n" << "Response string: " << this->response_str << "\n";
}
