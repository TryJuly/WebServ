/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/06/03 08:55:18 by strieste         ###   ########.fr       */
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

# define MAX_EVENTS 10
# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class Server
{
	public:
		/*	Default	*/
		Server(int ac, char **av);
		Server(Server const &copy);
		~Server();
		Server&	operator=(Server const &copy);

		/*	Set Up Config Server	*/
		void	SetUpServer();
		void	CheckConfigServer();
		void	ParseConfig(std::vector<std::string> &fileArray);
		void	CleanSetError();

		/*	Starting Part	*/
		void	StopServer();
		void	StartServer();
		// void	AcceptClient(int index);
		bool	IsSocketServer(int fd);

		/*	Set Private Attribute	*/
		// void	SetFdServer(int fd);
		void	SetNumberConfig(int number);
		void	SetClient(Client const &client);
		// void	SetSockAddr(struct sockaddr_in sockaddr);
		void	SetConfigServer(ConfigServer const &config);

		/*	Get Private Attribute	*/
		int	GetNumberConfig( void );
		ConfigServer&	GetConfigServer( int index);

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