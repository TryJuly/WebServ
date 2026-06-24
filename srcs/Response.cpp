#include "../header/Response.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

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
    else {
        sendError(405, config);
        // Bad request 400 ou Method not allowed 405 ?
    }
}

void Response::sendRedir(std::string path, ConfigServer& config, ConfigLocation& loc) {
    std::string redir = loc.GetRedir();
    _status = "HTTP/1.1 301 Moved Permanently\r\n";
    std::ostringstream port;
    port << config.GetPort();
    size_t filePos = path.find('/', 1);
    std::string file = "";
    if (filePos != std::string::npos)
        file = path.substr(filePos);
    _headers.insert(std::make_pair("Location: ", "http://" + config.GetServerName() + ":" + port.str() + redir + file));
    _headers.insert(std::make_pair("Content-Length:", "0"));
}

void Response::getResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();

    int index_location = loc_index(path, config);

    if (index_location < 0) {
        sendError(404, config);
        return;
    }
    else if (config.GetConfigLocation(index_location).GetRedir() != "") {
        sendRedir(path, config, config.GetConfigLocation(index_location));
        return ;
    }
    else if (config.GetConfigLocation(index_location).GetBoolGet() != 1) {
        sendError(405, config);
        return ;
    }

    if (path == "/") {
        sendIndex(config);
    }
    else {
        ConfigLocation loc = config.GetConfigLocation(index_location);
        std::string root = loc.GetRoot();
        std::string f_path = root + path;
        struct stat sb;
        if (stat(f_path.c_str(), &sb) != 0 || S_ISDIR(sb.st_mode)) {
            if (loc.GetAutoIndex()) {
                autoIndex(config, loc, f_path);
                // sendIndex(config);
            }
            else if (loc.Getindex() != "") {
                sendLocIndex(config, loc);
            }
            else
                sendError(404, config);
            return;
        }
        _status = "HTTP/1.1 200 OK\r\n";
        std::pair<std::string, std::string> type("Content-Type:", "*/*");
        std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
        _headers.insert(type);
        _headers.insert(length);
        _body = extract_file(f_path.c_str());
    }
}

void Response::sendLocIndex(ConfigServer& config, ConfigLocation& loc) {
    std::string f_path = loc.GetRoot() + loc.GetPath() + "/" + loc.Getindex();
    std::cout << f_path << std::endl;

    struct stat sb;
    if (stat(f_path.c_str(), &sb) != 0) {
        sendError(404, config);
        return ;
    }
    _status = "HTTP/1.1 200 OK\r\n";
    std::pair<std::string, std::string> type("Content-Type:", "*/*");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
    _body = extract_file(f_path);
}

void Response::multipart(std::map<std::string, std::string>::iterator c_type, Request& req, ConfigServer& config, std::string upload_path) {
    std::string delimiter = c_type->second.substr(c_type->second.find("boundary=") + 9);

    std::cout << delimiter << std::endl;

    std::string req_body = req.getBody();

    if (req_body.find("--" + delimiter) == std::string::npos)
        return ;//error

    //get filename
    size_t line_start = req_body.find("Content-Disposition");
    std::string line = req_body.substr(line_start + 21, req_body.find("\n", line_start) - (line_start + 21));
    std::stringstream ss(line);
    std::string file;
    std::vector<std::string> v;

    while(std::getline(ss, file, ' ')) {
        v.push_back(file);
    }
    for (size_t i = 0; i < v.size(); i++) {
        ClearSpace(v[i]);
        if (v[i].find("filename=\"") != std::string::npos)
            file = v[i].substr(v[i].find('=') + 2 , (v[i].size() - 2) - (v[i].find('=') + 2));
    }
    //create file with filename
    std::string f_path = upload_path + "/" + file;
    
    struct stat sb;
    if (stat(f_path.c_str(), &sb) == 0) {
        std::cout << "a file with this name already exists" << std::endl;
        sendError(409, config);
        return ;
    }

    std::ofstream ofs(f_path.c_str());
    
    size_t body_pos = req_body.find("\r\n\r\n", req_body.find("--" + delimiter));
    std::string file_body = req_body.substr(body_pos + 4, (req_body.size()) - (req_body.size() - req_body.find(delimiter + "--") + body_pos + 6));
    _body = file_body;
    ofs << _body;
    _status = "HTTP/1.1 201 Created\r\n";
    std::pair<std::string, std::string> type("Content-Type:", "*/*");
    stat(f_path.c_str(), &sb);
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
}
// mettre dans utils 
std::string generateTimestamp() {
    std::ostringstream time;
    time << std::time(NULL);
    return (time.str());
}

void Response::octetStream(Request& req, ConfigServer& config, std::string upload_path) {
    std::string file = generateTimestamp();
    std::string f_path = upload_path + "/" + file;
    
    struct stat sb;
    if (stat(f_path.c_str(), &sb) == 0) {
        std::cout << "a file with this name already exists" << std::endl;
        sendError(409, config);
        return ;
    }

    std::ofstream ofs(f_path.c_str());
    
    _body = req.getBody();
    ofs << _body;
    _status = "HTTP/1.1 201 Created\r\n";
    std::pair<std::string, std::string> type("Content-Type:", "*/*");
    stat(f_path.c_str(), &sb);
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
}

void Response::postResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();

    int index_location = loc_index(path, config);

    if (index_location < 0) {
        sendError(404, config);
        return;
    }
    else if (config.GetConfigLocation(index_location).GetBoolPost() != 1) {
        sendError(405, config);
        return ;
    }

    ConfigLocation loc = config.GetConfigLocation(index_location);
    std::string upload_path = loc.GetUpload();

    std::map<std::string, std::string> req_head = req.getHeaders();
    std::map<std::string, std::string>::iterator c_type = req_head.find("Content-Type");
    
    if (c_type->second.find("multipart/form-data") != std::string::npos) {
        multipart(c_type, req, config, upload_path);
    }
    else if (c_type->second == "application/octet-stream") {
        octetStream(req, config, upload_path);
    }
    else
        throw std::runtime_error("415 ");
}

void Response::deleteResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();
    
    int index_location = loc_index(path, config);

    if (index_location < 0) {
        sendError(404, config);
        return;
    }
    else if (config.GetConfigLocation(index_location).GetBoolDelete() != 1) {
        sendError(405, config);
        return ;
    }
    
    ConfigLocation loc = config.GetConfigLocation(index_location);
    std::string root = loc.GetRoot();
    std::string f_path = root + path;
    if (remove(f_path.c_str())) {
        sendError(404, config);
        return ;
    }
    _status = "HTTP/1.1 204 No Content\r\n";
}


std::string SetAutoIndexPage(std::string d_path, ConfigLocation& loc, ConfigServer& config);

void    Response::autoIndex(ConfigServer& config, ConfigLocation& loc, std::string f_path)
{
    std::string result;
    std::string d_path = loc.GetRoot() + loc.GetPath();

    struct stat sb;
    if (stat(f_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        result = SetAutoIndexPage(f_path, loc, config);
    }
    else if (stat(d_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        result = SetAutoIndexPage(d_path, loc, config);
    }
    if (result.empty())
        throw (std::runtime_error("404 Error Dirread"));
    _body = result;
    _status = "HTTP/1.1 200 OK\r\n";
    std::pair<std::string, std::string> type("Content-Type:", "text/html");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(_body.size()));
    _headers.insert(type);
    _headers.insert(length);
}

std::string SetAutoIndexPage(std::string d_path, ConfigLocation& loc, ConfigServer &config)
{
    std::string head = "<html>\n<head>\n\t<title>Index of " + loc.GetPath() + "</title>\n<head>\n";
    std::string topBody = "<body>\n\t<h1>Index of " + loc.GetPath() + "</h1>\n\t<hr>\n\t<pre>\n";
    std::string endBody = "\t\t</pre>\n\t<hr>\n</body>\n</html>\n";
    std::vector<std::string>    href;
    DIR *dir = opendir(d_path.c_str());
    if (dir == NULL)
        return (std::string());

    struct dirent *dp;
    std::string domain = "http://" + config.GetServerName() + ":" + config.GetPortStr();
    std::string path = d_path.substr(loc.GetRoot().size());
    if (path.back() != '/')
        path.append("/");
    while ((dp = readdir(dir)) != NULL) {
        std::string name = dp->d_name;
        if (name == ".")
            continue ;
        if (dp->d_type == DT_REG) {
            std::string line = "\t\t<a href=\"" + domain + path  + name + "\">" + name + "</a>\n";
            href.push_back(line);
        }
        if (dp->d_type == DT_DIR) {
            std::string line = "\t\t<a href=\"" + domain + path + name + "\">" + name + "/</a>\n";
            href.push_back(line);
        }
    }
    closedir(dir);
    std::string body = head + topBody;
    for (size_t i = 0; i < href.size(); i++) {
        body.append(href[i]);
    }
    body.append(endBody);
    std::cout << body << std::endl;
    return (body);
}

void Response::sendIndex(ConfigServer& config) {
    std::string f_path = config.GetRoot() + "/" + config.GetIndex();
    std::cout << f_path << std::endl;

    struct stat sb;
    if (stat(f_path.c_str(), &sb) != 0) {
        sendError(404, config);
        return ;
    }
    _status = "HTTP/1.1 200 OK\r\n";
    std::pair<std::string, std::string> type("Content-Type:", "*/*");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
    _body = extract_file(f_path);
}

std::string Response::getStatus(void) const {
    return (_status);
}

void Response::setStatus(std::string status) {
    std::cout << status << std::endl;
    return;
}

std::string Response::getBody(void) const {
    return (_body);
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
            _status = "HTTP/1.1 404 Not Found\r\n";
            break ;
        
        case 405:
            err_file = config.GetErrorPages(status);
            _status = "HTTP/1.1 405 Method Not Allowed\r\n";
            break;

        case 409:
            err_file = config.GetErrorPages(status);
            _status = "HTTP/1.1 409 Conflict\r\n";
            break;
        
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
        return ("NULL");
    }
    std::string body = "";
    std::string line;
    while (std::getline(ifs, line)) {
        body.append(line);
        body.append("\n");					
    }
    return (body);
}

int loc_index(std::string path, ConfigServer& config) {
    int index_location = -1;
    if (path != "/") {
        int index = path.find_first_of('/', 1);
        if (index == -1) {
            index_location = config.FindLocationPath(path);
            if (index_location == -1) {
                std::string file = path.erase(0, 1);
                std::string loc = "/";
                index_location = config.FindLocationPath(loc);
            }
        }
        else {
            std::string loc_path = path.substr(0, index);
            std::cout << loc_path << std::endl;
            index_location = config.FindLocationPath(loc_path);
        }
    }
    else {
        index_location = config.FindLocationPath(path);
    }
    return (index_location);
}