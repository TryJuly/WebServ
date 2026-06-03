/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLocation.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cbezenco <cbezenco@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:35:48 by strieste          #+#    #+#             */
/*   Updated: 2026/06/02 11:23:01 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/ConfigLocation.hpp"

ConfigLocation::ConfigLocation()
{
	_get = false;
	_post = false;
	_delete = false;
	_autoindex = false;
	return ;
}

ConfigLocation::ConfigLocation(ConfigLocation const &copy)
{
	(*this) = copy;
	return ;
}

ConfigLocation::~ConfigLocation()
{ return ; }

ConfigLocation&	ConfigLocation::operator=(ConfigLocation const &copy)
{
	if (this != &copy) {
		_cgi = copy._cgi;
		_path = copy._path;
		_root = copy._root;
		_index = copy._index;
		_redir = copy._redir;
		_get = copy._get;
		_post = copy._post;
		_delete = copy._delete;
		_autoindex = copy._autoindex;
		_uploadPath = copy._uploadPath;
	}
	return (*this);
}

std::string&	ConfigLocation::GetPath()
{ return (_path); }

std::string&	ConfigLocation::GetRoot()
{ return (_root); }

std::string&	ConfigLocation::Getindex()
{ return (_index); }

std::string&	ConfigLocation::GetRedir()
{ return (_redir); }

std::string&	ConfigLocation::GetUpload()
{ return (_uploadPath); }

bool	ConfigLocation::GetAutoIndex()
{ return (_autoindex); }

std::string&	ConfigLocation::GetCGI(std::string key)
{
	std::map<std::string, std::string>::iterator it = _cgi.find(key);
	if (it == _cgi.end())
		throw (std::out_of_range("Error: CGI key not found: " + key));
	return (it->second);
}

void	ConfigLocation::SetPath(std::string const &str)
{
	_path = str;
	return ;
}

void	ConfigLocation::SetRoot(std::string const &str)
{
	_root = str;
	return ;
}

void	ConfigLocation::Setindex(std::string const &str)
{
	_index = str;
	return ;
}

void	ConfigLocation::SetAutoIndex(std::string &value)
{
	if (!value.compare("on"))
		_autoindex = true;
	else if (value == "off")
		_autoindex = false;
	else
		throw (std::invalid_argument("Error: Value of autoindex is different from on and off."));
	return ;
}

void	ConfigLocation::SetRedir(std::string const &str)
{
	size_t	endKey = str.find_first_of(" \t");
	if (endKey == std::string::npos)
			throw (std::invalid_argument("Error: Invalid syntaxe line: " + str));
	std::string val = str.substr(endKey + 1, str.size() - endKey);
	_redir = val;
	return ;
}

void	ConfigLocation::SetUpload(std::string const &str)
{
	_uploadPath = str;
	return ;
}

// void	ConfigLocation::SetBoolGet( void )
// {
// 	_get = true;
// 	return ;
// }

// void	ConfigLocation::SetBoolPost( void )
// {
// 	_post = true;
// 	return ;
// }
// void	ConfigLocation::SetBoolDelete( void )
// {
// 	_delete = true;
// 	return ;
// }

void	ConfigLocation::SetMethodes(std::string const &str)
{
	std::string	word;
	std::stringstream ss(str);
	std::vector<std::string> v;

	while (std::getline(ss, word, ' '))
		v.push_back(word);

	for (size_t i = 0; i < v.size(); i++) {
		ClearSpace(v[i]);
		if (!v[i].compare("GET"))
			_get = true;
		else if (!v[i].compare("POST"))
			_post = true;
		else if (!v[i].compare("DELETE"))
			_delete = true;
		else
			throw (std::invalid_argument("Error: Invalide syntax methode: " + str));
	}
	return ;
}

void	ConfigLocation::SetCGI(std::string const &value)
{
	size_t	endKey = value.find_first_of(" \t");
	if (endKey == std::string::npos)
			throw (std::invalid_argument("Error: Invalid syntaxe line555: " + value));

	std::string key = value.substr(0, endKey);
	std::string val = value.substr(endKey + 1, value.size() - endKey);
	_cgi.insert(std::pair<std::string, std::string>(key, val));
	return ;
}

bool    ConfigLocation::GetBoolGet()
{ return (_get); }

bool    ConfigLocation::GetBoolPost()
{ return (_post); }

bool    ConfigLocation::GetBoolDelete()
{ return (_delete); }

std::map<std::string, std::string> &ConfigLocation::GetCGIMap()
{return (_cgi); }
