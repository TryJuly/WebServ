/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLocation.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 13:20:16 by strieste          #+#    #+#             */
/*   Updated: 2026/05/25 17:50:03 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOCATION_HPP
# define CONFIGLOCATION_HPP

# include <string>
# include <vector>
# include <map>

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
		std::string&	GetMethodes(int index);
		std::string&	GetCGI(std::string key);

		void	SetPath(std::string const &str);
		void	SetRoot(std::string const &str);
		void	Setindex(std::string const &str);
		void	SetRedir(std::string const &str);
		void	SetUpload(std::string const &str);
		void	GetMethodes(std::string const &str);
		void	GetCGI(std::string const &key, std::string const &value);

	private:
		std::string	_path;
		std::string	_root;
		std::string	_index;
		// bool		_autoindex;
		std::string	_redir;	// 301
		std::string	_uploadPath;	// PATH UPLOAD
		std::vector<std::string>	_methode;	// GET POST DELETE
		std::map<std::string, std::string>	_cgi;	// PATH CGI
};

#endif