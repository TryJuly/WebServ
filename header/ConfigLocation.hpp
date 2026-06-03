/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLocation.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 13:20:16 by strieste          #+#    #+#             */
/*   Updated: 2026/06/03 14:39:38 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOCATION_HPP
# define CONFIGLOCATION_HPP

# include <map>
# include <string>
# include <vector>
# include <sstream>
# include <iostream>
# include <exception>
# include <sys/stat.h>

class ConfigLocation
{
	public:
		ConfigLocation();
		ConfigLocation(ConfigLocation const &copy);
		~ConfigLocation();
		ConfigLocation&	operator=(ConfigLocation const &copy);

		std::string&	GetPath();
		std::string&	GetRoot();
		std::string&	Getindex();
		std::string&	GetRedir();
		std::string&	GetUpload();
		bool			GetAutoIndex();
		// std::string&	GetMethodes(int index);
		std::string&	GetCGI(std::string key);
		bool			GetBoolGet();
		bool			GetBoolPost();
		bool			GetBoolDelete();
		
		std::map<std::string, std::string> &GetCGIMap();


		void	SetPath(std::string const &str);
		void	SetRoot(std::string const &str);
		void	Setindex(std::string const &str);
		void	SetAutoIndex(std::string &value);
		void	SetRedir(std::string const &str);
		void	SetUpload(std::string const &str);
		void	SetMethodes(std::string const &str);
		void	SetCGI(std::string const &value);
		// void	SetBoolGet( void);
		// void	SetBoolPost( void);
		// void	SetBoolDelete( void);

	private:
		std::string	_path;
		std::string	_root;
		std::string	_index;
		bool		_autoindex;
		int		_get;
		int		_post;
		int		_delete;
		std::string	_redir;	// 301
		std::string	_uploadPath;	// PATH UPLOAD
		// std::vector<std::string>	_methode;	// GET POST DELETE
		std::map<std::string, std::string>	_cgi;	// PATH CGI
};

void	ClearSpace(std::string &str);

#endif