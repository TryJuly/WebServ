/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 08:36:43 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 10:12:43 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/ConfigServer.hpp"

static char	GetIdentifier(std::string &str);
static int	SkipLocation(std::vector<std::string> &serverBloc, int index);
static ssize_t	StartLocation(std::vector<std::string> &serverChunk, size_t index);
static void	ServerPart(std::vector<std::string> &serverBloc, ConfigServer &config);
static void	FillConfigLocation(ConfigLocation &config, std::vector<std::string> &locationChunk, std::string &path);

/*	Set Configuration Minumum	*/
ConfigServer::ConfigServer()
{
	_port = 8080;
	_socket = -1;
	_maxBodySize = 1048576;
	_rootPath = "./";	// PATH Root TODO
	_index = "./";	// PATH Index	TODO
	_serverName = "localhost";
	_errorPages.insert(std::pair<int, std::string>(400, "/errors/400.html"));
	_errorPages.insert(std::pair<int, std::string>(401, "/errors/401.html"));
	_errorPages.insert(std::pair<int, std::string>(403, "/errors/403.html"));
	_errorPages.insert(std::pair<int, std::string>(404, "/errors/404.html"));
	_errorPages.insert(std::pair<int, std::string>(500, "/errors/500.html"));
	_errorPages.insert(std::pair<int, std::string>(502, "/errors/502.html"));
	_errorPages.insert(std::pair<int, std::string>(503, "/errors/503.html"));
	_errorPages.insert(std::pair<int, std::string>(504, "/errors/504.html"));

	ConfigLocation config;
	config.SetPath("/");
	config.SetMethodes("GET");
	_locations.push_back(config);

	return ;
}

void	ConfigServer::AddConfigLocation(ConfigLocation &config)
{
	_locations.push_back(config);
	return ;
}

int	ConfigServer::FindLocationPath(std::string &path)
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

ConfigServer::ConfigServer(std::string const &file)
{
	(void) file;
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
		_socket = copy._socket;
		_maxBodySize = copy._maxBodySize;
		_numberLocation = copy._numberLocation;
		_rootPath = copy._rootPath;
		_index = copy._index;
		_serverName = copy._serverName;
		_errorPages = copy._errorPages;
		_locations = copy._locations;
	}
	return (*this);
}

int	ConfigServer::GetPort()
{ return (_port); }

int	ConfigServer::GetSocket()
{ return (_socket); }

int	ConfigServer::GetMaxBodySize()
{ return (_maxBodySize); }

std::string&	ConfigServer::GetRoot()
{ return (_rootPath); }

std::string&	ConfigServer::GetIndex()
{ return (_index); }

std::string&	ConfigServer::GetServerName()
{ return (_serverName); }

struct sockaddr_in&	ConfigServer::GetSockAddr(void)
{ return (_sockAddress); }

std::string	ConfigServer::GetErrorPages(int number)
{
	std::map<int, std::string>::iterator it;
	it = _errorPages.find(number);
	if (it == _errorPages.end())
		return ("Error");
	return (it->second);
}
ConfigLocation&	ConfigServer::GetConfigLocation(int index)
{ return (_locations[index]); }

int	ConfigServer::GetNumberLocation(void)
{ return (_locations.size()); }

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
	std::string	number = errorPage.substr(0, end);
	std::string	path = errorPage.substr(end, errorPage.size());
	ClearSpace(path);
	int	result = std::atoi(number.c_str());
	if (_checkDoubleError.count(result) > 0)
		throw (std::invalid_argument("Error: Two same error page find."));
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
		else
			checkDouble.insert(path);

		int	indexConfigLocation = FindLocationPath(path);
		if (indexConfigLocation == -1) {
			ConfigLocation conf;
			conf.SetRoot(GetRoot());
			FillConfigLocation(conf, locationChunk, path);
			_locations.push_back(conf);
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
	(void) locationChunk;
	config.SetPath(path);
	// char	iD = GetIdentifierLocation(locationChunk);
	std::cout << "\nDEBUG START\n" << std::endl;
	for (unsigned int i = 1; i < locationChunk.size(); i++) {
		// size_t	end = locationChunk[i].find_first_of(" \t");
		// std::string token = locationChunk[i].substr(0, end);
		std::cout << config.GetPath() << std::endl;
		// std::cout << "DEBUG TOKEN: " << token << " END" << std::endl;
	}
	std::cout << "\nDEBUG END\n" << std::endl;
	return ;
}

static ssize_t	StartLocation(std::vector<std::string> &serverChunk, size_t index)
{
	unsigned int	i = index;
	std::string::size_type	n;
	n = serverChunk[i].find("location");

	while (n == std::string::npos && i < serverChunk.size()){
		i++;
		n = serverChunk[i].find("location");
	}
	if (i == serverChunk.size())
		return (-1);
	return (i);
}

static void	ServerPart(std::vector<std::string> &serverBloc, ConfigServer &config)
{
	std::set<char> checkDouble;
	for (unsigned int i = 0; i < serverBloc.size(); i++) {
		if (serverBloc[i][0] == '#')
			continue ;
		char id = GetIdentifier(serverBloc[i]);
		switch (id) {
			case 'L':	// Listen port
				if (checkDouble.count('L') > 0)
					throw (std::invalid_argument("Error: More than one 'Listen' instruction find."));
				checkDouble.insert('L');
				config.SetConfigServer(serverBloc[i], 'L');
				break ;
			case 'S':
				if (checkDouble.count('S') > 0)
					throw (std::invalid_argument("Error: More than one 'Server' instruction find."));
				checkDouble.insert('S');
				config.SetConfigServer(serverBloc[i], 'S');
				break ;
			case 'R':
				if (checkDouble.count('R') > 0)
					throw (std::invalid_argument("Error: More than one 'Root' instruction find."));
				checkDouble.insert('R');
				config.SetConfigServer(serverBloc[i], 'R');
				break ;
			case 'I':
				if (checkDouble.count('I') > 0)
					throw (std::invalid_argument("Error: More than one 'Index' instruction find."));
				checkDouble.insert('S');
				config.SetConfigServer(serverBloc[i], 'I');
				break ;
			case 'C':
				if (checkDouble.count('C') > 0)
					throw (std::invalid_argument("Error: More than one 'Max body client' instruction find."));
				checkDouble.insert('C');
				config.SetConfigServer(serverBloc[i], 'C');
				break ;
			case 'E':
				config.SetConfigServer(serverBloc[i], 'E');
				break ;
			case 'l':	// Location
				i = SkipLocation(serverBloc, i);
				break ;
			case 's':
				break ;
			default:
				std::cerr << "Error: Invalide syntaxe line: " << serverBloc[i];
				throw (std::invalid_argument("."));
				break ;
			}
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

void	ConfigServer::SetConfigServer(std::string &str, char iD)
{
	size_t	start = str.find_first_of(" \t");
	size_t	end = str.find(';');
	std::string value = str.substr(start, end - start);
	ClearSpace(value);
	switch (iD) {
		case 'L':
			SetPort(std::atoi(value.c_str()));
			break ;
		case 'S':
			SetServerName(value);
			break ;
		case 'R':
			SetRoot(value);
			break ;
		case 'I':
			SetIndex(value);
			break ;
		case 'C':
			SetMaxBodySize(std::atoi(value.c_str()));
			break ;
		case 'E':
			SetErrorPages(value);
			break ;
		default:
			break;
	}
}

static char	GetIdentifier(std::string &str)
{
	std::string token = str.substr(0, str.find_first_of(" \t"));
	if (token == "listen")
		return ('L');
	if (token == "server_name")
		return ('S');
	if (token == "root")
		return ('R');
	if (token == "index")
		return ('I');
	if (token == "client_max_body_size")
		return ('C');
	if (token == "error_page")
		return ('E');
	if (token == "location")
		return ('l');
	if (token == "server" || token == "{" || token == "}")
		return ('s');
	return ('n');
}
