/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:35:11 by seully            #+#    #+#             */
/*   Updated: 2026/07/02 06:45:12 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/CGI.hpp"

static bool ValidMethode(std::string method);
static	std::string	SetEnvpVarName(std::string str);
static std::string	UrlDecode(std::string const &url);

/*	Default	*/

CGI::CGI() : _pid(-1), _envp(NULL), _pipeFd(-1), _pipeInFd(-1)
{ return ; }

CGI::CGI(std::string const &request)
{
	_envp = NULL;
	ParseFirstLine(request);
	ParseHeaders(request);

	std::map<std::string, std::string>::iterator	it = _stock.find("Methode");

	if (it == _stock.end() || ValidMethode(it->second) == false)
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
	else if (it->second != "GET" && it->second != "POST")
			throw (std::invalid_argument("405 Error: Invalid HTTP request"));
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
		_pipeInFd = copy._pipeInFd;
	}
	return (*this);
}

void	CGI::ParseFirstLine(std::string const &request)
{
	bool isQuery = true;
	std::string first_line = request.substr(0, request.find_first_of('\n'));
	size_t	posStart = first_line.find("/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("400 Error: Invalid HTTP request");
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

	posStart = first_line.find("HTTP/");
	if (posStart == std::string::npos)
		throw std::invalid_argument("400");
	std::string version = first_line.substr(posStart, 8);
	_stock.insert(std::make_pair("Version", version));
	return ;
}

void	CGI::ParseHeaders(std::string const &request)
{
	size_t	pos = request.find("\n") + 1;
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
	return ;
}

void	CGI::LaunchCGI(ConfigLocation const &config, ConfigServer &configServer)
{
	SetEnvpCGI(config, configServer);
	char *argv[] = {
		const_cast<char*>(_inter.c_str()),
		const_cast<char*>(_scriptPath.c_str()),
		NULL
	};
	int pipeIn[2];
	int pipeOut[2];
	if (pipe(pipeIn) == -1)
		throw (std::runtime_error("500 Error: pipe() failed: "));

	if (pipe(pipeOut) == -1) {
		close(pipeIn[0]);
		close(pipeIn[1]);
		throw (std::runtime_error("500 Error: pipe() failed: "));
	}

	pid_t	child = fork();
	if (child == -1)
		throw (std::runtime_error("500 Error: fork() failed: "));
	if (child == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		dup2(pipeOut[1], STDOUT_FILENO);
		dup2(pipeOut[1], STDERR_FILENO);
		close(pipeOut[0]);
		close(pipeOut[1]);
		execve(argv[0], argv, _envp);
		exit(1);
	}
	else {
		close(pipeIn[0]);
		fcntl(pipeIn[1], F_SETFL, O_NONBLOCK);
		_pipeInFd = pipeIn[1];
		close(pipeOut[1]);
		fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);
		_pipeFd = pipeOut[0];
		_pid = child;
	}
	return ;
}

/*	Getter	*/

ssize_t	CGI::GetBodySize( void )
{
	std::map<std::string, std::string>::iterator it = _stock.find("Body");
	if (it == _stock.end())
		return (-1);
	return (it->second.size());
}

pid_t	CGI::GetPidCgi()
{ return (_pid); }

int		CGI::GetPipeFd()
{ return (_pipeFd); }

int		CGI::GetPipeInFd()
{ return (_pipeInFd); }

std::string	CGI::GetCgiBody()
{
	std::map<std::string, std::string>::iterator it = _stock.find("Body");
	if (it == _stock.end())
		return ("");
	return (it->second);
}

/*	Setter	*/

void	CGI::SetTimeStart(std::time_t time)
{
	_cgiStart = time;
	return ;
}

void	CGI::SetEnvpCGI(ConfigLocation const &config, ConfigServer &configServer)
{
	std::map<std::string, std::string>::iterator it = _stock.find("Methode");
	if (it->second == "GET" && config.GetBoolGet() == false)
		throw (std::runtime_error("405 Error: Methode not allowed"));
	else if (it->second == "POST" && config.GetBoolPost() == false)
		throw (std::runtime_error("405 Error: Methode not allowed"));

	_scriptPath = config.GetRoot() + _stock["Path"];
	_scriptPath = _scriptPath.substr(_scriptPath.find('/') + 1);
	struct stat sstat;
	if (stat(_scriptPath.c_str(), &sstat) != 0)
		throw (std::invalid_argument("404 Error: Invalid path location: " + _scriptPath));
	std::string extension = _scriptPath.substr(_scriptPath.find_last_of('.'));
	_inter = config.GetCGI(extension);

	_envp = new char*[_stock.size() + 9];
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = _stock.begin(); it != _stock.end(); it++) {
		if (it->first == "Body")
			continue ;
		std::string input = SetEnvpVarName(it->first) + "=" + it->second;
		_envp[i] = new char[input.size() + 1];
		std::strcpy(_envp[i], input.c_str());
		i++;
	}
	if (_stock.find("Query") == _stock.end()) {
		std::string input = "QUERY_STRING=";
		_envp[i] = new char[input.size() + 1];
		std::strcpy(_envp[i++], input.c_str());
	}
	std::string scriptFilename = "SCRIPT_FILENAME=" + _scriptPath;
	_envp[i] = new char[scriptFilename.size() + 1];
	std::strcpy(_envp[i++], scriptFilename.c_str());
	std::string pathInfo = "PATH_INFO=";
	_envp[i] = new char[pathInfo.size() + 1];
	std::strcpy(_envp[i++], pathInfo.c_str());
	std::string serverName = "SERVER_NAME=" + configServer.GetServerName();
	_envp[i] = new char[serverName.size() + 1];
	std::strcpy(_envp[i++], serverName.c_str());
	std::string serverPort = "SERVER_PORT=" + configServer.GetPortStr();
	_envp[i] = new char[serverPort.size() + 1];
	std::strcpy(_envp[i++], serverPort.c_str());
	std::string gateway = "GATEWAY_INTERFACE=CGI/1.1";
	_envp[i] = new char[gateway.size() + 1];
	std::strcpy(_envp[i++], gateway.c_str());
	std::string	php = "REDIRECT_STATUS=200";
	_envp[i] = new char[php.size() + 1];
	std::strcpy(_envp[i++], php.c_str());
	_envp[i] = NULL;

	return ;
}

/*	Static Function	*/

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

static	std::string	SetEnvpVarName(std::string str)
{
	if (!str.compare("Methode"))
		return (std::string("REQUEST_METHOD"));
	else if (!str.compare("Query"))
		return (std::string("QUERY_STRING"));
	else if (!str.compare("Path"))
		return (std::string("SCRIPT_NAME"));
	else if (!str.compare("Content-Length"))
		return (std::string("CONTENT_LENGTH"));
	else if (!str.compare("Content-Type"))
		return (std::string("CONTENT_TYPE"));
	else if (!str.compare("Version"))
		return (std::string("SERVER_PROTOCOL"));
	std::string result = "HTTP_";
	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] == '-')
			result += '_';
		else
			result += static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
	}
	return (result);
}

static bool ValidMethode(std::string method)
{
	if (method == "GET")
		return (true);
	if (method == "POST")
		return (true);
	if (method == "DELETE")
		return (true);
	if (method == "PATCH")
		return (true);
	if (method == "PUT")
		return (true);
	if (method == "OPTIONS")
		return (true);
	if (method == "HEAD")
		return (true);
	if (method == "CONNECT")
		return (true);
	if (method == "TRACE")
		return (true);
	return (false);
}
