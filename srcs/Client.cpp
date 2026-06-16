/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:12:35 by strieste          #+#    #+#             */
/*   Updated: 2026/06/15 20:55:52 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Client.hpp"
#include <cerrno>
#include <cstddef>
#include <stdexcept>

Client::Client()
{
	_fd = -1;
	// _ready = false;
	// _remainingRead = 0;
	_isCgi = false;
	_pipeFd = -1;
	_cgiPid = -1;
	_indexConfigServer = -1;
	_time = std::time(NULL);
	// _indexFdsStruct = -1;
	return ;
}

Client::Client(int fd)
{
	_fd = fd;
	return ;
}

Client::Client(Client const &copy)
{
	(*this) = copy;
	return ;
}

Client::~Client()
{}

Client&	Client::operator=(Client const &copy)
{
	if (this != &copy) {
		_fd = copy._fd;
		// _ready = copy._ready;
		_request = copy._request;
		_idClient = copy._idClient;
		_fdConfigServer = copy._fdConfigServer;
		_indexConfigServer = copy._indexConfigServer;
		_time = copy._time;
		_isCgi = copy._isCgi;
		_cgiPid = copy._cgiPid;
		_pipeFd = copy._pipeFd;
		_startCgi = copy._startCgi;
		_cgiResponse = copy._cgiResponse;
	}
	return (*this);
}

void	Client::SetIndexConfigServer(int index)
{
	_indexConfigServer = index;
	return ;
}

int	Client::GetIndexConfigServer()
{ return (_indexConfigServer); }

int	Client::GetFd()
{ return (_fd); }

void	Client::SetFd(int fd)
{
	_fd = fd;
	return ;
}
void	Client::SetIdClient(int id)
{
	_idClient = id;
	return ;
}

std::string	Client::GetRequest( void )
{ return (_request); }

// void	Client::SetReadyRequest(bool value)
// {
// 	_ready = value;
// 	return ;
// }

int	Client::GetIdClient( void )
{ return (_idClient); }

int Client::GetFdConfigServer( void )
{ return (_fdConfigServer); }

void Client::SetFdConfigServer( int fd)
{
	_fdConfigServer = fd;
	return ;
}

std::time_t	Client::GetTime( void )
{ return (_time); }

void	Client::SetTime(std::time_t value)
{
	(void) value;
	_time = std::time(NULL);
	return ;
}

void	Client::CloseFd( void )
{
	close(_fd);
	_fd = -1;
	return ;
}

// void	Client::SetIndexFdsStruct(int index)
// {
// 	_indexFdsStruct = index;
// }

// int	Client::GetIndexFdsStruct( void )
// { return (_indexFdsStruct); }

// int	Client::GetReadyRequest( void )
// { return (_ready); }

void	Client::ResetRequest( void )
{
	_request.clear();
	return ;
}

void	Client::FillRequestClient(std::string const &str)
{
	_request = _request + str;
	return ;
}

//		Verify is the request un completed
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
		// If no content length and no transfer encoding chunked find empty line
		size_t	emptyLine = _request.find("\r\n\r\n");
		if (emptyLine != std::string::npos)
			return (true);
	}
	return (false);
}

void	Client::SetPipeFd(int fd)
{
	_pipeFd = fd;
}

void	Client::SetPidCgi(pid_t pid)
{ _cgiPid = pid; }

pid_t	Client::GetPidCgi( void )
{ return (_cgiPid); }

int		Client::GetPipeFd( void )
{ return (_pipeFd); }

void	Client::SetIsCgi(bool value)
{ _isCgi = value; }

bool	Client::GetIsCgi( void )
{ return (_isCgi); }

std::string	Client::GetCgiResponse( void )
{ return (_cgiResponse); }

void	Client::AppendCgiResponse(char *buff, int bytes)
{
	_cgiResponse.append(buff, bytes);
	return ;
}

void	Client::CleanCgiResponse( void )
{ _cgiResponse.clear(); }

void	Client::SetTimeCgi(std::time_t time)
{_startCgi = time; }

std::time_t	Client::GetTimeCgi(void )
{ return (_startCgi); }