/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 08:36:43 by strieste          #+#    #+#             */
/*   Updated: 2026/06/16 11:59:31 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/ConfigServer.hpp"

static char	GetIdentifier(std::string &str);
static char	GetIdentifierLocation(std::string &token);
static void	CheckConfigRequired(std::set<char> &checkDoube);
static int	SkipLocation(std::vector<std::string> &serverBloc, int index);
static ssize_t	StartLocation(std::vector<std::string> &serverChunk, size_t index);
static void	ServerPart(std::vector<std::string> &serverBloc, ConfigServer &config);
static void	FillConfigLocation(ConfigLocation &config, std::vector<std::string> &locationChunk, std::string &path);

/*	Set Configuration Minumum	*/
ConfigServer::ConfigServer()
{
	struct stat	st;
	_port = 8080;
	_socket = -1;
	_maxBodySize = 1048576;
	
	if (stat("./var/www", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www"));
	_rootPath = "./var/www";	// PATH Root TODO

	if (stat("./var/www/html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/html"));
	_index = "./var/www/html";	// PATH Index	TODO
	_serverName = "localhost";

	if (stat("./var/www/errors/400.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/400.html"));
	_errorPages.insert(std::pair<int, std::string>(400, "./var/www/errors/400.html"));
	
	if (stat("./var/www/errors/401.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/401.html"));
	_errorPages.insert(std::pair<int, std::string>(401, "./var/www/errors/401.html"));

	if (stat("./var/www/errors/403.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/403.html"));
	_errorPages.insert(std::pair<int, std::string>(403, "./var/www/errors/403.html"));

	if (stat("./var/www/errors/404.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/404.html"));
	_errorPages.insert(std::pair<int, std::string>(404, "./var/www/errors/404.html"));

	if (stat("./var/www/errors/405.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/405.html"));
	_errorPages.insert(std::pair<int, std::string>(405, "./var/www/errors/405.html"));

	if (stat("./var/www/errors/500.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/500.html"));
	_errorPages.insert(std::pair<int, std::string>(500, "./var/www/errors/500.html"));

	if (stat("./var/www/errors/502.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/502.html"));
	_errorPages.insert(std::pair<int, std::string>(502, "./var/www/errors/502.html"));

	if (stat("./var/www/errors/503.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/503.html"));
	_errorPages.insert(std::pair<int, std::string>(503, "./var/www/errors/503.html"));

	if (stat("./var/www/errors/504.html", &st) != 0)
		throw (std::invalid_argument("Error: Invalid path : ./var/www/errors/504.html"));
	_errorPages.insert(std::pair<int, std::string>(504, "./var/www/errors/504.html"));

	ConfigLocation config;
	config.SetPath("/");
	config.SetMethodes("GET");
	config.SetRoot(_rootPath);
	AddConfigLocation(config);

	return ;
}

void	ConfigServer::AddConfigLocation(ConfigLocation &config)
{
	_locations.push_back(config);
	return ;
}

int	ConfigServer::FindLocationPath(std::string const &path)
{
	for (unsigned int i = 0; i < _locations.size(); i++) {
		if (_locations[i].GetPath() == path)
			return (i);
	}
	return (-1);
}

void	ConfigServer::CleanSetError()
{
	_checkDoubleError.clear();
	return ;
}

ConfigServer::ConfigServer(ConfigServer const &copy)
{
	(*this) = copy;
	return ;
}

ConfigServer::~ConfigServer()
{ return ; }

ConfigServer&	ConfigServer::operator=(ConfigServer const &copy)
{
	if (this != &copy) {
		_port = copy._port;
		_index = copy._index;
		_socket = copy._socket;
		_rootPath = copy._rootPath;
		_locations = copy._locations;
		_serverName = copy._serverName;
		_errorPages = copy._errorPages;
		_sockAddress = copy._sockAddress;
		_maxBodySize = copy._maxBodySize;
		_numberLocation = copy._numberLocation;
		_checkDoubleError = copy._checkDoubleError;
	}
	return (*this);
}

void	ConfigServer::FillConfigServer(std::vector<std::string> &serverChunk)
{
	ServerPart(serverChunk, (*this));

	ssize_t	end = 0;
	ssize_t	start = 0;
	std::set<std::string>	checkDouble;

	while (start < static_cast<ssize_t>(serverChunk.size())) {
		start = StartLocation(serverChunk, start);
		end = EndChunk(serverChunk, start);
		if (start == -1)
			break ;

		std::vector<std::string>	locationChunk;
		for (ssize_t i = start; i < end; i++)
			locationChunk.push_back(serverChunk[i]);

		std::string::size_type	n = locationChunk[0].find("/");
		if (n == std::string::npos)
			throw (std::invalid_argument("Error: Invalid location path."));
		int size = locationChunk[0].find_first_of(" \t{");
		std::string path = locationChunk[0].substr(n, size - n);
		size = path.find_first_of(" \t{");
		path = path.substr(0, size);

		if (checkDouble.count(path) > 0)
			throw (std::invalid_argument("Error: Double location path."));
		checkDouble.insert(path);

		int	indexConfigLocation = FindLocationPath(path);
		if (indexConfigLocation == -1) {
			ConfigLocation conf;
			AddConfigLocation(conf);
			_locations.back().SetRoot(GetRoot());
			FillConfigLocation(_locations.back(), locationChunk, path);
		}
		else {
			_locations[indexConfigLocation].SetRoot(GetRoot());
			FillConfigLocation(_locations[indexConfigLocation], locationChunk, path);
		}
		locationChunk.clear();
		start = end;
	}
}

static void	FillConfigLocation(ConfigLocation &config, std::vector<std::string> &locationChunk, std::string &path)
{
	std::set<std::string>	checkDouble;
	for (unsigned int i = 1; i < locationChunk.size(); i++) {
		size_t	endToken = locationChunk[i].find_first_of(" \t");
		size_t	endValue = locationChunk[i].find_first_of(';');

		if (locationChunk[i] == "}")
			continue ;

		if (endToken == std::string::npos)
			throw (std::invalid_argument("Error: Invalid syntax line: " + locationChunk[i]));
		if (endValue == std::string::npos)
			throw (std::invalid_argument("Error: Missing `;' end of line: " + locationChunk[i]));

		std::string token = locationChunk[i].substr(0, endToken);
		std::string	value = locationChunk[i].substr(endToken + 1, endValue - endToken - 1);

		if (token[0] == '#')
			continue ;

		if (token.compare("cgi") != 0 && checkDouble.count(token) > 0)
			throw (std::invalid_argument("Error: Double declaration of: " + locationChunk[i]));
		checkDouble.insert(token);

		char	iD = GetIdentifierLocation(token);
		ClearSpace(value);
		if (value[value.length() - 1] == ';')
			value.erase(value.length() - 1);

		switch (iD) {
		case 'D':
			break ;
		case 'C':
			config.SetCGI(value);
			break;
		case 'r':
			config.SetRoot(value);
			break;
		case 'R':
			config.SetRedir(value);
			break;
		case 'I':
			config.Setindex(value);
			break;
		case 'U':
			config.SetUpload(value);
			break;
		case 'M':
			config.SetMethodes(value);
			break;
		case 'A':
			config.SetAutoIndex(value);
			break;
		default:
			throw (std::invalid_argument("Error: Invalid argument `" + locationChunk[i] + "'."));
			break;
		}
	}
	config.SetPath(path);
	return ;
}

/*	GETTER	*/

int	ConfigServer::GetPort()
{ return (_port); }

int	ConfigServer::GetSocket()
{ return (_socket); }

int	ConfigServer::GetMaxBodySize()
{ return (_maxBodySize); }

std::string	&ConfigServer::GetRoot()
{ return (_rootPath); }

std::string	&ConfigServer::GetIndex()
{ return (_index); }

std::string	&ConfigServer::GetServerName()
{ return (_serverName); }

struct sockaddr_in	&ConfigServer::GetSockAddr(void)
{ return (_sockAddress); }

std::map<int, std::string>	&ConfigServer::GetMapError()
{ return (_errorPages); }


std::string	ConfigServer::GetErrorPages(int number)
{
	std::map<int, std::string>::iterator it;
	it = _errorPages.find(number);
	if (it == _errorPages.end())
		return ("Error");
	return (it->second);
}
ConfigLocation	&ConfigServer::GetConfigLocation(int index)
{ return (_locations[index]); }

int	ConfigServer::GetNumberLocation(void)
{ return (_locations.size()); }

/*	SETTER	*/

void	ConfigServer::SetPort(int port)
{
	_port = port;
	return ;
}

void	ConfigServer::SetSocket(int socket)
{
	_socket = socket;
	return ;
}

void	ConfigServer::SetMaxBodySize(int BodySize)
{
	_maxBodySize = BodySize;
	return ;
}

void	ConfigServer::SetRoot(std::string const &rootPath)
{
	_rootPath = rootPath;
	return ;
}

void	ConfigServer::SetIndex(std::string const &index)
{
	_index = index;
	return ;
}

void	ConfigServer::SetServerName(std::string const &ServerName)
{
	_serverName = ServerName;
	return ;
}

void	ConfigServer::SetErrorPages(std::string errorPage)
{
	size_t	end = errorPage.find_first_of(" \t");
	if (end == std::string::npos)
		throw (std::invalid_argument("Error: Error page config file."));

	std::string	number = errorPage.substr(0, end);
	std::string	path = errorPage.substr(end, errorPage.size());

	ClearSpace(path);
	char *endptr;
	long result = std::strtol(number.c_str(), &endptr, 10);

	if (*endptr != '\0' || result < 100 || result > 599)
		throw (std::invalid_argument("Error: Invalid error page code: " + number));
	if (_checkDoubleError.count(result) > 0)
		throw (std::invalid_argument("Error: Two same error page find: " + number));
	_checkDoubleError.insert(result);

	std::map<int, std::string>::iterator it;
	it = _errorPages.find(result);

	if (it != _errorPages.end())
		it->second = path;
	else
		_errorPages.insert(std::pair<int, std::string>(result, path));
	return ;
}

void	ConfigServer::SetConfigLocation(ConfigLocation const &config)
{
	_locations.push_back(config);
	return ;
}

void	ConfigServer::SetConfigServer(std::string &str, char iD)
{
	size_t	start = str.find_first_of(" \t");
	size_t	end = str.find(';');

	if (end == std::string::npos)
			throw (std::invalid_argument("Error: Missing `;' end of line: " + str));
	else if (start == std::string::npos)
			throw (std::invalid_argument("Error: Invalid syntax line: " + str));

	std::string value = str.substr(start, end - start);
	ClearSpace(value);
	switch (iD) {
		case 'L': {
			char *endptr;
			long port = std::strtol(value.c_str(), &endptr, 10);
			if (*endptr != '\0' || port < 0 || port > 65535)
				throw (std::invalid_argument("Error: Invalid port value: " + value));
			SetPort(port);
			break ;
		}
		case 'S':
			SetServerName(value);
			break ;
		case 'R':
			SetRoot(value);
			break ;
		case 'I':
			SetIndex(value);
			break ;
		case 'C': {
			char *endptr;
			long size = std::strtol(value.c_str(), &endptr, 10);
			if (*endptr != '\0' || size < 0)
				throw (std::invalid_argument("Error: Invalid port value: " + value));
			SetMaxBodySize(size);
			break ;
		}
		case 'E':
			SetErrorPages(value);
			break ;
		default:
			break;
	}
	return ;
}

/*	Statique help function	*/

static void	ServerPart(std::vector<std::string> &serverBloc, ConfigServer &config)
{
	std::set<char> checkDouble;
	for (unsigned int i = 0; i < serverBloc.size(); i++) {
		if (serverBloc[i][0] == '#')
			continue ;

		char id = GetIdentifier(serverBloc[i]);
		switch (id) {
			case 'L':
				if (checkDouble.count('L') > 0)
					throw (std::invalid_argument("Error: 2 identical instructions: " + serverBloc[i]));
				checkDouble.insert('L');
				config.SetConfigServer(serverBloc[i], 'L');
				break ;
			case 'S':
				if (checkDouble.count('S') > 0)
					throw (std::invalid_argument("Error: 2 identical instructions: " + serverBloc[i]));
				checkDouble.insert('S');
				config.SetConfigServer(serverBloc[i], 'S');
				break ;
			case 'R':
				if (checkDouble.count('R') > 0)
					throw (std::invalid_argument("Error: 2 identical instructions: " + serverBloc[i]));
				checkDouble.insert('R');
				config.SetConfigServer(serverBloc[i], 'R');
				break ;
			case 'I':
				if (checkDouble.count('I') > 0)
					throw (std::invalid_argument("Error: 2 identical instructions: " + serverBloc[i]));
				checkDouble.insert('I');
				config.SetConfigServer(serverBloc[i], 'I');
				break ;
			case 'C':
				if (checkDouble.count('C') > 0)
					throw (std::invalid_argument("Error: 2 identical instructions: " + serverBloc[i]));
				checkDouble.insert('C');
				config.SetConfigServer(serverBloc[i], 'C');
				break ;
			case 'E':
				config.SetConfigServer(serverBloc[i], 'E');
				break ;
			case 'l':
				i = SkipLocation(serverBloc, i);
				break ;
			case 's':
				break ;
			default:
				throw (std::invalid_argument("Error: Invalid syntax line: " + serverBloc[i]));
				break ;
			}
	}
	CheckConfigRequired(checkDouble);
	return ;
}

static void	CheckConfigRequired(std::set<char> &checkDoube)
{
	for (size_t	i = 0; i < checkDoube.size(); i++) {
		if (checkDoube.count('L') < 1)
			throw (std::invalid_argument("Error: Missing `listen' instructions."));
		if (checkDoube.count('S') < 1)
			throw (std::invalid_argument("Error: Missing `server_name' instructions."));
		if (checkDoube.count('R') < 1)
			throw (std::invalid_argument("Error: Missing `root' instructions."));
		if (checkDoube.count('I') < 1)
			throw (std::invalid_argument("Error: Missing `index' instructions."));
		if (checkDoube.count('C') < 1)
			throw (std::invalid_argument("Error: Missing `client_max_body_size' instructions."));
	}
	return ;
}

static int	SkipLocation(std::vector<std::string> &serverBloc, int index)
{
	int balance = 1;
	unsigned int i = index + 1;

	while (i < serverBloc.size()) {
		int j = 0;
		while (serverBloc[i][j]) {
			if (serverBloc[i][j] == '{')
				balance++;
			else if (serverBloc[i][j] == '}') {
				balance--;
				if (balance == 0)
					return (i) ;
			}
			j++;
		}
		i++;
	}
	return (-1);
}

static char	GetIdentifier(std::string &str)
{
	size_t	index = str.find_first_of(" \t");
	if (index == std::string::npos && str.compare("}"))
		throw (std::invalid_argument("Error: Invalid syntax line: " + str));
	std::string token = str.substr(0, index);

	if (!token.compare("listen"))
		return ('L');
	if (!token.compare("server_name"))
		return ('S');
	if (!token.compare("root"))
		return ('R');
	if (!token.compare("index"))
		return ('I');
	if (!token.compare("client_max_body_size"))
		return ('C');
	if (!token.compare("error_page"))
		return ('E');
	if (!token.compare("location"))
		return ('l');
	if (!token.compare("server") || !token.compare("{") || !token.compare("}"))
		return ('s');
	return ('n');
}

static char	GetIdentifierLocation(std::string &token)
{
	if (!token.compare("autoindex"))
		return ('A');
	if (!token.compare("methods"))
		return ('M');
	if (!token.compare("index"))
		return ('I');
	if (!token.compare("redirect"))
		return ('R');
	if (!token.compare("root"))
		return ('r');
	if (!token.compare("cgi"))
		return ('C');
	if (!token.compare("upload_path"))
		return ('U');
	if (!token.compare("}"))
		return ('D');
	return ('e');
}

static ssize_t	StartLocation(std::vector<std::string> &serverChunk, size_t index)
{
	unsigned int	i = index;

	while (i < serverChunk.size() && serverChunk[i].find("location") == std::string::npos)	// change
		i++;

	if (i >= serverChunk.size())	// change
		return (-1);
	return (i);
}
