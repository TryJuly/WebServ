#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include "Request.hpp"
#include <sys/stat.h>
#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstdio>

class Response {

    private:

        std::string _status;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:

        Response();
        Response(const Response& other);
        Response& operator=(const Response& other);
        ~Response();

        Response( Request& req);
        std::string getStatus(void) const;
        void setStatus(std::string status);
        std::string printResponse(void);
        void sendError(const char *status);
};

std::string extract_file(std::string filename);
std::string return_file_length(size_t length);

#endif