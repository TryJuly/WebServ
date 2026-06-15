/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/06/15 15:25:31 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <set>
# include <poll.h>
# include <string>
# include <vector>
# include <errno.h>
# include <cstdlib>	// std::atoi
# include <cstring>	// strncmp
# include <unistd.h>
# include <iostream>
# include <sys/stat.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include "Client.hpp"
# include "ConfigServer.hpp"
# include "CGI.hpp"
#include "Request.hpp"
#include <fcntl.h>

# define MAX_EVENTS 200
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
		Server(int ac, char **av);
		Server(Server const &copy);
		~Server();
		Server&	operator=(Server const &copy);

		/*	Set Up Config Server	*/
		void	SetUpServer();
		void	CleanSetError();
		void	CheckConfigServer();
		void	ParseConfig(std::vector<std::string> &fileArray);

		/*	Starting Part	*/
		void	StopServer();
		void	StartServer();
		bool	IsSocketServer(int fd);
		void	AcceptClient(int fd, int idClient);

		/*	Set Private Attribute	*/
		void	SetNumberConfig(int number);
		void	SetClient(Client const &client);
		void	SetConfigServer(ConfigServer const &config);

		/*	Get Private Attribute	*/
		int	GetIndexClient(int fd);
		int	GetNumberConfig( void );
		ConfigServer	&GetConfigServer( int index);
		bool	IsCgiEvent(int fd);
		int 	GetClientByPipe(int fd);
		void	HandleCgiRequest(ConfigServer &config, Request const &req, int indexClient);
		
		void	SendCgiResponse(int i);
		void	CheckTimeoutClient( void );
		void	CatchClientRequest(int i, int &NbClient);

	private:
		int	_numberConfig;
		int	_numberClient;
		std::vector<Client>	_client;
		std::vector<struct pollfd>	_fds;
		std::vector<ConfigServer>	_configServer;

};

void	ClearSpace(std::string &str);
int	EndChunk(std::vector<std::string> &fileArray, unsigned int index);

#endif