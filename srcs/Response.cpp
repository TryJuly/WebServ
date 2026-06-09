#include "../header/Response.hpp"

Response::Response() {}

Response::Response(const Response& other) {
    if (this != &other)
        *this = other;
}

Response& Response::operator=(const Response& other) {
    if (this != &other) {
       _status = other._status;
       _headers = other._headers;
       _body = other._body;
    }
    return (*this);
}

Response::~Response() {}

Response::Response( Request& req, ConfigServer& config) {
    //POST method
    if (req.getMethod() == "POST") {
        postResponse(req, config);
        return;
    }
    //DELETE method
    else if (req.getMethod() == "DELETE") {
        deleteResponse(req, config);
        return;
    }
    //GET method
    else if (req.getMethod() == "GET") {
        getResponse(req, config);
        return;
    }
    //OTHERS
    else {}
}

void Response::getResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();
    // std::cout << path << std::endl;

    int index_location;
    if (path != "/") {
        int index = path.find_first_of('/', 1);
        std::string loc_path = path.substr(0, index);
        std::cout << loc_path << std::endl;
        index_location = config.FindLocationPath(loc_path);
    }
    else {
        std::string l_path = config.GetRoot() + path;
        index_location = config.FindLocationPath(l_path);
    }

    if (index_location < 0) {
        std::cout << "haha" << std::endl;
        sendError(404, config);
        return;
    }
    else if (config.GetConfigLocation(index_location).GetBoolGet() != 1) {
        //Error 405 method not allowed
        return ;
    }

    if (path == "/") {
        std::string f_path = config.GetRoot() + "/" + config.GetIndex();
        std::cout << f_path << std::endl;
        _status = "HTTP/1.1 200 OK\r\n";
        std::pair<std::string, std::string> type("Content-Type:", "text/html");
        struct stat sb;
        stat(f_path.c_str(), &sb);
        std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
        _headers.insert(type);
        _headers.insert(length);
        _body = extract_file(f_path);
    }
    else {
        std::string f_path = config.GetRoot() + path;
        std::cout << f_path << std::endl;
        struct stat sb;
        if (stat(f_path.c_str(), &sb) != 0) {
            sendError(404, config);
            return;
        }
        _status = "HTTP/1.1 200 OK\r\n";
        std::pair<std::string, std::string> type("Content-Type:", "text/html");
        std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
        _headers.insert(type);
        _headers.insert(length);
        _body = extract_file(f_path.c_str());
    }
}

void Response::postResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();
    int index = path.find_first_of('/', 1);
    std::string loc_path = path.substr(0, index);
    std::cout << loc_path << std::endl;
    int index_location = config.FindLocationPath(loc_path);
    if (config.GetConfigLocation(index_location).GetBoolPost() != 1) {
        //Error 405 method not allowed
        return ;
    }
    // get upload path form config location
    std::ofstream ofs(path.c_str());
    _body = req.getBody();
    ofs << _body;
    _status = "HTTP/1.1 200 OK\r\n";
}

void Response::deleteResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();
    std::cout << path << std::endl;
    int index_location = config.FindLocationPath(path);
    if (config.GetConfigLocation(index_location).GetBoolDelete() != 1) {
        //Error 405 method not allowed
        return ;
    }
    //check if path/file is removable or not --> config
    if (remove(path.c_str())) {
        std::cout << "could not delete" << std::endl;
    }
    _status = "HTTP/1.1 200 OK\r\n";
}

std::string Response::getStatus(void) const {
    return (_status);
}

void Response::setStatus(std::string status) {
    std::cout << status << std::endl;
    return;
}

std::string Response::printResponse(void) {
    std::string response;
    response.append(_status);
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++) {
        response.append(it->first + it->second + "\r\n");
    }
    response.append("\r\n");
    response.append(_body);
    return (response);
}

void Response::sendError(int status, ConfigServer& config) {
    std::string err_file;
    switch (status) {
        
        case 404:
            err_file = config.GetErrorPages(status);
            std::cout << err_file << std::endl;
            _status = "HTTP/1.1 404 Not Found\r\n";
            break ;
        
        default:
            std::cout << "Error" << std::endl;
            break;
    }
    struct stat sb;
    stat(err_file.c_str(), &sb);
    std::pair<std::string, std::string> type("Content-Type:", "text/html");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
    _body = extract_file(err_file);
}

std::string extract_file(std::string filename) {
    std::ifstream ifs;
    ifs.open(filename.c_str());
    if (!(ifs.is_open())) {
        std::cout << "Not good file extraction" << std::endl;
    }
    std::string body = "";
    std::string line;
    while (std::getline(ifs, line)) {
        body.append(line);
        body.append("\n");					
    }
    return (body);
}

std::string return_file_length(size_t length) {
    std::ostringstream len;
    len << length;
    return (len.str());
}