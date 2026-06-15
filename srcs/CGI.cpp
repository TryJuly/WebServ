/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:35:11 by seully            #+#    #+#             */
/*   Updated: 2026/06/15 14:48:39 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/CGI.hpp"
#include <stdexcept>

static std::string	UrlDecode(std::string const &url);

CGI::CGI()
{ return ; }

CGI::CGI(std::string const &request)
{
	_envp = NULL;
	ParseFirstLine(request);
	// Parse headers
	ParseHeaders(request);

	std::map<std::string, std::string>::iterator	it = _stock.find("Methode");

	if (it == _stock.end() || (it->second != "GET" && it->second != "POST"))
		throw (std::invalid_argument("400 Error: Invalid HTTP request"));

	if (it->second == "POST") {
		std::map<std::string, std::string>::iterator	search = _stock.find("Transfer-Encoding");
		if (search != _stock.end()) {
			if (search->second != "chunked")
				throw (std::invalid_argument("400 Error: Invalid HTTP request"));
			std::string body;
			size_t	bodyStart = request.find("\r\n\r\n");
			if (bodyStart != std::string::npos)
				bodyStart += 4;
			else {
				bodyStart = request.find("\n\n");
				if (bodyStart == std::string::npos)
					throw(std::invalid_argument("400 Error: Invalid Http request"));
				bodyStart += 2;
			}
			size_t	pos = bodyStart;
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
		}
		else if (_stock.find("Content-Length") != _stock.end()) {
			search = _stock.find("Content-Length");
			size_t contentLength = std::strtol(search->second.c_str(), NULL, 10);
			size_t	bodyStart = request.find("\r\n\r\n");
			if (bodyStart == std::string::npos) {
				bodyStart = request.find("\n\n");
				bodyStart += 2;
			}
			else
				bodyStart += 4;
			std::string body = request.substr(bodyStart, contentLength);
			if (body.size() != contentLength)
				throw (std::invalid_argument("400 Error: Invalid HTTP request"));
			_stock.insert((std::make_pair("Body", body)));
		}
		else
			throw (std::invalid_argument("405 Error: Invalid HTTP request"));
	}
	// std::cout << RED << "HERE4" << RESET << std::endl;
	return ;
}

void	CGI::ParseFirstLine(std::string const &request)
{
	bool isQuery = true;
	// size_t	pos = 0;
	std::string first_line = request.substr(0, request.find_first_of('\n'));
	size_t	posStart = first_line.find("/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("400 Error: Invalid HTTP request");	// Error 400
	if (first_line.substr(0, 3) == "GET")
		_stock.insert(std::make_pair("Methode", "GET"));
	else if (first_line.substr(0, 4) == "POST")
		_stock.insert(std::make_pair("Methode", "POST"));
	size_t	posEnd = first_line.find("?");
	if (posEnd == std::string::npos) {
		posEnd = first_line.find(" ", posStart);
		isQuery = false;
	}
	std::string path = first_line.substr(posStart, posEnd - posStart);
	_stock.insert(std::make_pair("Path", path));

	if (isQuery == true) {
		size_t	posEndQuery = first_line.find(' ', posEnd);
		std::string query = first_line.substr(posEnd + 1, posEndQuery - posEnd - 1);
		std::string result = UrlDecode(query);
		_stock.insert(std::make_pair("Query", result));
	}
	// Find html version
	posStart = first_line.find("HTTP/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("400");	// Error 400
	std::string version = first_line.substr(posStart, 8);	// HTTP/1.1
	_stock.insert(std::make_pair("Version", version));
	return ;
}

void	CGI::ParseHeaders(std::string const &request)
{
	// size_t	startBody = request.find("\r\n\r\n");
	size_t	pos = request.find("\n") + 1;
	while (pos < request.size()) {	//	pos < startBody
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
	return ;
}

void	CGI::SetEnvpCGI(ConfigLocation const &config)
{
	std::map<std::string, std::string>::iterator it = _stock.find("Methode");
	if (it->second == "GET" && config.GetBoolGet() == false)
		throw (std::runtime_error("405 Error: Methode not allowed"));
	else if (it->second == "POST" && config.GetBoolPost() == false)
		throw (std::runtime_error("405 Error: Methode not allowed"));

	_envp = new char*[_stock.size() + 1];

	int i = 0;
	for (std::map<std::string, std::string>::iterator it = _stock.begin(); it != _stock.end(); it++) {
		if (it->first == "Body")
			continue ;
		std::string input = it->first + "=" + it->second;
		_envp[i] = new char[input.size() + 1];
		std::strcpy(_envp[i], input.c_str());
		i++;
	}
	_envp[i] = NULL;

	_scriptPath = config.GetRoot() + _stock["Path"];	// Path exec CGI /usr/bin/python3
	_scriptPath = _scriptPath.substr(_scriptPath.find('/') + 1);
	struct stat sstat;
	// std::cout << RED << "HERE" << RESET << std::endl;
	if (stat(_scriptPath.c_str(), &sstat) != 0)
		throw (std::invalid_argument("404 Error: Invalid path location: " + _scriptPath));
	std::string extension = _scriptPath.substr(_scriptPath.find_last_of('.'));
	_inter = config.GetCGI(extension);		// Path exec script /var/www/cgi-bin/hello.py
	return ;
}

// std::string	CGI::Execute(ConfigLocation const &config)
// {
// 	SetEnvpCGI(config);
// 	for (size_t	i = 0; _envp[i]; i++)
// 		std::cout << _envp[i] << std::endl;

// 	char *argv[] = {
// 		const_cast<char*>(_inter.c_str()),
// 		const_cast<char*>(_scriptPath.c_str()),
// 		NULL
// 	};

// 	int pipeFd[2];
// 	std::string	response;
// 	if (pipe(pipeFd) == -1)
// 		throw (std::runtime_error("500 Error: pipe() failed: " + std::string(strerror(errno))));

// 	SetTimeStart(std::time(NULL));
// 	pid_t child = fork();
// 	if (child == -1)
// 		throw (std::runtime_error("500 Error fork() failed: " + std::string(strerror(errno))));
// 	if (child == 0) {
// 		dup2(pipeFd[0], STDIN_FILENO);
// 		dup2(pipeFd[1], STDOUT_FILENO);
// 		close(pipeFd[0]);
// 		close(pipeFd[1]);
// 		execve(argv[0], argv, _envp);
// 		std::cerr << "Error: child Process" << std::endl;
// 		exit(1);
// 	}
// 	else {
// 		std::map<std::string, std::string>::iterator it = _stock.find("Body");
// 		if (it != _stock.end())
// 		write(pipeFd[1], it->second.c_str(), it->second.size());

// 		close(pipeFd[1]);
// 		char buff[1024];
// 		fcntl(pipeFd[0], F_SETFL, O_NONBLOCK);
// 		while (true) {
// 			if (time(NULL) - _cgiStart > 10) {
// 				kill(child, SIGKILL);
// 				waitpid(child, NULL, 0);
// 				throw (std::runtime_error("504 Error: Timeout cgi"));
// 			}
// 			int bytes = read(pipeFd[0], buff, sizeof(buff) - 1);
// 			if (bytes > 0) {
// 				buff[bytes] = '\0';
// 				response += buff;
// 			}
// 			else if (bytes == 0)
// 				break ;
// 			else if (errno != EAGAIN)
// 				break ;
// 		}
// 		close(pipeFd[0]);
// 	}

	//	Create status line and content-length
// 	if (response.find("Content-Length:") == std::string::npos) {
// 		std::string body = response.substr(response.find("\r\n\r\n") + 4);
// 		std::string length = return_file_length(body.size());
// 		std::string contentLength = "Content-Length: " + length + "\r\n";
// 		response = contentLength + response;
// 	}
// 	if (response.substr(0, 5) != "HTTP/") {
// 		std::string status = "HTTP/1.1 200 OK\r\n";
// 		response = status + response;
// 	}
// 	return (response);
// }

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

void	CGI::SetTimeStart(std::time_t time)
{
	_cgiStart = time;
	return ;
}

CGI::CGI(CGI const &copy)
{
	(*this) = copy;
	return ;
}

CGI::~CGI()
{
	if (_envp) {
		for (size_t i = 0; _envp[i] != NULL; i++)
			if (_envp[i])
				delete [] _envp[i];
		delete [] _envp;
	}
	return ;
}

CGI&	CGI::operator=(CGI const &copy)
{
	if (this != &copy) {
		if (copy._envp) {
			int i = 0;
			while (copy._envp[i])
				i++;
			_envp = new char*[i + 1];
			for (int j = 0; j < i; j++) {
				_envp[j] = new char[std::strlen(copy._envp[j]) + 1];
				std::strcpy(_envp[j], copy._envp[j]);
			}
			_envp[i] = NULL;
		}
		_stock = copy._stock;
		_inter = copy._inter;
		_scriptPath = copy._scriptPath;
		_cgiStart = copy._cgiStart;
		_pid = copy._pid;
		_pipeFd = copy._pipeFd;
	}
	return (*this);
}

void	CGI::LaunchCGI(ConfigLocation const &config)
{
	SetEnvpCGI(config);
	char *argv[] = {
		const_cast<char*>(_inter.c_str()),
		const_cast<char*>(_scriptPath.c_str()),
		NULL
	};
	int pipeIn[2];
	int pipeOut[2];
	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw (std::runtime_error("500 Error: pipe() failed: "));

	pid_t	child = fork();
	if (child == -1)
		throw (std::runtime_error("500 Error: fork() failed: "));
	if (child == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeOut[0]);
		close(pipeOut[1]);
		execve(argv[0], argv, _envp);
		exit(1);
	}
	else {
		std::map<std::string, std::string>::iterator it = _stock.find("Body");
		close(pipeIn[0]);
		if (it != _stock.end())
			write(pipeIn[1], it->second.c_str(), it->second.size());
		close(pipeIn[1]);
		close(pipeOut[1]);
		fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);	// Set le fd en mode non bloquant
		_pipeFd = pipeOut[0];
		_pid = child;
	}
	return ;
}

pid_t	CGI::GetPidCgi()
{ return (_pid); }

int		CGI::GetPipeFd()
{ return (_pipeFd); }