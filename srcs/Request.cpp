# include "../header/Request.hpp"
#include <cstddef>

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

std::string Request::getCookie( void )
{
    std::map<std::string, std::string>::iterator it = _headers.find("Cookie");
    if (it == _headers.end())
        return ("");
    size_t  pos = it->second.find("=");
    if (pos == std::string::npos)
        return ("");
    std::string value = it->second.substr(pos + 1);
    return (value);
}

Request::~Request() {}

void    Request::checkMethod(std::string& first_line) {
    _method = first_line.substr(0, first_line.find(' '));
    std::string list[6] = {"OPTIONS", "HEAD", "PUT", "CONNECT", "TRACE", "PATCH"};
    std::vector<std::string> methods(list, list + 6);
    for (unsigned int i = 0; i < methods.size(); i++) {
        if (_method == methods[i])
            throw std::runtime_error("405 Error: Method not implemented");
    }
    throw std::runtime_error("400 Error: Invalid Method");
}

Request::Request(std::string buff) {
	_isCGI = false;

    std::string req = static_cast<std::string>(buff);
    if (req == "\r\n\r\n")
        throw std::runtime_error("400 Error: Empty Request");

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
    else
        checkMethod(first_line);
    //path
    if (first_line.find_first_of('/') != std::string::npos)
        _path = first_line.substr(first_line.find_first_of('/'), first_line.find_last_of(' ') - (first_line.find_first_of('/')));
    else
        throw std::runtime_error("400 Error: No Path");
    //std::cout << _path << " <-- just here" << std::endl;

    //headers
    fillHeaders(req);
    if (_headers.find("Host") == _headers.end()) {
        throw (std::runtime_error("400 Error: No Host"));
    }
    else if (_headers.find("Host")->second == "")
        throw (std::runtime_error("400 Error: No Host"));

	//	CGI Check
    size_t  pos = _path.find_last_of('.');
    if (pos != std::string::npos) {
        size_t  queryPos = _path.find('?', pos);
        std::string extension = _path.substr(pos, queryPos - pos);
        if (extension == ".py" || extension == ".php") {
            _isCGI = true;
            return ;
        }
    }
	// size_t	pos = _path.find_first_of(".");
	// if (pos != std::string::npos) {
	// 	std::string extension = _path.substr(pos,_path.size());
	// 	if (extension == ".py") {
	// 		_isCGI = true;
	// 		return ;
	// 	}
	// }

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