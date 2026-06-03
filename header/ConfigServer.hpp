/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:51:30 by strieste          #+#    #+#             */
/*   Updated: 2026/06/03 09:16:51 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGSERVER_HPP
# define CONFIGSERVER_HPP

# include <map>
# include <set>
# include <string>
# include <vector>
# include <cstdlib>	// std::atoi
# include <iostream>
# include <exception>
# include <netinet/in.h>
# include <sys/socket.h>
# include "ConfigLocation.hpp"

# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class ConfigServer
{
	public:
		ConfigServer();
		// ConfigServer(std::string const &file);
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
		std::string	_index;
		std::string	_rootPath;
		std::string	_serverName;
		struct sockaddr_in	_sockAddress;
		std::set<int>	_checkDoubleError;
		std::vector<ConfigLocation>	_locations;
		std::map<int, std::string>	_errorPages;
};

void	ClearSpace(std::string &str);
int	EndChunk(std::vector<std::string> &fileArray, unsigned int index);

#endif