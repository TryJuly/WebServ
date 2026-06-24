/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:51:30 by strieste          #+#    #+#             */
/*   Updated: 2026/06/22 09:32:02 by seully           ###   ########.fr       */
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
		/*	Default	*/
		ConfigServer();
		~ConfigServer();
		ConfigServer(ConfigServer const &copy);
		ConfigServer&	operator=(ConfigServer const &copy);

		/*	Getter	*/

		int	GetPort( void );
		int	GetSocket( void );
		int	GetMaxBodySize( void );
		int	GetNumberLocation(void);

		std::string&	GetRoot( void );
		std::string&	GetIndex( void );
		sockaddr_in&	GetSockAddr(void);
		std::string		GetPortStr( void );
		std::string&	GetServerName( void );
		std::string		GetErrorPages(int number);
		ConfigLocation&	GetConfigLocation(int index);
		std::map<int, std::string>&	GetMapError( void );

		/*	Setter	*/

		void	SetPort(int port);
		void	SetSocket(int socket);
		void	SetPortStr(std::string port);
		void	SetMaxBodySize(int BodySize);
		void	SetIndex(std::string const &index);
		void	SetRoot(std::string const &rootPath);
		void	SetErrorPages(std::string errorPage);
		void	SetServerName(std::string const &ServerName);
		void	SetConfigLocation(ConfigLocation const &config);

		/*	Function	*/
		void	CleanSetError( void );
		int		FindLocationPath(std::string const &path);
		void	AddConfigLocation(ConfigLocation &config);
		void	SetConfigServer(std::string &str, char iD);
		void	FillConfigServer(std::vector<std::string> &serverChunk);

	private:
		int	_port;
		int	_socket;
		int	_maxBodySize;
		std::string	_index;
		int	_numberLocation;
		std::string	_portStr;
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