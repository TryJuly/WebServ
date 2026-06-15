#include "Request.hpp"

Request::Request() {
	_isCGI = false;
}

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
		_isCGI = other._isCGI;
    }
    return (*this);
}

Request::~Request() {}

Request::Request(char *buff) {
	_isCGI = false;

    std::string req = static_cast<std::string>(buff);
    std::string first_line = req.substr(0, req.find_first_of('\n'));
    //std::cout << req << std::endl << std::endl;
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
    _path = first_line.substr(first_line.find_first_of('/'), first_line.find_last_of(' ') - (first_line.find_first_of('/')));

    //headers
    fillHeaders(req);

	//	CGI Check
	size_t	pos = _path.find_first_of(".");
	if (pos != std::string::npos) {
		std::string extension = _path.substr(pos,_path.size());
		if (extension == ".py") {
			_isCGI = true;
			return ;
		}
	}

    //body
    size_t body_pos = req.find("\r\n\r\n");
    if (body_pos == std::string::npos) {
        body_pos = req.find("\n\n");
        body_pos += 2;
    }
    else
        body_pos += 4;
    _body = req.substr(body_pos);

}

void Request::fillHeaders(std::string req) {
    size_t pos = req.find("\n") + 1;
    while (pos < req.size()) {
        size_t end = req.find("\n", pos);
        if (end != std::string::npos && req[end - 1] == '\r')
            end--;
        if (end == std::string::npos || end ==  pos)
            break;
        std::string line = req.substr(pos, end - pos);
        size_t point = line.find(":");
        if (point != std::string::npos) {
            std::string key = line.substr(0, point);
            std::string value = line.substr(point + 2);
            _headers.insert(std::make_pair(key, value));
            std::cout << key << " : " << value << std::endl;
        }
        pos = end + 2;
    }
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

bool	Request::IsCGI( void )
{ return (_isCGI); }