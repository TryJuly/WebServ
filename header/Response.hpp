#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Request.hpp"
#include "ConfigServer.hpp"

#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <dirent.h>   // directory
#include <sys/types.h>

class Response {

    private:

        std::string _status;
        std::map<std::string, std::string> _headers;
        std::string _body;
        void    getResponse(Request& req, ConfigServer& config);
        void    postResponse(Request& req, ConfigServer& config);
        void    deleteResponse(Request& req, ConfigServer& config);
        //void    sendIndex(ConfigServer& config);
        void    sendRedir(std::string path, ConfigServer& config, ConfigLocation& loc);
        void    sendLocIndex(ConfigLocation& loc);
        void    multipart(std::map<std::string, std::string>::iterator c_type, Request& req, std::string upload_path);
        void    octetStream(Request& req, std::string upload_path);
        void    autoIndex(ConfigServer& config, ConfigLocation& loc, std::string f_path);


    public:

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        ~Response();

        Response( Request& req, ConfigServer& config);
        std::string getStatus(void) const;
        void setStatus(std::string status);
        std::string getBody(void) const;
        std::string printResponse(void);
        //void sendError(int status, ConfigServer& config);
};

std::string extract_file(std::string filename);
std::string return_file_length(size_t length);
int         loc_index(std::string path, ConfigServer& config);
std::string MimeType(std::string const &path);

#endif