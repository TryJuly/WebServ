/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 09:13:27 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/stat.h>
// # include <sys/event.h>	// MacOS
// # include <sys/epoll.h>	// Linux
# include <iostream>
# include <unistd.h>
# include <errno.h>
# include <vector>
# include <poll.h>
# include <cstdlib>	// std::atoi
# include <cstring>	// strncmp
# include "ConfigServer.hpp"
# include "Client.hpp"
# include <set>

# define MAX_EVENTS 10

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
		void	AcceptClient(int index);
		bool	IsSocketServer(int fd);

		/*	Set Private Attribute	*/
		void	SetFdServer(int fd);
		void	SetNumberConfig(int number);
		void	SetClient(Client const &client);
		void	SetSockAddr(struct sockaddr_in sockaddr);
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