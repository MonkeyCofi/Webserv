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

// returns iterator of search_val in to_search case-insensitively
size_t    CGIinfo::nameFound(str& to_search, str search_val)
{
    str cpys = to_search;
    for (unsigned int i = 0; i < cpys.length(); i++)
        cpys[i] = std::tolower(cpys[i]);
    for (unsigned int i = 0; i < search_val.length(); i++)
        search_val[i] = std::tolower(search_val[i]);
    return (cpys.find(search_val));
}

str CGIinfo::getValue(str& main_str, str key, size_t key_start)
{
    size_t  value_start_pos = key_start;
    size_t  value_end_pos = main_str.find("\r\n", value_start_pos);
    str value = main_str.substr(main_str.find(":", value_start_pos) + 1, value_end_pos - (main_str.find(":", value_start_pos) + 1));
    // str value = main_str.substr(value_start_pos, value_end_pos - value_start_pos);
    // size_t colonSp_pos = value.find(":");
    // if (colonSp_pos != str::npos)
    //     value.erase(colonSp_pos, 1);
    std::cout << "Key: " << key << " value: " << value << "\n";
    return (value);
}

Response    CGIinfo::parseCgiResponse()
{
    std::cout << "in parse CGI response function\n";
    Response    r;
    str         header;
    size_t   content_type_iter = nameFound(this->response_str, "content-type");
    str             content_type;

    if (content_type_iter != str::npos)
        content_type = getValue(this->response_str, "content-type", content_type_iter);
    size_t  endPos = -1;
    if (this->response_str.find("\r\n\r\n") != str::npos)
    {
        endPos = this->response_str.find("\r\n\r\n") + str("\r\n\r\n").length();
        r.setBody(this->response_str.substr(endPos), content_type);
    }
    else    // if the cgi script doesn't write a body, write the headers
    {
        ;
    }
    (void)content_type_iter;
    (void)content_type;
    return (r);
}
