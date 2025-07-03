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

bool    CGIinfo::charEq(const char& c1, const char& c2)
{
    return (c1 == c2);
}

// returns iterator of search_val in to_search case-insensitively
str::iterator    CGIinfo::nameFound(str to_search, str search_val)
{
    return (std::search(to_search.begin(), to_search.end(), search_val.begin(), search_val.end(), CGIinfo::charEq));
}

str CGIinfo::getValue(str main_str, str key, str::iterator& key_start)
{
    size_t  value_start_pos = std::distance(main_str.begin(), key_start) + std::strlen(key.c_str());
    size_t  value_end_pos = main_str.find("\r\n", value_start_pos);
    return (main_str.substr(value_start_pos, value_end_pos - value_start_pos));
}

Response    CGIinfo::parseCgiResponse()
{
    Response    r;
    str         header;

    str::iterator   key_pos_iter = nameFound(this->response_str, "content-type");
    if (key_pos_iter != this->response_str.end())
        r.setHeaderField("Content-type", getValue(this->response_str, "content-type", key_pos_iter));
    key_pos_iter = nameFound(this->response_str, "content-length");
    if (key_pos_iter != this->response_str.end())
        r.setHeaderField("Content-Length", getValue(this->response_str, "content-length", key_pos_iter));
    // the position after \r\n\r\n
    // std::string::iterator pos = std::find(this->response_str.begin(), this->response_str.end(), str("\r\n\r\n"));
    size_t  endPos = this->response_str.find("\r\n\r\n");
    // write the body to the header
    r.setHeaderField("", this->response_str.substr(endPos));
    // r.setHeaderField("", this->response_str.substr(std::distance(this->response_str.begin(), pos)));
    return (r);
}
