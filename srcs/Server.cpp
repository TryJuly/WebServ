/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cbezenco <cbezenco@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 09:56:16 by cbezenco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Server.hpp"
#include <iostream>
#include <fstream>
#include "../header/Request.hpp"
#include "../header/Response.hpp"

// static char	GetIdentifier(std::string &str);
static int	IsBalance(std::vector<std::string> &fileArray);
static int	IsValideBloc(std::vector<std::string> &serverBloc);
static int	FindIndexServerFD(Server &server, int fd);
// static size_t	Countlocation(std::vector<std::string> &serverChunk);
// static void	ServerPart(std::vector<std::string> &serverBloc, Server &server);
// static void	LocationPart(std::vector<std::string> &serverChunk, Server &server);
// static int	EndChunk(std::vector<std::string> &fileArray, unsigned int index);
// static int	EndLocationBloc(std::vector<std::string> &serverChunk, unsigned int index);

Server::Server(int ac, char **av)
{
	std::string	fileName;

	if (ac == 2)
		fileName = av[1];
	else
		fileName = "default.cnf";
	struct stat	sStat;
	if (stat(fileName.c_str(), &sStat) == 0 && S_ISDIR(sStat.st_mode))
		throw (std::invalid_argument("Error: Is a directory."));
	std::ifstream	fd(fileName.c_str(), std::ios::in);
	if (!fd.is_open())
		throw (std::invalid_argument("Error: Open file."));
	std::vector<std::string>	fileArray;
	std::string	buff;
	_numberConfig = 0;
	while (!fd.eof()) {
		std::getline(fd, buff);
		ClearSpace(buff);
		if (!buff.empty())
			fileArray.push_back(buff);
	}
	fd.close();
	ParseConfig(fileArray);
	CleanSetError();
	CheckConfigServer();
	SetUpServer();
	return ;
}

void	Server::CleanSetError()
{
	int size = _configServer.size();
	for (int i = 0; i < size; i++)
		_configServer[i].CleanSetError();
	return ;
}

/*	Function Set up socket, fdserver bind, listen all server*/
void	Server::SetUpServer()
{
	for (unsigned int i = 0; i < _configServer.size(); i++) {
		ConfigServer	&config = _configServer[i];
		config.SetSocket(socket(AF_INET, SOCK_STREAM, 0));
		if (config.GetSocket() == -1)
			throw (std::runtime_error("Error: socket() failed: " + std::string(strerror(errno))));
		struct sockaddr_in	&sockAddr = config.GetSockAddr();
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(config.GetPort());
		sockAddr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
		if (bind(config.GetSocket(), reinterpret_cast<struct sockaddr *>(&sockAddr), sizeof(sockAddr)) != 0)
			throw (std::runtime_error("Error: bind() failed: " + std::string(strerror(errno))));
		if (listen(config.GetSocket(), 0) != 0) // Need to change connection max
			throw (std::runtime_error("Error: listen() failed: " + std::string(strerror(errno))));

		struct pollfd	serverPoll;
		serverPoll.fd = config.GetSocket();
		serverPoll.events = POLLIN;
		serverPoll.revents = 0;
		_fds.push_back(serverPoll);
	}
	return ;
}

/*	Function verify if the configuration parsing is valid*/
void	Server::CheckConfigServer()
{}

void	PrintConfig(Server &server)
{
	std::cout << "\nNumber config: " << server.GetNumberConfig() << std::endl;
	for (int i = 0; i < server.GetNumberConfig(); i++) {
		ConfigServer config;
		config = server.GetConfigServer(i);
		std::cout << "Port: :" << config.GetPort() << ":" << std::endl;
		std::cout << "Socket: " << config.GetSocket() << std::endl;
		std::cout << "Max Body: " << config.GetMaxBodySize() <<std::endl;
		std::cout << "Root path: " << config.GetRoot() << std::endl;
		std::cout << "Index: " << config.GetIndex() << std::endl;
		try {
			std::cout << "Error 400: " << config.GetErrorPages(400) << std::endl;
			std::cout << "Error 404: " << config.GetErrorPages(404) << std::endl;
			std::cout << "Error 500: " << config.GetErrorPages(500) << std::endl;
			std::cout << "Error 502: " << config.GetErrorPages(502) << std::endl;
			std::cout << "Error 503: " << config.GetErrorPages(503) << std::endl;
			std::cout << "Error 504: " << config.GetErrorPages(504) << std::endl;
		}
		catch(const std::exception& e) {
			std::cerr << "Error: Unset error_page value" << std::endl;;
		}
		std::cout << "\nEnd\n" << std::endl;
	}
}

void parseRequest(char * request) {
	std::cout << request << std::endl;
	std::string line;
	
}

/*	Linux version	*/
void	Server::AcceptClient(int index)
{
	int fdServer = FindIndexServerFD(*this, _fds[index].fd);
	if (fdServer == -1)
		throw (std::runtime_error("Error: Find FD serveur failed"));
	struct sockaddr_in	&clientAddr = _configServer[fdServer].GetSockAddr();
	socklen_t addrLen = sizeof(clientAddr);
	int socketClient = accept(_fds[index].fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);

	struct pollfd clientPoll;
	clientPoll.fd = socketClient;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	_fds.push_back(clientPoll);
	Client newClient(socketClient);
	newClient.SetIdClient(_numberClient++);
	_client.push_back(newClient);
	return ;
}

void Server::StartServer()
{
	int NbRequest = 0;
	int NbClient = 0;
	int	IdClient = 0;
	while (true) {
		// std::cout << "\n###	Wait Request Client	###\n" << std::endl;
		// int nfds = epoll_wait(_fdServer, changeList, MAX_EVENTS, -1);	// -1 wait unlimited
		// for (int i = 0; i < nfds; i++) {
		// 	for (int j = 0; j < _numberConfig; j++) {
		// 		if (changeList[i].data.fd == _configServer[j].GetSocket()) {
		// 			std::cout << "###	INFOS	###\n" << std::endl;
		// 			std::cout << "Recu " << changeList[i].data.fd << " socket: " << _configServer[0].GetSocket() << " event: " << changeList[i].events << " data ptr: " << changeList[i].data.ptr << std::endl;
		// 			std::cout << "\n###	FIN INFOS	###\n" << std::endl;
		// 			socklen_t addrLen = sizeof(_sockAddress);
		// 			int socketClient = accept(_configServer[0].GetSocket(), reinterpret_cast<struct sockaddr *>(&_sockAddress), &addrLen);
		// 			NbClient++;
		// 			struct epoll_event clientEvent;
		// 			clientEvent.events = EPOLLIN | EPOLLET;
		// 			clientEvent.data.fd = socketClient;
		// 			epoll_ctl(_fdServer, EPOLL_CTL_ADD, socketClient, &clientEvent);
		// 		}
		// 		else {
		// 			char buff[1024];
		// 			if (read(changeList[i].data.fd, buff, sizeof(buff)) == 0) {
		// 				close(changeList[i].data.fd);
		// 				NbClient--;
		// 			}
		// 			else {
		// 			std::cout << "###	Client Message:	###\n" << std::endl;
		// 			Request req(buff);
		// 			std::cout << "###	End client message	###\n" << std::endl;

		// 			Response rep(req);
		// 			std::string response = rep.printResponse();
		// 			write(changeList[i].data.fd, response.c_str(), response.size());
		// 			std::cout << "###  Server Message: ###\n\n" << response << "\n###  End server message ###\n" << std::endl;
		int nfds = _fds.size();
		int nb = poll(&_fds[0], nfds , -1);
		if (nb == -1)
			throw (std::invalid_argument("Error: Poll."));
		for (unsigned int i = 0; i < _fds.size(); i++) {
				if (_fds[i].revents & POLLIN) {
					if (IsSocketServer(_fds[i].fd)) {
						int fdServer = FindIndexServerFD(*this, _fds[i].fd);
						if (fdServer == -1)
							throw (std::runtime_error("Error: Find FD serveur failed"));
						struct sockaddr_in	&clientAddr = _configServer[fdServer].GetSockAddr();
						socklen_t addrLen = sizeof(clientAddr);
						int socketClient = accept(_fds[i].fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);

						struct pollfd clientPoll;
						clientPoll.fd = socketClient;
						clientPoll.events = POLLIN;
						clientPoll.revents = 0;
						_fds.push_back(clientPoll);
						Client newClient(socketClient);
						newClient.SetIdClient(IdClient++);
						NbClient++;
						_client.push_back(newClient);
					}
					else {
						char buff[1024];
						if (read(_fds[i].fd, buff, sizeof(buff)) == 0) {
							close(_fds[i].fd);
							_fds.erase(_fds.begin() + i);	// supprimer la struct
							NbClient--;
						}
						else {
						std::cout << "###	Client Message:	###\n" << std::endl;
						Request req(buff);
						std::cout << "###	End client message	###\n" << std::endl;

						Response rep(req);
						std::string response = rep.printResponse();
						write(_fds[i].fd, response.c_str(), response.size());
						std::cout << "###  Server Message: ###\n\n" << response << "\n###  End server message ###\n" << std::endl;
						}
					}
			}
			NbRequest++;
			std::cout << "Nb Request is: " << NbRequest << std::endl;
			std::cout << "Nb Client is: " << NbClient << std::endl;
		}
	}
}	


static int	FindIndexServerFD(Server &server, int fd)
{
	for (int i = 0; i < server.GetNumberConfig(); i++) {
		ConfigServer &config = server.GetConfigServer(i);
		if (fd == config.GetSocket())
			return (config.GetSocket());
	}
	return (-1);
}

void	Server::ParseConfig(std::vector<std::string> &fileArray)
{
	int isBlance = IsBalance(fileArray);
	if (isBlance > 0)
		throw (std::invalid_argument("Error: Missing '}' in config file."));
	else if (isBlance < 0)
		throw (std::invalid_argument("Error: Missing '{' in config file."));
	unsigned int i = (fileArray.size() - 1);
	for (int j = 0; fileArray[i][j] != '}'; i--) {
		if (fileArray[i][0] != '}' && fileArray[i][0] != '#')
			throw (std::invalid_argument("Error: Invalid syntaxe config file."));
	}

	size_t	start = 0;
	size_t	index = 0;
	while (start < fileArray.size()) {
		ssize_t end = EndChunk(fileArray, start);
		if (end != -1) {
			std::vector<std::string> serverChunk;
			for (int i = start; i < end; i++)
				serverChunk.push_back(fileArray[i]);

			if (IsValideBloc(serverChunk))
				throw (std::invalid_argument("Error: Invalid syntaxe."));
			ConfigServer conf;
			_configServer.push_back(conf);

			_configServer[index].FillConfigServer(serverChunk);
			start = end;
			index++;
			_numberConfig++;
		}
		else
			break;
	}
	return ;
}

static int IsValideBloc(std::vector<std::string> &serverBloc)
{
	std::string first;
	for (unsigned int i = 0; i < serverBloc.size(); i++) {
		if (serverBloc[i][0] == '#')
			continue ;
		for (int j = 0; j < serverBloc[i][j]; j++) {
			if (serverBloc[i][j] == ' ')
				continue ;
			first.push_back(serverBloc[i][j]);
		}
		break ;
	}
	if (first.compare("server{"))
		return (1);
	for (unsigned int i = 0; i < serverBloc.size(); i++) {
		if (serverBloc[i][0] != '{' && serverBloc[i][0] != '}' && serverBloc[i].size() < 5)
			return (1);
	}
	return (0);
}

static int	IsBalance(std::vector<std::string> &fileArray)
{
	int balance = 0;
	for (unsigned int i = 0; i < fileArray.size(); i++) {
		for (int j = 0; fileArray[i][j]; j++) {
			if (fileArray[i][j] == '{')
				balance++;
			else if (fileArray[i][j] == '}') {
				balance--;
				if (balance < 0)
					return (-1);
			}
		}
	}
	return (balance);
}

bool	Server::IsSocketServer(int fd)
{
	for (unsigned int i = 0; i < _configServer.size(); i++) {
		if (fd == _configServer[i].GetSocket())
			return (true);
	}
	return (false);
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

void	Server::SetFdServer(int fd)
{
	(void)fd;
	// _fdServer = fd;
	return ;
}

void	Server::SetNumberConfig(int number)
{
	_numberConfig = number;
	return ;
}

void	Server::SetSockAddr(struct sockaddr_in sockaddr)
{
	(void) sockaddr;
	// _sockAddress = sockaddr;
	return ;
}

void	Server::SetConfigServer(ConfigServer const &config)
{
	_configServer.push_back(config);
	return ;
}

void	Server::SetClient(Client const &client)
{
	_client.push_back(client);
	return ;
}

// int	Server::GetFdServer( int index )
// {
// 	return (_fdServer[index]);
// }

int	Server::GetNumberConfig( void )
{ return (_numberConfig); }

ConfigServer&	Server::GetConfigServer(int index)
{ return (_configServer[index]); }
