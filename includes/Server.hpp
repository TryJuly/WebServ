/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:34:40 by strieste          #+#    #+#             */
/*   Updated: 2026/05/14 16:36:29 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/event.h>
# include <iostream>

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
		int _kq;
		// std::string	_ipAddress;
		struct sockaddr_in	_sockAddress;

		struct kevent _changeList;
		struct kevent _evenList;
};

#endif