/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:12:35 by strieste          #+#    #+#             */
/*   Updated: 2026/06/22 09:15:12 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <cerrno>
# include <cstddef>
# include <stdexcept>
# include "../header/Client.hpp"

/*	Default	*/
Client::Client()
{
	_fd = -1;
	_pipeFd = -1;
	_cgiPid = -1;
	_cookie = "";
	_isCgi = false;
	_indexConfigServer = -1;
	_time = std::time(NULL);
	_startCgi = std::time(NULL);
	return ;
}

Client::Client(int fd)
{
	_fd = fd;
	_pipeFd = -1;
	_cgiPid = -1;
	_cookie = "";
	_isCgi = false;
	_indexConfigServer = -1;
	_time = std::time(NULL);
	_startCgi = std::time(NULL);
	return ;
}

Client::Client(Client const &copy)
{ (*this) = copy; return ; }

Client::~Client()
{ return ; }

Client&	Client::operator=(Client const &copy)
{
	if (this != &copy) {
		_fd = copy._fd;
		_time = copy._time;
		_isCgi = copy._isCgi;
		_pipeFd = copy._pipeFd;
		_cookie = copy._cookie;
		_cgiPid = copy._cgiPid;
		_request = copy._request;
		_idClient = copy._idClient;
		_startCgi = copy._startCgi;
		_cgiResponse = copy._cgiResponse;
		_fdConfigServer = copy._fdConfigServer;
		_indexConfigServer = copy._indexConfigServer;
	}
	return (*this);
}

/*	Setter	*/

void	Client::SetCookie(std::string const &value)
{ _cookie = value; return ; }

std::string	Client::GetCookie( void )
{ return (_cookie); }

void	Client::SetIndexConfigServer(int index)
{ _indexConfigServer = index; return ; }

void	Client::SetFd(int fd)
{ _fd = fd; return ; }

void	Client::SetIdClient(int id)
{ _idClient = id; return ; }

void Client::SetFdConfigServer( int fd)
{ _fdConfigServer = fd; return ; }

//		CHECK
void	Client::SetTime(std::time_t value)
{ (void) value; _time = std::time(NULL); return ; }

void	Client::SetPipeFd(int fd)
{ _pipeFd = fd; return ; }

void	Client::SetPidCgi(pid_t pid)
{ _cgiPid = pid; return ; }

void	Client::SetTimeCgi(std::time_t time)
{ _startCgi = time; return ; }

void	Client::SetIsCgi(bool value)
{ _isCgi = value; return ; }

/*	Getter	*/

int	Client::GetIndexConfigServer()
{ return (_indexConfigServer); }

int	Client::GetFd()
{ return (_fd); }

std::string	Client::GetRequest( void )
{ return (_request); }

int	Client::GetIdClient( void )
{ return (_idClient); }

int Client::GetFdConfigServer( void )
{ return (_fdConfigServer); }

std::time_t	Client::GetTime( void )
{ return (_time); }

pid_t	Client::GetPidCgi( void )
{ return (_cgiPid); }

int		Client::GetPipeFd( void )
{ return (_pipeFd); }

bool	Client::GetIsCgi( void )
{ return (_isCgi); }

std::string	Client::GetCgiResponse( void )
{ return (_cgiResponse); }

std::time_t	Client::GetTimeCgi(void )
{ return (_startCgi); }

/*	Function	*/

void	Client::CloseFd( void )
{
	close(_fd);
	_fd = -1;
	return ;
}

void	Client::ResetRequest( void )
{ _request.clear(); return ; }

void	Client::FillRequestClient(std::string const &str)
{ _request = _request + str; return ; }

bool	Client::ClientRequestIsReady( void )
{
	size_t	contentLength = _request.find("Content-Length:");
	size_t	tranferEncoding = _request.find("Transfer-Encoding: chunked");
	if (contentLength != std::string::npos) {
		size_t	posEnd = _request.find("\r\n", contentLength);
		std::string line = _request.substr(contentLength, posEnd - contentLength);
		size_t	startLength = line.find_first_of("0123456789");
		std::string strLength = line.substr(startLength);
		long bodyLength = std::strtol(strLength.c_str(), NULL, 10);
		if (errno == ERANGE || bodyLength < 0)
			throw (std::runtime_error("400 Error: Client Content-Length: "));

			// Body part
		size_t	bodyStart = _request.find("\r\n\r\n");
		if (bodyStart == std::string::npos) {
			bodyStart = _request.find("\n\n");
			bodyStart += 2;
		}
		else
			bodyStart += 4;
		std::string	body = _request.substr(bodyStart);
		if (static_cast<long>(body.size()) >= bodyLength)
			return (true);
	}
	else if (tranferEncoding != std::string::npos) {
		size_t	bodyEnd = _request.rfind("0\r\n\r\n");
		if (bodyEnd != std::string::npos)
			return (true);
	}
	else {
		size_t	emptyLine = _request.find("\r\n\r\n");
		if (emptyLine != std::string::npos)
			return (true);
	}
	return (false);
}

void	Client::AppendCgiResponse(char *buff, int bytes)
{ _cgiResponse.append(buff, bytes); return ; }

void	Client::CleanCgiResponse( void )
{ _cgiResponse.clear(); return ; }
