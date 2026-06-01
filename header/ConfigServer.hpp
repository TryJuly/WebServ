/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:51:30 by strieste          #+#    #+#             */
/*   Updated: 2026/05/28 15:21:13 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

# include <string>
# include <map>
# include <vector>
# include <sys/socket.h>
# include <exception>
# include <iostream>
# include <set>
# include <cstdlib>	// std::atoi
# include <netinet/in.h>
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
		std::string		GetErrorPages(int number);
		ConfigLocation&	GetConfigLocation(int index);
		sockaddr_in&	GetSockAddr(void);
		int	GetNumberLocation(void);

		void	FillConfigServer(std::vector<std::string> &serverChunk);
		void	SetConfigServer(std::string &str, char iD);
		void	CleanSetError();
		int		FindLocationPath(std::string &path);
		void	AddConfigLocation(ConfigLocation &config);

		void	SetPort(int port);
		void	SetSocket(int socket);
		void	SetMaxBodySize(int BodySize);
		void	SetRoot(std::string const &rootPath);
		void	SetIndex(std::string const &index);
		void	SetServerName(std::string const &ServerName);
		void	SetErrorPages(std::string errorPage);
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
		std::set<int>	_checkDoubleError;
		struct sockaddr_in	_sockAddress;
};

void	ClearSpace(std::string &str);
int	EndChunk(std::vector<std::string> &fileArray, unsigned int index);

#endif