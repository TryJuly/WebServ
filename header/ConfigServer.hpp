/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:51:30 by strieste          #+#    #+#             */
/*   Updated: 2026/05/25 17:50:17 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

# include <string>
# include <map>
# include <vector>
# include "ConfigLocation.hpp"

class ConfigServer
{
	public:
		ConfigServer();
		ConfigServer(std::string const &file);
		ConfigServer(ConfigServer const &copy);
		~ConfigServer();
		ConfigServer&	operator=(ConfigServer const &copy);

		int	GetPort();
		int	GetSocket();
		int	GetMaxBodySize();
		std::string&	GetRoot();
		std::string&	GetIndex();
		std::string&	GetServerName();
		std::string&	GetErrorPages(int number);
		ConfigLocation&	GetConfigLocation(int index);

		void	SetPort(int port);
		void	SetSocket(int socket);
		void	SetMaxBodySize(int BodySize);
		void	SetRoot(std::string const &rootPath);
		void	SetIndex(std::string const &index);
		void	SetServerName(std::string const &ServerName);
		void	SetErrorPages(int number, std::string path);
		void	SetConfigLocation(ConfigLocation const &config);

	private:
		int	_port;
		int	_socket;
		int	_maxBodySize;
		int	_numberLocation;
		std::string	_rootPath;
		std::string	_index;
		std::string	_serverName;
		std::map<int, std::string>	_errorPages;
		std::vector<ConfigLocation>	_locations;
};

#endif