/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/05/26 13:12:22 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/stat.h>
// # include <sys/event.h>	// MacOS
# include <sys/epoll.h>	// Linux
# include <iostream>
# include <unistd.h>
# include <errno.h>
# include <vector>
# include "ConfigServer.hpp"
# include "Client.hpp"

# define MAX_EVENTS 10

class Server
{
	public:
		Server();
		// Server(char **av);
		Server(Server const &copy);
		~Server();
		Server&	operator=(Server const &copy);

		void	StartServer();
		void	StopServer();

	private:
		int _fdServer;
		int	_numberConfig;
		std::vector<ConfigServer>	_configServer;
		std::vector<Client>	_client;

		// int	_socket;
		// int	_port;
		struct sockaddr_in	_sockAddress;

		// struct epoll_event	_event;
		// struct epoll_event	_eventList;
		// struct kevent _changeList;	MacOS
		// struct kevent _evenList;		MacOS
};

#endif