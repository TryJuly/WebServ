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

    if (req.getMethod() == "POST")
        postResponse(req, config);

    else if (req.getMethod() == "DELETE")
        deleteResponse(req, config);


    else if (req.getMethod() == "GET")
        getResponse(req, config);

    else
        throw std::runtime_error("405 ");
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

    if (index_location < 0)
        throw std::runtime_error("404 ");

    else if (config.GetConfigLocation(index_location).GetRedir() != "") {
        sendRedir(path, config, config.GetConfigLocation(index_location));
        return ;
    }
    else if (config.GetConfigLocation(index_location).GetBoolGet() != 1)
        throw std::runtime_error("405 ");

    ConfigLocation loc = config.GetConfigLocation(index_location);
    std::string root = loc.GetRoot();
    std::string f_path = root + path;
    struct stat sb;
    if (stat(f_path.c_str(), &sb) != 0)
        throw std::runtime_error("404 ");
    if (S_ISDIR(sb.st_mode)) {
        if (loc.GetAutoIndex()) {
            autoIndex(config, loc, f_path);
            return ;
        }
        else if (loc.Getindex() != "") {
            sendLocIndex(loc);
            return ;
        }
        else
            throw std::runtime_error("404 ");
    }
    _body = extract_file(f_path.c_str());
    if (_body.empty())
        throw std::runtime_error("500 ");
    _status = "HTTP/1.1 200 OK\r\n";
    std::pair<std::string, std::string> type("Content-Type:", MimeType(f_path));
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
}

void Response::sendLocIndex(ConfigLocation& loc) {
    std::string f_path;
    if (loc.GetPath()[-1] != '/')
        f_path = loc.GetRoot() + loc.GetPath() + "/" + loc.Getindex();
    else
        f_path = loc.GetRoot() + loc.GetPath() + loc.Getindex();
    struct stat sb;
    if (stat(f_path.c_str(), &sb) != 0)
        throw std::runtime_error("404 ");
    _body = extract_file(f_path);
    if (_body.empty())
        throw std::runtime_error("500 ");

    _status = "HTTP/1.1 200 OK\r\n";
    std::pair<std::string, std::string> type("Content-Type:", MimeType(f_path));
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(sb.st_size));
    _headers.insert(type);
    _headers.insert(length);
}

void Response::multipart(std::map<std::string, std::string>::iterator c_type, Request& req, std::string upload_path) {
    std::string delimiter = "--" + c_type->second.substr(c_type->second.find("boundary=") + 9);
    std::string end_delim = delimiter + "--";

    std::string req_body = req.getBody();

    std::vector<std::string> parts;

    size_t pos = 0;
    while (req_body.find(delimiter, pos) != std::string::npos) {
        std::string part = req_body.substr(pos, req_body.find(delimiter, pos + 1) - pos);
        parts.push_back(part);
        if (req_body.find(delimiter, pos + 1) == req_body.find(end_delim))
            break ;
        pos = req_body.find(delimiter, pos + 1);
    }

    std::string first_loc = "";
    size_t  first_size = 0;
    (void) first_size;

    for (unsigned int i = 0; i < parts.size(); i++) {
        size_t line_start = parts[i].find("Content-Disposition");
        std::string line = parts[i].substr(line_start + 21, parts[i].find("\n", line_start) - (line_start + 21));
        std::stringstream ss(line);
        std::string token;
        std::string file;
        std::vector<std::string> v;

        while(std::getline(ss, token, ' ')) {
            v.push_back(token);
        }
        for (size_t i = 0; i < v.size(); i++) {
            ClearSpace(v[i]);
            if (v[i].find("filename=\"") != std::string::npos)
                file = v[i].substr(v[i].find('=') + 2 , (v[i].size() - 2) - (v[i].find('=') + 2));
        }
        if (file.empty())
            continue ;

        std::string f_path = upload_path + "/" + file;

        struct stat sb;
        if (stat(f_path.c_str(), &sb) == 0)
            throw std::runtime_error("409 ");

        std::ofstream ofs(f_path.c_str(), std::ios::binary);

        size_t body_pos = parts[i].find("\r\n\r\n");
        if (body_pos == std::string::npos)
            throw std::runtime_error("400 ");

        size_t data_start = body_pos + 4;
        size_t data_end = parts[i].size();
        if (data_end >= data_start + 2 && parts[i][data_end - 2] == '\r' && parts[i][data_end - 1] == '\n')
            data_end -= 2;
        if (data_end < data_start)
            throw std::runtime_error("400 ");

        std::string file_body = parts[i].substr(data_start, data_end - data_start);
        ofs << file_body;

        if (first_loc == "") {
            first_loc = req.getPath() + "/" + file;
            struct stat sb;
            stat(f_path.c_str(), &sb);
            first_size = sb.st_size;
        }
    }
    _status = "HTTP/1.1 201 Created\r\n";
    std::string responseBody = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Upload</title></head><body><h1>Upload successful</h1></body></html>";
    std::pair<std::string, std::string> type("Content-Type:", "text/html");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(responseBody.size()));
    _body = responseBody;
    std::pair<std::string, std::string> location("Location: ", first_loc);
    _headers.insert(type);
    _headers.insert(length);
    _headers.insert(location);
}

std::string generateFilename() {
    static int upcount = 0; 
    std::stringstream ss;
    ss << upcount++;
    return (ss.str());
}

void Response::octetStream(Request& req, std::string upload_path) {

    std::string file = generateFilename();
    std::string f_path = upload_path + "/" + file;
    
    struct stat sb;
    if (stat(f_path.c_str(), &sb) == 0)
        throw std::runtime_error("409 ");

    std::ofstream ofs(f_path.c_str(), std::ios::binary);
    ofs << req.getBody();
    _status = "HTTP/1.1 201 Created\r\n";
    std::string responseBody = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Upload</title></head><body><h1>Upload successful</h1></body></html>";
    std::pair<std::string, std::string> type("Content-Type:", "text/html");
    std::pair<std::string, std::string> length("Content-Length:", return_file_length(responseBody.size()));
    _body = responseBody;
    _headers.insert(type);
    _headers.insert(length);
}

void Response::postResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();

    int index_location = loc_index(path, config);

    if (index_location < 0)
        throw std::runtime_error("404 ");

    else if (config.GetConfigLocation(index_location).GetBoolPost() != 1)
        throw std::runtime_error("405 ");

    ConfigLocation loc = config.GetConfigLocation(index_location);
    std::string upload_path = loc.GetUpload();

    std::map<std::string, std::string> req_head = req.getHeaders();
    std::map<std::string, std::string>::iterator c_type = req_head.find("Content-Type");
    if (c_type == req_head.end())
        throw std::runtime_error("415 ");

    if (c_type->second.find("multipart/form-data") != std::string::npos) {
        multipart(c_type, req, upload_path);
    }
    else if (c_type->second == "application/octet-stream" || c_type->second == "text/plain") {
        octetStream(req, upload_path);
    }
    else
        throw std::runtime_error("415 ");
}

void Response::deleteResponse(Request& req, ConfigServer& config) {
    std::string path = req.getPath();
    
    int index_location = loc_index(path, config);

    if (index_location < 0)
        throw std::runtime_error("404 ");

    else if (config.GetConfigLocation(index_location).GetBoolDelete() != 1)
        throw std::runtime_error("405 ");
    
    ConfigLocation loc = config.GetConfigLocation(index_location);
    std::string root = loc.GetRoot();
    std::string f_path = root + path;
    if (remove(f_path.c_str()))
        throw std::runtime_error("404 ");

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
    std::string head = "<html>\n<head>\n\t<title>Index of " + loc.GetPath() + "</title>\n</head>\n";
    std::string topBody = "<body>\n\t<h1>Index of " + loc.GetPath() + "</h1>\n\t<hr>\n\t<pre>\n";
    std::string endBody = "\t\t</pre>\n\t<hr>\n</body>\n</html>\n";
    std::vector<std::string>    href;
    DIR *dir = opendir(d_path.c_str());
    if (dir == NULL)
        return (std::string());

    struct dirent *dp;
    std::string domain = "http://" + config.GetServerName() + ":" + config.GetPortStr();
    size_t rootLen = loc.GetRoot().size();
    std::string path = (rootLen <= d_path.size()) ? d_path.substr(rootLen) : d_path;
    if (path.empty() || path[path.size() - 1] != '/')
        path.append("/");
    while ((dp = readdir(dir)) != NULL) {
        std::string name = dp->d_name;
        if (name == "." || name == "..")
            continue ;
        if (dp->d_type == DT_REG) {
            std::string line = "\t\t<a href=\"" + domain + path + name + "\">" + name + "</a>\n";
            href.push_back(line);
        }
        else if (dp->d_type == DT_DIR) {
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
    return (body);
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

std::string extract_file(std::string filename) {
    std::ifstream ifs(filename.c_str(), std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "Not good file extraction" << std::endl;
        return ("");
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
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
            index_location = config.FindLocationPath(loc_path);
        }
    }
    else {
        index_location = config.FindLocationPath(path);
    }
    return (index_location);
}