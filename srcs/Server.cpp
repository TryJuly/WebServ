/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/05/18 10:36:00 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Server.hpp"

Server::Server()
{
	_port = 8080;
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw(std::invalid_argument("Error: Socket server."));
	_sockAddress.sin_family = AF_INET;
	_sockAddress.sin_port = htons(_port);
	_sockAddress.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
	if (bind(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), sizeof(_sockAddress)) != 0) {
		std::cout << errno << " " << _socket << std::endl;
		throw(std::invalid_argument("Error: Bind server."));
	}
	if (listen(_socket, 0) != 0) // Need to change connection max
		throw(std::invalid_argument("Error: Listen server."));
	_epFd = epoll_create(1);	// the size number is ignored 
	// _kq = kqueue(); // epoll_creat()	MacOS
	if (_epFd == -1)
		throw(std::invalid_argument("Error: Kqueue Server."));
	_event.events = EPOLLIN | EPOLLET;
	_event.data.fd = _socket;
	if (epoll_ctl(_epFd, EPOLL_CTL_ADD, _socket, &_event) == -1) {
		std::cout << _epFd << std::endl;
		throw(std::invalid_argument("Error: Epoll_ctl Server."));
	}
	// EV_SET(&_changeList, _socket, EVFILT_READ, EV_ADD, 0, 0, NULL);	MacOS
	// kevent(_kq, &_changeList, 1, NULL, 0, NULL);	MacOS
	// _eventList.data.fd
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
		std::cout << "wait" << std::endl;
		// kevent(_kq, NULL, 0, &_evenList, 1, 0);	MacOS
		int nfds = epoll_wait(_epFd, _eventList, MAX_EVENTS, -1);	// -1 wait unlimited
		// if (epoll_ctl(_epFd, EPOLL_CTL_ADD, _socket, &_eventList) != 0)
			// throw(std::invalid_argument("Error: Epoll_ctl Server."));
		for (int i = 0; i < nfds; i++) {
			if (_eventList[i].events & EPOLLIN) {
				std::cout << _eventList[i].data.fd << std::endl;
				// if (_eventList[i].data.fd == _socket) {
				std::cout << "Recu " << _event.data.fd << " socket: " << _socket << " event: " << _event.events << " data ptr: " << _event.data.ptr << std::endl;
				socklen_t addrLen = sizeof(_sockAddress);
				int newSocket = accept(_event.data.fd, reinterpret_cast<struct sockaddr *>(&_sockAddress), &addrLen);
				if (epoll_ctl(_epFd, EPOLL_CTL_ADD, newSocket, &_event) == -1)
				std::cout << _epFd << std::endl;
				std::cout << "new client fd: " << newSocket << std::endl;
			// for (int j = 0; j < MAX_EVENTS; j++)
			// std::cout << j << " : " << _eventList[j].data.fd << std::endl;
			// EV_SET(&_changeList, newSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);	MacOS
			// kevent(_kq, &_changeList, 1, NULL, 0, NULL);	MacOS
			}
			else {
				std::cout << "Hello from Nigeria ! " << _event.data.fd << std::endl;
				char buff[1024];
				read(_event.data.fd, buff, sizeof(buff));
				// if (buff[0] == '\0')
				// 	return ;
				std::cout << "Client message: " << buff << std::endl;
				write(_event.data.fd, "HTTP/1.1 200 OK\r\nContent-Length:13\r\n\r\nHello, world!", 52);
			}
		}
		
	}
}

void Server::StopServer()
{
}
