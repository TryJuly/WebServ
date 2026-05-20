/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/05/20 09:16:42 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
// # include <sys/event.h>	// MacOS
# include <sys/epoll.h>	// Linux
# include <iostream>
# include <unistd.h>
# include <errno.h>

# define MAX_EVENTS 10

class Server
{
	public:
		Server();
		Server(Server const &copy);
		~Server();
		Server&	operator=(Server const &copy);
		
		void	StartServer();
		void	StopServer();

	private:
		int	_socket;
		int	_port;
		// int _kq;
		int _fdServer;
		// std::string	_ipAddress;
		struct sockaddr_in	_sockAddress;

		// struct epoll_event	_event;
		// struct epoll_event	_eventList;
		// struct kevent _changeList;	MacOS
		// struct kevent _evenList;		MacOS
};

#endif