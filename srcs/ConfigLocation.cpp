/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLocation.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cbezenco <cbezenco@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 09:35:48 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 11:25:12 by cbezenco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/ConfigLocation.hpp"

ConfigLocation::ConfigLocation()
{ return ; }

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
		_methode = copy._methode;
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


//	Change GET POST DELETE
std::string&	ConfigLocation::GetMethodes(int index)
{ return (_methode[index]); }

std::string&	ConfigLocation::GetCGI(std::string key)
{
	std::map<std::string, std::string>::iterator it;
	it = _cgi.find(key);
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

void	ConfigLocation::SetRedir(std::string const &str)
{
	_redir = str;
	return ;
}

void	ConfigLocation::SetUpload(std::string const &str)
{
	_uploadPath = str;
	return ;
}

void	ConfigLocation::SetMethodes(std::string const &str)
{
	for (int i = 0; str[i]; i++) {
		if (str[i] == 'G')
			_methode.push_back("GET");
		else if (str[i] == 'P')
			_methode.push_back("POST");
		else if (str[i] == 'D')
			_methode.push_back("DELETE");
	}
	return ;
}

void	ConfigLocation::SetCGI(std::string const &key, std::string const &value)
{
	_cgi.insert(std::pair<std::string, std::string>(key, value));
	return ;
}

bool    ConfigLocation::GetBoolGet()
{ return (_get); }

bool    ConfigLocation::GetBoolPost()
{ return (_post); }

bool    ConfigLocation::GetBoolDelete()
{ return (_delete); }