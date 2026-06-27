/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/06/27 11:15:56 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <set>
# include <poll.h>
# include <string>
# include <vector>
# include <fcntl.h>
# include <errno.h>
# include <cstdlib>	// std::atoi
# include <cstring>	// strncmp
# include <unistd.h>
# include <iostream>
# include <sys/stat.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include "CGI.hpp"
# include "Client.hpp"
# include "Request.hpp"
# include "ConfigServer.hpp"

// # define MAX_EVENTS 200
# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class Server
{
	public:
		/*	Default	*/
		Server();
		~Server();
		Server(int ac, char **av);
		Server(Server const &copy);
		Server&	operator=(Server const &copy);

		/*	Set Up Config Server	*/
		void	SetUpServer( void );
		void	CleanSetError( void );
		void	CheckConfigServer( void );
		void	ParseConfig(std::vector<std::string> &fileArray);

		/*	Starting Part	*/
		void	StopServer( void );
		void	StartServer( void );
		bool	IsSocketServer(int fd);
		void	AcceptClient(int fd, int idClient);

		/*	Set Private Attribute	*/
		void	SetNumberConfig(int number);
		void	SetClient(Client const &client);
		void	SetConfigServer(ConfigServer const &config);

		/*	Get Private Attribute	*/
		int	GetIndexClient(int fd);
		int	GetNumberConfig( void );
		int	GetClientByPipe(int fd);
		ConfigServer	&GetConfigServer( int index);

		/*	Function	*/
		bool	IsCgiEvent(int fd);
		bool	IsCgiStdinEvent(int fd);
		void	SendCgiBody(int i);
		void	SendCgiResponse(int i);
		void	CheckTimeoutClient( void );
		int		GetClientByPipeIn(int fd);
		void	WriteClientBuffer(int i, int &NbClient);
		void	CatchClientRequest(int i, int &NbClient);
		int		FindClientByCookie(std::string const &cookie);
		void	HandleCgiRequest(ConfigServer &config, Request const &req, int indexClient);

	private:
		int	_numberConfig;
		int	_numberClient;
		std::vector<Client>	_client;
		std::vector<struct pollfd>	_fds;
		std::vector<ConfigServer>	_configServer;
		std::map<int, std::vector<int> >	_socketToConfigs;

};

void	ClearSpace(std::string &str);
int	EndChunk(std::vector<std::string> &fileArray, unsigned int index);

#endif