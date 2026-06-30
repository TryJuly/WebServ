/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 14:38:06 by strieste          #+#    #+#             */
/*   Updated: 2026/06/30 15:03:44 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../header/Server.hpp"
# include "../header/Request.hpp"
# include "../header/Response.hpp"

/*	ON/OFF Server	*/
int run = 1;

static void	HelpCgiResponse(std::string &response);
static bool	SetTokenConfigServer(std::string token);
static int	FindIndexServerFD(Server &server, int fd);
static int	IsBalance(std::vector<std::string> &fileArray);
static void	AddCookieSession(std::string &str, Client &client);
static int	IsValideBloc(std::vector<std::string> &serverBloc);
static void CheckServerName(std::vector<ConfigServer>& configs);
static void	CheckConfigRequired(std::set<std::string> configRequired);
static struct sockaddr_in ResolveHostToAddr(const std::string& host, int port);
static std::string	SendErrorPage(ConfigServer &serverConfig, int errorNumber);
static bool	IsValideMethodeForLocation(std::string methode, ConfigLocation const &location);

/*	Default	*/

Server::Server()
{
	_numberClient = -1;
	_numberConfig = -1;
	return ;
}

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
	if (fileArray.empty())
		throw (std::invalid_argument("Error: Empty file: " + fileName));
	ParseConfig(fileArray);
	CleanSetError();
	CheckServerName(_configServer);
	CheckConfigServer();
	SetUpServer();
	return ;
}

Server::Server(Server const &copy)
{ (*this) = copy; return; }

Server::~Server()
{
	for (size_t	i = 0; i < _fds.size(); i++)
		close(_fds[i].fd);
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

/*	Getter	*/

int	Server::GetNumberConfig( void )
{ return (_numberConfig); }

ConfigServer&	Server::GetConfigServer(int index)
{ return (_configServer[index]); }

int	Server::GetIndexClient(int fd)
{
	for (size_t	i = 0; i < _client.size(); i++) {
		if (_client[i].GetFd() == fd)
			return (i);
	}
	return (-1);
}

/*	Setter	*/

void	Server::SetNumberConfig(int number)
{ _numberConfig = number; return ; }

void	Server::SetConfigServer(ConfigServer const &config)
{ _configServer.push_back(config); return ; }

void	Server::SetClient(Client const &client)
{ _client.push_back(client); return ; }

/*	Function	*/

void	signalHandler(int sig)
{
	(void)sig;
	run = 0;
	return ;
}

void Server::StartServer()
{
	int	IdClient = 0;
	signal(SIGINT, signalHandler);
	while (run) {
		int nfds = _fds.size();
		int nb = poll(&_fds[0], nfds , 1000);
		if (nb == -1)
			throw (std::runtime_error("Error: Poll()." + std::string(strerror(errno))));
		for (unsigned int i = 0; i < _fds.size(); i++) {
			if (_fds[i].revents & POLLIN || _fds[i].revents & POLLHUP) {
				if (IsSocketServer(_fds[i].fd)) {
					AcceptClient(_fds[i].fd, IdClient);
					IdClient++;
					_numberClient++;
				}
				else if (IsCgiEvent(_fds[i].fd))
					SendCgiResponse(i);
				else
					CatchClientRequest(i, _numberClient);
			}
			else if (_fds[i].revents & POLLOUT) {
				if (IsCgiStdinEvent(_fds[i].fd))
					SendCgiBody(i);
				else if (!IsSocketServer(_fds[i].fd))
					WriteClientBuffer(i, _numberClient);
			}
		}
		CheckTimeoutClient();
	}
	return ;
}

void	Server::SetUpServer()
{
	for (unsigned int i = 0; i < _configServer.size(); i++) {
		ConfigServer	&config = _configServer[i];

		int existingSocket = -1;
		for (unsigned int j = 0; j < i; j++) {
			if (_configServer[j].GetPort() == config.GetPort() && _configServer[j].GetHost() == config.GetHost()) {
				existingSocket = _configServer[j].GetSocket();
				break ;
			}
		}
		if (existingSocket != -1) {
			config.SetSocket(existingSocket);
			_socketToConfigs[existingSocket].push_back(i);
			continue ;
		}

		config.SetSocket(socket(AF_INET, SOCK_STREAM, 0));
		if (config.GetSocket() == -1)
			throw (std::runtime_error("Error: socket() failed: " + std::string(strerror(errno))));
		int opt = 1;
		setsockopt(config.GetSocket(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (fcntl(config.GetSocket(), F_SETFL, O_NONBLOCK | FD_CLOEXEC) == -1) {
			close(config.GetSocket());
			throw (std::runtime_error("Error: fcntl() failed: " + std::string(strerror(errno))));
		}
		struct sockaddr_in sockAddr = ResolveHostToAddr(config.GetHost(), config.GetPort());
		config.SetSockAddr(sockAddr);

		if (bind(config.GetSocket(), reinterpret_cast<struct sockaddr *>(&sockAddr), sizeof(sockAddr)) != 0)
			throw (std::runtime_error("Error: bind() failed: " + std::string(strerror(errno))));
		if (listen(config.GetSocket(), SOMAXCONN) != 0)
			throw (std::runtime_error("Error: listen() failed: " + std::string(strerror(errno))));

		_socketToConfigs[config.GetSocket()].push_back(i);
		struct pollfd	serverPoll;
		serverPoll.fd = config.GetSocket();
		serverPoll.events = POLLIN;
		serverPoll.revents = 0;
		_fds.push_back(serverPoll);
	}
	return ;
}

void	Server::AcceptClient(int fd, int idClient)
{
	int fdServer = FindIndexServerFD(*this, fd);
	if (fdServer == -1)
		return ;
	struct sockaddr_in	&clientAddr = _configServer[fdServer].GetSockAddr();
	socklen_t addrLen = sizeof(clientAddr);
	int socketClient = accept(fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);
	if (socketClient == -1)
		return ;
	if (fcntl(socketClient, F_SETFL, O_NONBLOCK | FD_CLOEXEC) == -1) {
		close(socketClient);
		return ;
	}

	struct pollfd clientPoll;
	clientPoll.fd = socketClient;
	clientPoll.events = POLLIN | POLLOUT;
	clientPoll.revents = 0;
	_fds.push_back(clientPoll);

	Client newClient(socketClient);
	newClient.SetIdClient(idClient);
	newClient.SetFdConfigServer(fdServer);
	newClient.SetIndexConfigServer(fdServer);
	newClient.SetTime(std::time(NULL));
	_client.push_back(newClient);
	return ;
}

bool	Server::IsCgiEvent(int fd)
{
	for (size_t	i = 0; i < _client.size(); i++) {
		if (_client[i].GetIsCgi() == true && fd == _client[i].GetPipeFd())
			return (true);
	}
	return (false);
}

int	Server::GetClientByPipe(int fd)
{
	for (size_t	i = 0; i < _client.size(); i++) {
		if (_client[i].GetIsCgi() == true && _client[i].GetPipeFd() == fd)
			return (i);
	}
	return (-1);
}

void	Server::SendCgiResponse(int i)
{
	int indexClient = GetClientByPipe(_fds[i].fd);
	if (indexClient == -1)
		return ;
	char buff[1024];

	try {
		int bytes = read(_client[indexClient].GetPipeFd(), buff, sizeof(buff) - 1);
		if (bytes < 0)
			throw (std::runtime_error("500 Error internal"));
		if (bytes > 0) {
			buff[bytes] = '\0';
			_client[indexClient].AppendCgiResponse(buff, bytes);
		}
		else if (bytes == 0) {
			std::string response = _client[indexClient].GetCgiResponse();
			HelpCgiResponse(response);
			if (_client[indexClient].GetCookie().empty())
				AddCookieSession(response, _client[indexClient]);
			_client[indexClient].AppendWriteBuffer(response);
			close(_fds[i].fd);
			int status;
			waitpid(_client[indexClient].GetPidCgi(), &status, WNOHANG);
			if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
				_fds.erase(_fds.begin() + i);
				_client[indexClient].SetIsCgi(false);
				_client[indexClient].CleanCgiResponse();
				_client[indexClient].ResetRequest();
				throw (std::runtime_error("500 Error: Child Process"));
			}
			_fds.erase(_fds.begin() + i);
			i--;
			_client[indexClient].ResetRequest();
			_client[indexClient].SetIsCgi(false);
			_client[indexClient].CleanCgiResponse();
			_client[indexClient].SetTime(std::time(NULL));
		}
	}
	catch (const std::exception& e) {
		std::string handleError = e.what();
		std::string stringError = handleError.substr(0, 3);
		int error = std::strtol(stringError.c_str(), NULL, 10);
		ConfigServer &config = _configServer[_client[indexClient].GetIndexConfigServer()];
		std::string response = SendErrorPage(config, error);
		close(_fds[i].fd);
		_fds.erase(_fds.begin() + i);
		_client[indexClient].AppendWriteBuffer(response);
		_client[indexClient].ResetRequest();
		_client[indexClient].CleanCgiResponse();
		_client[indexClient].SetIsCgi(false);
		_client[indexClient].SetTime(std::time(NULL));
	}
	return ;
}

void	Server::HandleCgiRequest(ConfigServer &config, Request const &req, int indexClient)
{
	CGI	process(_client[indexClient].GetRequest());
	if (process.GetBodySize() > config.GetMaxBodySize())
		throw (std::invalid_argument("413 Error: Body size limit"));
	std::string path = req.getPath();
	size_t	start = path.find('/');
	size_t	end = path.find('/', start + 1);
	int indexLocation = 0;
	if (end == std::string::npos)
		indexLocation = config.FindLocationPath("/");
	else {
		std::string locationPath = path.substr(start, end - start);
		indexLocation = config.FindLocationPath(locationPath);
		if (indexLocation == -1)
			throw (std::runtime_error("404 Error:"));
	}
	if (IsValideMethodeForLocation(req.getMethod(), config.GetConfigLocation(indexLocation)) == true) {
		process.LaunchCGI(config.GetConfigLocation(indexLocation), config);
		_client[indexClient].SetPipeFd(process.GetPipeFd());
		_client[indexClient].SetPidCgi(process.GetPidCgi());
		_client[indexClient].SetTimeCgi(std::time(NULL));
		_client[indexClient].SetIsCgi(true);
		struct pollfd pollCgi;
		pollCgi.fd = process.GetPipeFd();
		pollCgi.events = POLLIN;
		pollCgi.revents = 0;
		_fds.push_back(pollCgi);
		std::string body = process.GetCgiBody();
		if (!body.empty()) {
			_client[indexClient].SetPipeInFd(process.GetPipeInFd());
			_client[indexClient].SetCgiBody(body);
			struct pollfd pollCgiIn;
			pollCgiIn.fd = process.GetPipeInFd();
			pollCgiIn.events = POLLOUT;
			pollCgiIn.revents = 0;
			_fds.push_back(pollCgiIn);
		}
		else
			close(process.GetPipeInFd());
	}
	else
		throw (std::runtime_error("405 Error:"));
	return ;
}

int	Server::FindClientByCookie(std::string const &cookie)
{
	for (size_t	i = 0; i < _client.size(); i++) {
		if (_client[i].GetFd() == -1 && cookie == _client[i].GetCookie())
			return (i);
	}
	return (-1);
}

void	Server::CatchClientRequest(int i, int &NbClient)
{
	int indexClient = GetIndexClient(_fds[i].fd);
	if (indexClient == -1)
		return ;
	try {
		char buff[1024];
		int indexConfigServer = _client[indexClient].GetIndexConfigServer();
		if (indexConfigServer < 0)
			return ;
		int bytes = read(_client[indexClient].GetFd(), buff, sizeof(buff) - 1);
		if (bytes < 0) {
			close(_client[indexClient].GetFd());
			_fds.erase(_fds.begin() + i);
			_client[indexClient].SetFd(-1);
			i--;
			NbClient--;
		}
		else if (bytes == 0) {
			close(_client[indexClient].GetFd());
			_fds.erase(_fds.begin() + i);
			_client[indexClient].SetFd(-1);
			i--;
			NbClient--;
		}
		else {
			buff[bytes] = '\0';
			_client[indexClient].FillRequestClient(buff, bytes);
			if (_client[indexClient].ClientRequestIsReady() == false)
				return ;
			std::cout << "###	Client Message:	###\n" << std::endl;
			Request req(_client[indexClient].GetRequest());
			std::cout << "\n###	End client message	###\n" << std::endl;

			std::map<std::string, std::string> header = req.getHeaders();
			std::map<std::string, std::string>::iterator it = header.find("Host");
			if (it != header.end()) {
				std::string hostHeader = it->second;
				size_t colon = hostHeader.find(':');
				if (colon != std::string::npos)
					hostHeader = hostHeader.substr(0, colon);
				int sock = _configServer[indexConfigServer].GetSocket();
				std::map<int, std::vector<int> >::iterator mIt = _socketToConfigs.find(sock);
				if (mIt != _socketToConfigs.end()) {
					for (size_t k = 0; k < mIt->second.size(); k++) {
						int index = mIt->second[k];
						if (_configServer[index].GetServerName() == hostHeader) {
							indexConfigServer = index;
							_client[indexClient].SetIndexConfigServer(index);
							break ;
						}
					}
				}
			}
			ConfigServer &activeConfig = _configServer[indexConfigServer];

			std::string	cookie = req.getCookie();
			if (cookie.empty() == false) {
				int indexClientByCookie = FindClientByCookie(cookie);
				if (indexClientByCookie != -1) {
					_client[indexClientByCookie].SetFd(_client[indexClient].GetFd());
					_client[indexClientByCookie].ResetRequest();
					std::string req = _client[indexClient].GetRequest();
					_client[indexClientByCookie].FillRequestClient(req.c_str(), req.size());
					_client.erase(_client.begin() + indexClient);
					NbClient--;
					indexClient = GetIndexClient(_fds[i].fd);
					if (indexClient == -1)
						return ;
				}
			}
			if (req.IsCGI() && req.getMethod() == "DELETE")
				throw (std::invalid_argument("405 Error Method not allowed"));
			if (req.getBody().size() > static_cast<unsigned int>(activeConfig.GetMaxBodySize()))
				throw std::runtime_error("413 Error: Payload too large");
			if (req.IsCGI() == true) {
				HandleCgiRequest(activeConfig, req, indexClient);
				_client[indexClient].SetTimeCgi(std::time(NULL));
			}
			else {
				Response rep(req, activeConfig);
				if (rep.getBody().size() > static_cast<unsigned int>(activeConfig.GetMaxBodySize()))
					throw std::runtime_error("413 Error: Payload too large");
				std::string response = rep.printResponse();
				if (cookie.empty())
					AddCookieSession(response, _client[indexClient]);
				_client[indexClient].AppendWriteBuffer(response);
			}
			_client[indexClient].ResetRequest();
			_client[indexClient].SetTime(std::time(NULL));
		}
	}
	catch (const std::exception& e) {
		std::string handleError = e.what();
		std::string stringError = handleError.substr(0, 3);
		int error = std::strtol(stringError.c_str(), NULL, 10);
		ConfigServer &config = _configServer[_client[indexClient].GetIndexConfigServer()];
		std::string response = SendErrorPage(config, error);
		_client[indexClient].AppendWriteBuffer(response);
		_client[indexClient].CleanCgiResponse();
		_client[indexClient].ResetRequest();
		_client[indexClient].SetTime(std::time(NULL));
	}
	return ;
}

void	Server::CheckTimeoutClient( void )
{
	for (size_t k = 0; k < _client.size(); k++) {
		if (_client[k].GetFd() == -1) {
			if (std::time(NULL) - _client[k].GetTime() > 3000) {
				_client.erase(_client.begin() + k);
				k--;
				continue ;
			}
		}
		if (std::time(NULL) - _client[k].GetTime() > 30) {
			int fdClose = _client[k].GetFd();
			if (fdClose == -1)
				continue ;
			for (size_t j = 0; j < _fds.size(); j++) {
				if (_fds[j].fd == fdClose) {
					_fds.erase(_fds.begin() + j);
					_numberClient--;
					j--;
					break ;
				}
			}
			close(fdClose);
			_client[k].SetFd(-1);
			continue ;
		}
		if (_client[k].GetIsCgi() && std::time(NULL) - _client[k].GetTimeCgi() > 10) {
			int pipeFd = _client[k].GetPipeFd();
			kill(_client[k].GetPidCgi(), SIGKILL);
			waitpid(_client[k].GetPidCgi(), NULL, 0);
			for (size_t	i = 0; i < _fds.size(); i++) {
				if (_fds[i].fd == pipeFd) {
					_fds.erase(_fds.begin() + i);
					i--;
					break ;
				}
			}
			close(pipeFd);
			ConfigServer	&config = _configServer[_client[k].GetIndexConfigServer()];
			std::string response = SendErrorPage(config, 504);
			_client[k].AppendWriteBuffer(response);
			_client[k].SetIsCgi(false);
		}
	}
	return ;
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

void Server::StopServer()
{
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
			throw (std::invalid_argument("Error: Invalid syntax in config file -> " + fileArray[i]));
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
				throw (std::invalid_argument("Error: Invalid syntax in config file."));
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

void	Server::CheckConfigServer()
{
	struct stat	sstat;
	std::set<int> portCheck;
	for (size_t	i = 0; i < _configServer.size(); i++) {
		ConfigServer &configServer = GetConfigServer(i);
		int port = configServer.GetPort();
		if (port < 0 || port > 65534)
			throw (std::invalid_argument("Error: Invalid port."));
		if (portCheck.count(port) > 0)
			throw (std::invalid_argument("Error: Same port."));
		portCheck.insert(port);

		for (int j = 0; j < configServer.GetNumberLocation(); j++) {
			ConfigLocation &location = configServer.GetConfigLocation(j);
			std::string path = location.GetRoot() + location.GetPath();
			if (stat(path.c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + path));
			if (stat(location.GetRoot().c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + location.GetRoot()));
			if (!location.GetUpload().empty() && stat(location.GetUpload().c_str(), &sstat) != 0)
				throw (std::invalid_argument("Error: Invalid path location: " + location.GetUpload()));
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

/*	Static Functin	*/

static int	IsBalance(std::vector<std::string> &fileArray)
{
	int balance = 0;
	for (unsigned int i = 0; i < fileArray.size(); i++) {
		if (fileArray[i][0] == '#')
			continue ;
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

static int IsValideBloc(std::vector<std::string> &serverChunk)
{
	std::string first;
	std::set<std::string>	configRequired;
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
		size_t	pos = serverChunk[i].find(' ');
		if (serverChunk[i] != "}" && serverChunk[i] != "{" && pos == std::string::npos)
			throw (std::invalid_argument("Error: invalid input -> " + serverChunk[i]));
		std::string token = serverChunk[i].substr(0, pos);
		if (token == "location") {
			while (i < serverChunk.size() && serverChunk[i] != "}")
				i++;
			continue ;
		}
		if (token != "}" && SetTokenConfigServer(token) == false)
			throw (std::invalid_argument("Error: invalid input -> " + serverChunk[i]));
		configRequired.insert(token);

		if (serverChunk[i][0] == '#')
			continue ;
		if (serverChunk[i][0] != '{' && serverChunk[i][0] != '}' && serverChunk[i].size() < 5)
			return (1);
	}
	CheckConfigRequired(configRequired);
	return (0);
}

static void	CheckConfigRequired(std::set<std::string> configRequired)
{
	int valid = 0;
	if (configRequired.count("listen") != 1) {
		valid++;
		std::cerr << RED << "Error: Missing configuration minimum required ‘listen'." << RESET << std::endl;
	}
	if (configRequired.count("server_name") != 1) {
		valid++;
		std::cerr << RED << "Error: Missing configuration minimum required ‘server_name'." << RESET << std::endl;
	}
	if (configRequired.count("root") != 1) {
		valid++;
		std::cerr << RED << "Error: Missing configuration minimum required ‘root'." << RESET << std::endl;
	}
	// if (configRequired.count("index") != 1) {
	// 	valid++;
	// 	std::cerr << RED << "Error: Missing configuration minimum required ‘index'." << RESET << std::endl;
	// }
	if (configRequired.count("client_max_body_size") != 1) {
		valid++;
		std::cerr << RED << "Error: Missing configuration minimum required ‘client_max_body_size'." << RESET << std::endl;
	}
	if (valid != 0)
		throw (std::invalid_argument("Error: Missing configuration minimum required."));
	return ;
}

static bool	SetTokenConfigServer(std::string token)
{
	if (!token.compare("server"))
		return (true);
	if (!token.compare("listen"))
		return (true);
	if (!token.compare("server_name"))
		return (true);
	if (!token.compare("root"))
		return (true);
	if (!token.compare("index"))
		return (true);
	if (!token.compare("client_max_body_size"))
		return (true);
	if (!token.compare("error_page"))
		return (true);
	if (!token.compare("location"))
		return (true);
	return (false);
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

static bool	IsValideMethodeForLocation(std::string methode, ConfigLocation const &location)
{
	if (!methode.compare("GET"))
		return (location.GetBoolGet());
	else if (!methode.compare("POST"))
		return (location.GetBoolPost());
	else if (!methode.compare("DELETE"))
		return (location.GetBoolDelete());
	return (false);
}

static std::string	SendErrorPage(ConfigServer &serverConfig, int errorNumber)
{
	std::string errorPage;
	std::string errorPath = serverConfig.GetErrorPages(errorNumber);
	if (!errorPath.compare("Error"))
		return ("");
	std::ifstream fd(errorPath.c_str(), std::ios::in);

	if (!fd.is_open())
		throw (std::runtime_error("500 Error: Open file SendErrorPage()"));

	std::string buff;
	while (!fd.eof()) {
		std::getline(fd, buff);
		errorPage += buff + '\n';
	}
	fd.close();
	std::string status;
	std::string type = "Content-Type: text/html\r\n";
	std::string length = "Content-Length:" + return_file_length(errorPage.size()) + "\r\n";
	std::string spaceBody = "\r\n\r\n";
	std::string paste = type + length + spaceBody + errorPage;
	std::string response;

	switch (errorNumber) {
	case 400:
		status = "HTTP/1.1 400 Bad Request\r\n";
		response = status + paste;
		break;
	case 401:
		status = "HTTP/1.1 401 Unauthorized\r\n";
		response = status + paste;
		break;
	case 403:
		status = "HTTP/1.1 403 Forbidden\r\n";
		response = status + paste;
		break;
	case 404:
		status = "HTTP/1.1 404 Not Found\r\n";
		response = status + paste;
		break;
	case 405:
		status = "HTTP/1.1 405 Methode Not Allowed\r\n";
		response = status + paste;
		break;
	case 409:
		status = "HTTP/1.1 409 Conflict\r\n";
		response = status + paste;
		break;
	case 413:
		status = "HTTP/1.1 413 Content Too Large\r\n";
		response = status + paste;
		break;
	case 415:
		status = "HTTP/1.1 415 Unsupported Media Type\r\n";
		response = status + paste;
		break;
	case 500:
		status = "HTTP/1.1 500 Internal Server Error\r\n";
		response = status + paste;
		break;
	case 502:
		status = "HTTP/1.1 502 Bad Gateway\r\n";
		response = status + paste;
		break;
	case 503:
		status = "HTTP/1.1 503 Service Unavailable\r\n";
		response = status + paste;
		break;
	case 504:
		status = "HTTP/1.1 504 Gateway Timeout\r\n";
		response = status + paste;
		break;

	default:
		break;
	}
	return(response) ;
}

static void	AddCookieSession(std::string &str, Client &client)
{
	int idClient = client.GetIdClient();
	std::stringstream ss;
	ss << idClient;
	std::string idClientStr = "Webserv" + ss.str();

	int time = std::time(NULL) + idClient;
	std::stringstream sd;
	sd << time;
	std::string value = sd.str();

	std::string cookie = "Set-Cookie: " + idClientStr + "=" + value + ";\r\n";
	client.SetCookie(value);
	size_t	pos = str.find("\n");
	std::string result = str.substr(pos + 1);
	result = cookie + result;
	result = str.substr(0, pos + 1) + result;
	str = result;
	return ;
}

static void	HelpCgiResponse(std::string &response)
{
	size_t	sep = response.find("\r\n\r\n");
	size_t	offs = 4;
	if (sep == std::string::npos) {
		sep = response.find("\r\n");
		offs = 2;
	}
	if (sep == std::string::npos) {
		std::string length = return_file_length(response.size());
		response = "Content-Length: " + length + "\r\n\r\n" + response;
	}
	else {
		std::string	body = response.substr(sep + offs);
		std::string length = return_file_length(body.size());
		response = "Content-Length: " + length + "\r\n" + response;
	}
	if (response.substr(0, 5) != "HTTP/") {
		std::string status = "HTTP/1.1 200 OK\r\n";
		response = status + response;
	}
	return ;
}

static void CheckServerName(std::vector<ConfigServer>& configs)
{
	for (size_t i = 0; i < configs.size(); ++i) {
		for (size_t j = i + 1; j < configs.size(); ++j) {
			bool SamePort = false;
			bool SameHost = false;
			bool SameName = false;
			
			if (configs[i].GetPort() == configs[j].GetPort())
				SamePort = true;
			if (configs[i].GetHost() == configs[j].GetHost())
				SameHost = true;
			if (configs[i].GetServerName() == configs[j].GetServerName())
				SameName = true;

			if (SamePort && SameHost && SameName)
				throw std::runtime_error( "Error: Conflicting server_name on same host:port: " + configs[i].GetServerName());
		}
	}
	return ;
}

bool	Server::IsCgiStdinEvent(int fd)
{
	for (size_t i = 0; i < _client.size(); i++) {
		if (_client[i].GetPipeInFd() == fd)
			return (true);
	}
	return (false);
}

int		Server::GetClientByPipeIn(int fd)
{
	for (size_t i = 0; i < _client.size(); i++) {
		if (_client[i].GetPipeInFd() == fd)
			return (i);
	}
	return (-1);
}

void	Server::SendCgiBody(int i)
{
	int indexClient = GetClientByPipeIn(_fds[i].fd);
	if (indexClient == -1)
		return ;
	std::string body = _client[indexClient].GetCgiBody();
	if (body.empty()) {
		close(_fds[i].fd);
		_fds.erase(_fds.begin() + i);
		_client[indexClient].SetPipeInFd(-1);
		return ;
	}
	int n = write(_fds[i].fd, body.c_str(), body.size());
	if (n <= 0) {
		kill(_client[indexClient].GetPidCgi(), SIGKILL);
		waitpid(_client[indexClient].GetPidCgi(), NULL, 0);
		close(_fds[i].fd);
		_fds.erase(_fds.begin() + i);
		_client[indexClient].SetPipeInFd(-1);
		ConfigServer &config = _configServer[_client[indexClient].GetIndexConfigServer()];
		std::string response = SendErrorPage(config, 500);
		_client[indexClient].AppendWriteBuffer(response);
		_client[indexClient].SetIsCgi(false);
		return ;
	}
	_client[indexClient].SetCgiBody(body.substr(n));
}

void	Server::WriteClientBuffer(int i, int &NbClient)
{
	int indexClient = GetIndexClient(_fds[i].fd);
	if (indexClient == -1)
		return ;
	std::string buf = _client[indexClient].GetWriteBuffer();
	if (buf.empty())
		return ;
	int n = write(_fds[i].fd, buf.c_str(), buf.size());
	if (n <= 0) {
		close(_fds[i].fd);
		_fds.erase(_fds.begin() + i);
		_client[indexClient].SetFd(-1);
		NbClient--;
		return ;
	}
	_client[indexClient].SetWriteBuffer(buf.substr(n));
}

static struct sockaddr_in ResolveHostToAddr(const std::string& host, int port)
{
	struct addrinfo hints;
	struct addrinfo* res;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	std::ostringstream portStream;
	portStream << port;

	int status = getaddrinfo(host.c_str(), portStream.str().c_str(), &hints, &res);
	if (status != 0)
		throw std::runtime_error("getaddrinfo() failed: " + std::string(gai_strerror(status)));

	struct sockaddr_in addr = *reinterpret_cast<struct sockaddr_in*>(res->ai_addr);

	freeaddrinfo(res);

	return addr;
}
