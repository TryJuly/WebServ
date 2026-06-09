/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:35:11 by seully            #+#    #+#             */
/*   Updated: 2026/06/09 09:24:55 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/CGI.hpp"

static std::string	UrlDecode(std::string const &url);

CGI::CGI()
{ return ; }

CGI::CGI(std::string const &request, ConfigLocation &location, ConfigServer &server)
{
	bool isQuery = true;
	size_t	pos = 0;
	size_t	posStart = request.find("/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("Error: Invalid HTTP request");	// Error 400
	size_t	posEnd = request.find("?");
	if (posEnd == std::string::npos) {
		posEnd = request.find(" ", posStart);
		isQuery = false;
	}
	std::string path = request.substr(posStart, posEnd - posStart);
	_stock.insert(std::make_pair("Path", path));
	if (isQuery == true) {
		size_t	posEndQuery = request.find(' ', posEnd);
		std::string query = request.substr(posEnd + 1, posEndQuery - posEnd - 1);
		std::string result = UrlDecode(query);
		_stock.insert(std::make_pair("Query", result));
	}
	// Find html version
	posStart = request.find("HTTP/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("Error: Invalid HTTP request");	// Error 400
	std::string version = request.substr(posStart, 8);	// HTTP/1.1
	_stock.insert(std::make_pair("Version", version));

	// Parse headers
	pos = request.find("\n") + 1;
	while (pos < request.size()) {
		size_t end = request.find("\n", pos);
		if (end != std::string::npos && request[end - 1] == '\r')
			end--;
		if (end == std::string::npos || end == pos)
			break ;
		std::string line = request.substr(pos, end - pos);
		size_t	point = line.find(":");
		if (point != std::string::npos) {
			std::string key = line.substr(0, point);
			std::string value = line.substr(point + 2);
			_stock.insert(std::make_pair(key, value));
		}
		pos = end + 2;
	}

	if (request.substr(0, 3) == "GET") {
		_stock.insert(std::make_pair("Methode", "GET"));
		//	Traitement de la request fork pipe ...
	}
	else if (request.substr(0, 4) == "POST") {
		_stock.insert(std::make_pair("Methode", "POST"));

		std::map<std::string, std::string>::iterator	search = _stock.find("Transfer-Encoding");
		if (search != _stock.end()) {
			if (search->second != "chunked")
				throw (std::invalid_argument("Error: 400 bad Request"));
			std::string body;
			size_t	bodyStart = request.find("\r\n\r\n");
			if (bodyStart == std::string::npos) {
				bodyStart = request.find("\n\n");
				bodyStart += 2;
			}
			else
				bodyStart += 4;
			pos = bodyStart;
			while (true) {
				size_t	chunkSizeEnd = request.find("\r\n", pos);
				std::string	hexSize = request.substr(pos, chunkSizeEnd - pos);
				int chunkSize = std::strtol(hexSize.c_str(), NULL, 16);
				if (chunkSize == 0)
					break ;
				pos = chunkSizeEnd + 2;
				body += request.substr(pos, chunkSize);
				pos += chunkSize + 2;
			}
			_stock.insert((std::make_pair("Body", body)));
			//	Unchunker body
		}
		else if (_stock.find("Content-Length") != _stock.end()) {
			search = _stock.find("Content-Length");
			int contentLength = std::strtol(search->second.c_str(), NULL, 10);
			size_t	bodyStart = request.find("\r\n\r\n");
			if (bodyStart == std::string::npos) {
				bodyStart = request.find("\n\n");
				bodyStart += 2;
			}
			else
				bodyStart += 4;
			std::string body = request.substr(bodyStart, contentLength);
			if (body.size() != contentLength) {
				// return 400 
				return ;
			}
			_stock.insert((std::make_pair("Body", body)));
		}
		else {
			throw (std::invalid_argument("Error: 400 bad request."));
			// Return Error 400 bad request
		}
	}
	// else
		// Send Error 400 bad request
	return ;
}

std::string	CGI::Execute(ConfigLocation &config)
{
	char **envp = new char*[_stock.size() + 1];
	std::string	response;

	int i = 0;
	for (std::map<std::string, std::string>::iterator it = _stock.begin(); it != _stock.end(); it++) {
		if (it->first == "Body")
			continue ;
		std::string input = it->first + "=" + it->second;
		envp[i] = new char[input.size() + 1];
		std::strcpy(envp[i], input.c_str());
		i++;
	}
	envp[i] = NULL;

	std::string scriptPath = config.GetRoot() + _stock["Path"];	// Path exec CGI /usr/bin/python3
	std::string inter = config.GetCGI(scriptPath);		// Path exec script /var/www/cgi-bin/hello.py
	char *argv[] = {
		const_cast<char*>(inter.c_str()),
		const_cast<char*>(scriptPath.c_str()),
		NULL
	};

	int pipeIn[2];
	int pipeOut[2];
	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw (std::runtime_error("Error: pipe() failed: " + std::string(strerror(errno))));

	pid_t child = fork();
	if (child == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		execve(inter.c_str(), argv, envp);
		std::cerr << "Error: child Process" << std::endl;
		exit(1);
	}
	else {
		std::map<std::string, std::string>::iterator it = _stock.find("Body");
		if (it != _stock.end())
			write(pipeIn[1], it->second.c_str(), it->second.size());

		close(pipeIn[0]);
		close(pipeIn[1]);
		char buff[1024];
		while (true) {
			int bytes = read(pipeOut[0], buff, sizeof(buff) - 1);
			if (bytes <= 0)
				break ;
			buff[bytes] = '\0';
			response += buff;
		}
		close(pipeOut[0]);
		close(pipeOut[1]);
		waitpid(child, NULL, 0);
	}
	for (size_t i = 0; envp[i] != NULL; i++)
		delete [] envp[i];
	delete [] envp;
	return (response);
}

/*
	Envp parsing cgi

	REQUEST_METHODE=
	QUERY_STRING=
	PATH_INFO=
	SCRIPT_FILENAME=
	CONTENT_TYPE=
	CONTENT_LENGTH=
	SERVER_NAME=
	SERVER_PORT=
	HTTP_HOST=
*/

static std::string	UrlDecode(std::string const &url)
{
	std::string result;
	for (size_t i = 0; i < url.size(); i++) {
		if (url[i] == '%' && i + 2 < url.size()) {
			std::string hex = url.substr(i + 1, 2);
			char c = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			result.push_back(c);
			i += 2;
		}
		else if (url[i] == '+')
			result.push_back(' ');
		else
			result.push_back(url[i]);
	}
	return (result);
}

CGI::CGI(CGI const &copy)
{
	(*this) = copy;
	return ;
}

CGI::~CGI()
{
	return ;
}

CGI&	CGI::operator=(CGI const &copy)
{
	if (this != &copy)
		_stock = copy._stock;
	return (*this);
}
