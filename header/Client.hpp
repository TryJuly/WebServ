/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:10:16 by strieste          #+#    #+#             */
/*   Updated: 2026/06/30 11:32:55 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <ctime>
# include <string>
# include <vector>
# include <cstdlib>	// std::strtol
# include <unistd.h>
# include "ConfigLocation.hpp"

# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class Client
{
	public:
		/*	Default	*/
		Client();
		~Client();
		Client(int fd);
		Client(Client const &copy);
		Client&	operator=(Client const &copy);

		/*	Getter	*/
		int	GetFd( void );
		int	GetPipeFd( void );
		int	GetPipeInFd( void );
		int	GetIdClient( void );
		int	GetIndexFdsStruct( void );
		int	GetFdConfigServer( void );
		int	GetIndexConfigServer( void );

		bool	GetIsCgi( void );
		pid_t	GetPidCgi( void );
		std::time_t	GetTime( void );
		std::time_t	GetTimeCgi(void );
		std::string	GetCookie( void );
		std::string	GetRequest( void );
		std::string	GetCgiBody( void );
		std::string	GetCgiResponse( void );
		std::string	GetWriteBuffer( void );

		/*	Setter	*/
		void	SetFd(int fd);
		void	SetPipeFd(int fd);
		void	SetPipeInFd(int fd);
		void	SetIdClient(int id);
		void	SetPidCgi(pid_t pid);
		void	SetIsCgi(bool value);
		void	SetFdConfigServer(int fd);
		void	SetTime(std::time_t value);
		void	SetTimeCgi(std::time_t time);
		void	SetCgiBody(std::string const &body);
		void	SetWriteBuffer(std::string const &data);
		void	SetIndexConfigServer(int index);
		void	SetCookie(std::string const &value);

		/*	Function	*/
		void	CloseFd( void );
		void	ResetRequest( void );
		void	CleanCgiResponse( void );
		void	ClearWriteBuffer( void );
		bool	ClientRequestIsReady( void );
		void	AppendCgiResponse(char *buff, int bytes);
		void	AppendWriteBuffer(std::string const &data);
		void	FillRequestClient(const char *buff, size_t bytes);


	private:
		int	_fd;
		int	_pipeFd;
		int	_pipeInFd;
		int	_idClient;
		int	_fdConfigServer;
		int	_indexConfigServer;

		bool	_isCgi;
		pid_t	_cgiPid;

		std::time_t	_time;
		std::string _cookie;
		std::string	_request;
		std::time_t	_startCgi;
		std::string	_cgiBody;
		std::string	_cgiResponse;
		std::string	_writeBuffer;

};

#endif