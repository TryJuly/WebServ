#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <iostream>

class Request {

    private:

        std::string _method;
        std::string _path;
        std::map<std::string, std::string> _headers;
        std::string _body;
        bool	_isCGI;
        void    fillHeaders(std::string req);

    public:

        Request();
        Request(const Request& other);
        Request& operator=(const Request& other);
        ~Request();

        Request(std::string buff);
        std::string getMethod(void) const;
        std::string getPath(void) const ;
        std::map<std::string, std::string> getHeaders(void) const;
        std::string getBody(void) const;
		bool	IsCGI( void );

};

#endif