/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/06/09 14:25:41 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include "../header/Server.hpp"
#include "../header/Request.hpp"
#include "../header/Response.hpp"

static int	IsBalance(std::vector<std::string> &fileArray);
static int	IsValideBloc(std::vector<std::string> &serverBloc);
static int	FindIndexServerFD(Server &server, int fd);

Server::Server(int ac, char **av)
{
	std::string	fileName;
	if (ac == 2)
		fileName = av[1];
	else
		fileName = "default.cnf";

	struct stat	sStat;
	if (stat(fileName.c_str(), &sStat) == 0 && S_ISDIR(sStat.st_mode))
		throw (std::invalid_argument("Error: Is a directory: " + fileName));

	std::ifstream	fd(fileName.c_str(), std::ios::in);
	if (!fd.is_open())
		throw (std::invalid_argument("Error: Open file: " + fileName));

	std::vector<std::string>	fileArray;
	std::string	buff;
	_numberConfig = 0;
	_numberClient = 0;

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

/*	Function to verify if the configuration parsing is valid*/
void	Server::CheckConfigServer()
{
	struct stat	sstat;
	for (size_t	i = 0; i < _configServer.size(); i++) {
		ConfigServer &configServer = GetConfigServer(i);
		if (configServer.GetPort() < 0 || configServer.GetPort() > 65535)
			throw (std::invalid_argument("Error: Invalid port."));

		for (int j = 0; j < configServer.GetNumberLocation(); j++) {
			ConfigLocation &location = configServer.GetConfigLocation(j);
			std::string path = location.GetRoot() + location.GetPath();
			if (stat(path.c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + path));
			if (stat(location.GetRoot().c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + location.GetRoot()));
			if (!location.GetUpload().empty() && stat(location.GetUpload().c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + location.GetUpload()));
			if (!location.GetRedir().empty() && stat(location.GetRedir().c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + location.GetRedir()));
		}
		std::map<int, std::string>	pMap = configServer.GetMapError();
		for (std::map<int, std::string>::iterator it = pMap.begin(); it != pMap.end(); it++) {
			std::string errorPath = it->second;
			if (stat(errorPath.c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + errorPath));
		}
	}
	return ;
}

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

void	Server::AcceptClient(int fd, int idClient)
{
	int fdServer = FindIndexServerFD(*this, fd);
	if (fdServer == -1)
		throw (std::runtime_error("Error: Find FD serveur failed"));
	struct sockaddr_in	&clientAddr = _configServer[fdServer].GetSockAddr();
	socklen_t addrLen = sizeof(clientAddr);
	int socketClient = accept(fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);

	struct pollfd clientPoll;
	clientPoll.fd = socketClient;
	clientPoll.events = POLLIN;
	clientPoll.revents = 0;
	_fds.push_back(clientPoll);

	Client newClient(socketClient);
	newClient.SetIdClient(idClient);
	newClient.SetFdConfigServer(fdServer);
	newClient.SetIndexConfigServer(fdServer);
	// NbClient++;
	_client.push_back(newClient);
	return ;
}

void Server::StartServer()
{
	int NbRequest = 0;
	int NbClient = 0;
	int	IdClient = 0;

	while (true) {
		int nfds = _fds.size();
		int nb = poll(&_fds[0], nfds , -1);
		if (nb == -1)
			throw (std::runtime_error("Error: Poll."));
		for (unsigned int i = 0; i < _fds.size(); i++) {
				if (_fds[i].revents & POLLIN) {
					if (IsSocketServer(_fds[i].fd)) {
						// AcceptClient(_fds[i].fd, IdClient);
						// IdClient++;
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
						newClient.SetFdConfigServer(fdServer);
						newClient.SetIndexConfigServer(fdServer);
						NbClient++;
						_client.push_back(newClient);
					}
					else {
						char buff[1024];
						int indexConfigServer = 0;
						for (unsigned int i = 0; i < _client.size(); i++) {
							if (_fds[i].fd == _client[i].GetFdConfigServer()) {
								indexConfigServer = i;
							}
						}
						//	Stock read in buffer client client._buffer += std::string(buff, Xoctet)
						ConfigServer &config = _configServer[indexConfigServer];
						if (read(_fds[i].fd, buff, sizeof(buff) - 1) == 0) {
							close(_fds[i].fd);
							_fds.erase(_fds.begin() + i);	// supprimer la struct
							NbClient--;
						}
						else {
							std::cout << "###	Client Message:	###\n" << std::endl;
							Request req(buff);
							std::cout << "###	End client message	###\n" << std::endl;

							if (req.IsCGI() == true) {
								try {
									CGI	process(buff);
									std::string path = req.getPath();
									size_t	start = path.find('/');
									size_t	end = path.find('/', start + 1);
									int indexLocation = 0;
									if (end == std::string::npos)
										indexLocation = config.FindLocationPath("/");
									else {
										std::string locationPath = path.substr(start, end - start - 1);
										indexLocation = config.FindLocationPath(locationPath);
									}
									std::string response = process.Execute(config.GetConfigLocation(indexLocation));
								}
								catch(const std::exception& e) {
									// error file send to client
									std::cerr << e.what() << '\n';
								}
							}
							else {
								Response rep(req, config);
								std::string response = rep.printResponse();
								write(_fds[i].fd, response.c_str(), response.size());
								std::cout << "###  Server Message: ###\n\n" << response << "\n###  End server message ###\n" << std::endl;
							}
						}
					}
			}
			std::cout << "Nb Request is: " << NbRequest << std::endl;
			std::cout << "Nb Client is: " << NbClient << std::endl;
		}
		// NbRequest++;
	}
}	


static int	FindIndexServerFD(Server &server, int fd)
{
	for (int i = 0; i < server.GetNumberConfig(); i++) {
		ConfigServer &config = server.GetConfigServer(i);
		if (fd == config.GetSocket())
			return (i);
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
			throw (std::invalid_argument("Error: Invalid syntax config file."));
	}

	size_t	start = 0;
	size_t	index = 0;
	while (start < fileArray.size()) {
		for (; start < fileArray.size(); start++) {
			if (fileArray[start][0] != '#')
				break ;
		}

		ssize_t end = EndChunk(fileArray, start);
		if (end != -1) {
			std::vector<std::string> serverChunk;
			for (int i = start; i < end; i++)
				serverChunk.push_back(fileArray[i]);

			if (IsValideBloc(serverChunk))
				throw (std::invalid_argument("Error: Invalid syntax."));
			ConfigServer conf;
			_configServer.push_back(conf);

			_configServer[index].FillConfigServer(serverChunk);
			index++;
			start = end;
			_numberConfig++;
		}
		else
			break;
	}
	return ;
}

static int IsValideBloc(std::vector<std::string> &serverChunk)
{
	std::string first;
	for (unsigned int i = 0; i < serverChunk.size(); i++) {
		if (serverChunk[i][0] == '#')
			continue ;
		for (size_t j = 0; j < serverChunk[i].size(); j++) {
			if (serverChunk[i][j] == ' ' || serverChunk[i][j] == '\t')
				continue ;
			first.push_back(serverChunk[i][j]);
		}
		break ;
	}
	if (first[0] != '#' && first.compare("server{"))
		return (1);
	for (unsigned int i = 0; i < serverChunk.size(); i++) {
		if (serverChunk[i][0] == '#')
			continue ;
		if (serverChunk[i][0] != '{' && serverChunk[i][0] != '}' && serverChunk[i].size() < 5)	// Need to check
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

void	Server::CleanSetError()
{
	int size = _configServer.size();
	for (int i = 0; i < size; i++)
		_configServer[i].CleanSetError();
	return ;
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
	if (this != &copy) {
		_fds = copy._fds;
		_client = copy._client;
		_numberClient = copy._numberClient;
		_numberConfig = copy._numberConfig;
		_configServer = copy._configServer;
	}
	return (*this);
}

void Server::StopServer()
{
}


void	Server::SetNumberConfig(int number)
{
	_numberConfig = number;
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

/*	GETTER	*/

int	Server::GetNumberConfig( void )
{ return (_numberConfig); }

ConfigServer&	Server::GetConfigServer(int index)
{ return (_configServer[index]); }