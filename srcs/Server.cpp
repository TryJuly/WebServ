/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/05/18 15:57:50 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Server.hpp"
#include <iostream>

//	Linux version
// Server::Server()
// {
// 	_port = 8080;
// 	_socket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (_socket == -1)
// 		throw(std::invalid_argument("Error: Socket server."));
// 	_sockAddress.sin_family = AF_INET;
// 	_sockAddress.sin_port = htons(_port);
// 	_sockAddress.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
// 	if (bind(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), sizeof(_sockAddress)) != 0) {
// 		std::cout << errno << " " << _socket << std::endl;
// 		throw(std::invalid_argument("Error: Bind server."));
// 	}
// 	if (listen(_socket, 0) != 0) // Need to change connection max
// 		throw(std::invalid_argument("Error: Listen server."));
// 	_epFd = epoll_create(1);	// the size number is ignored 
// 	if (_epFd == -1)
// 		throw(std::invalid_argument("Error: Kqueue Server."));
// 	struct epoll_event	eventServer;
// 	eventServer.events = EPOLLIN | EPOLLET;
// 	eventServer.data.fd = _socket;
// 	if (epoll_ctl(_epFd, EPOLL_CTL_ADD, _socket, &eventServer) == -1) {
// 		std::cout << _epFd << std::endl;
// 		throw(std::invalid_argument("Error: Epoll_ctl Server."));
// 	}
// }

// //	Linux version
// void Server::StartServer()
// {
// 	struct epoll_event	changeList[MAX_EVENTS];
// 	while (true) {
// 		std::cout << "wait" << std::endl;
// 		int nfds = epoll_wait(_epFd, changeList, MAX_EVENTS, -1);	// -1 wait unlimited
// 		for (int i = 0; i < nfds; i++) {
// 			if (changeList[i].data.fd == _socket) {
// 				std::cout << "Recu " << changeList[i].data.fd << " socket: " << _socket << " event: " << changeList[i].events << " data ptr: " << changeList[i].data.ptr << std::endl;
// 				socklen_t addrLen = sizeof(_sockAddress);
// 				int socketClient = accept(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), &addrLen);
// 				struct epoll_event clientEvent;
// 				clientEvent.events = EPOLLIN | EPOLLET;
// 				clientEvent.data.fd = socketClient;
// 				epoll_ctl(_epFd, EPOLL_CTL_ADD, socketClient, &clientEvent);
// 			}
// 			else {
// 				std::cout << "Hello from Nigeria ! " << changeList[i].data.fd << std::endl;
// 				char buff[1024];
// 				read(changeList[i].data.fd, buff, sizeof(buff));
// 				// if (buff[0] == '\0')
// 				// 	return ;
// 				std::cout << "Client message: " << buff << std::endl;
// 				write(changeList[i].data.fd, "HTTP/1.1 200 OK\r\nContent-Length:13\r\n\r\nHello, world!", 52);
// 			}
// 		}
		
// 	}
// }

/*			MacOS version			*/
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
	_fdServer = kqueue();
	if (_fdServer == -1)
		throw(std::invalid_argument("Error: Kqueue Server."));
	struct kevent changeList;
	EV_SET(&changeList, _socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(_fdServer, &changeList, 1, NULL, 0, NULL);
}

/*			MacOS version			*/
void Server::StartServer()
{
	struct kevent eventList[MAX_EVENTS];
	while (true) {
		int nfds = kevent(_fdServer, NULL, 0, eventList, MAX_EVENTS, NULL);
		for (int i = 0; i < nfds; i++) {
			if (eventList[i].ident == static_cast<unsigned long>(_socket)) {
				// std::cout << "Recu " << _event.data.fd << " socket: " << _socket << " event: " << _event.events << " data ptr: " << _event.data.ptr << std::endl;
				socklen_t addrLen = sizeof(_sockAddress);
				int newSocket = accept(_socket, reinterpret_cast<struct sockaddr *>(&_sockAddress), &addrLen);
				struct kevent eventClient;
				EV_SET(&eventClient, newSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(_fdServer, &eventClient, 1, NULL, 0, NULL);
				std::cout << "new client fd: " << newSocket << std::endl;
			}
			else {
				char buff[1024];
				if (read(eventList[i].ident, buff, sizeof(buff)) == 0)
					close(eventList[i].ident);
				std::cout << "Client message: " << buff << "End" << std::endl;
				write(eventList[i].ident, "HTTP/1.1 200 OK\r\nContent-Length:13\r\n\r\nHello, world!", 52);
			}
		}
	}
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

void Server::StopServer()
{
}
