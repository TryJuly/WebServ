#include "Request.hpp"

Request::Request() {}

Request::Request(const Request& other) {
    if (this != &other) {
        *this = other;
    }
}

Request& Request::operator=(const Request& other) {
    if (this != &other) {
        _method = other._method;
        _headers = other._headers;
        _body = other._body;
    }
    return (*this);
}

Request::~Request() {}

Request::Request(char *buff) {
    std::string req = static_cast<std::string>(buff);
    std::string first_line = req.substr(0, req.find_first_of('\n'));
    std::cout << buff << std::endl;
    //method
    if (first_line.find("GET") != first_line.npos) {
        _method = first_line.substr(0, 3);
    }
    else if (first_line.find("POST") != first_line.npos) {
        _method = first_line.substr(0, 4);
    }
    else if (first_line.find("DELETE") != first_line.npos) {
        _method = first_line.substr(0, 6);
    }
    //path
    _path = first_line.substr(first_line.find_first_of('/') + 1, first_line.find_last_of(' ') - (first_line.find_first_of('/') + 1));
    //body
    size_t body_pos = req.find_last_of("\r\n\r\n");
    _body = req.substr(body_pos);
}

std::string Request::getMethod(void) const {
    return (_method);
}

std::string Request::getPath(void) const {
    return (_path);
}

std::map<std::string, std::string> Request::getHeaders(void) const {
    return (_headers);
}

std::string Request::getBody(void) const {
    return (_body);
}