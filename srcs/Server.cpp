/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/05/14 17:07:55 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server()
{
	_port = 8080;
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw(std::invalid_argument("Error: Socket server."));
	_sockAddress.sin_family = AF_INET;
	_sockAddress.sin_port = htons(_port);
	_sockAddress.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
	if (bind(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), sizeof(_sockAddress)) != 0)
		throw(std::invalid_argument("Error: Bind server."));
	if (listen(_socket, 0) != 0) // Need to change connection max
		throw(std::invalid_argument("Error: Listen server."));
	_kq = kqueue(); // epoll_creat()
	if (_kq == -1)
		throw(std::invalid_argument("Error: Kqueue server."));
	EV_SET(&_changeList, _socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(_kq, &_changeList, 1, NULL, 0, NULL);
}

Server::Server(Server const &copy)
{
	(*this) = copy;
	return;
}

Server::~Server()
{
}

Server &Server::operator=(Server const &copy)
{
	(void)copy;
	return (*this);
}

void Server::StartServer()
{
	while (true) {
		kevent(_kq, NULL, 0, &_evenList, 1, 0);
		if (_evenList.ident == static_cast<unsigned long>(_socket)) {
			std::cout << "Recu " << _evenList.ident << " " << _socket << " " << _evenList.flags << " " << _evenList.filter << std::endl;
			socklen_t addrLen = sizeof(_sockAddress);
			int newSocket = accept(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), &addrLen);
			std::cout << "new client fd: " << newSocket << std::endl;
			EV_SET(&_changeList, newSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
			kevent(_kq, &_changeList, 1, NULL, 0, NULL);
		}
		else {
		}
	}
}

void Server::StopServer()
{
}
