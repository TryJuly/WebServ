/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLocation.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 13:20:16 by strieste          #+#    #+#             */
/*   Updated: 2026/06/22 09:42:47 by seully           ###   ########.fr       */
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

# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class ConfigLocation
{
	public:
		/*	Default	*/
		ConfigLocation();
		~ConfigLocation();
		ConfigLocation(ConfigLocation const &copy);
		ConfigLocation&	operator=(ConfigLocation const &copy);

		/*	Getter	*/
		std::string	GetPath() const;
		std::string	GetRoot() const;
		std::string	Getindex() const;
		std::string	GetRedir() const;
		std::string	GetUpload() const;
		std::string	GetCGI(std::string key) const;
		std::map<std::string, std::string> &GetCGIMap();

		bool	GetAutoIndex();
		bool	GetBoolGet() const;
		bool	GetBoolPost() const;
		bool	GetBoolDelete() const;

		/*	Setter	*/
		void	SetPath(std::string const &str);
		void	SetRoot(std::string const &str);
		void	Setindex(std::string const &str);
		void	SetAutoIndex(std::string &value);
		void	SetCGI(std::string const &value);
		void	SetRedir(std::string const &str);
		void	SetUpload(std::string const &str);
		void	SetMethodes(std::string const &str);

	private:
		bool	_get;
		bool	_post;
		bool	_delete;
		bool	_autoindex;

		std::string	_path;
		std::string	_root;
		std::string	_index;
		std::string	_redir;
		std::string	_uploadPath;
		std::map<std::string, std::string>	_cgi;

};

void	ClearSpace(std::string &str);

#endif