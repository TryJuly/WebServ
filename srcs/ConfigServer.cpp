/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 08:36:43 by strieste          #+#    #+#             */
/*   Updated: 2026/05/26 13:04:26 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/ConfigServer.hpp"

ConfigServer::ConfigServer()
{
	_port = 8080;
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw(std::invalid_argument("Error: Socket server default config."));
	_maxBodySize = 1048576;
	_rootPath = "var/www/html";
	_index = "index.html";
	_serverName = "localhost";
	_errorPages.insert(std::pair<int, std::string>(404, "/errors/404.html"));
	_errorPages.insert(std::pair<int, std::string>(500, "/errors/500.html"));

	ConfigLocation config;
	config.SetPath("/");
	config.SetMethodes("G");
	_locations.push_back(config);

	ConfigLocation configTwo;
	configTwo.SetPath("/upload");
	configTwo.SetMethodes("GP");
	configTwo.SetUpload("/var/www/uploads");
	_locations.push_back(configTwo);

	ConfigLocation configThree;
	configThree.SetPath("/old");
	configThree.SetRedir("/new");
	_locations.push_back(configThree);

	ConfigLocation configFour;
	configFour.SetPath("/cgi-bin");
	configFour.SetMethodes("GP");
	configFour.SetCGI(".py", "/usr/bin/python3");
	_locations.push_back(configFour);

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

std::string&	ConfigServer::GetErrorPages(int number)
{
	std::map<int, std::string>::iterator it;
	it = _errorPages.find(number);
	return (it->second);
}
ConfigLocation&	ConfigServer::GetConfigLocation(int index)
{ return (_locations[index]); }

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

void	ConfigServer::SetErrorPages(int number, std::string path)
{
	_errorPages.insert(std::pair<int, std::string>(number, path));
	return ;
}

void	ConfigServer::SetConfigLocation(ConfigLocation const &config)
{
	_locations.push_back(config);
	return ;
}
