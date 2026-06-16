/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:10:16 by strieste          #+#    #+#             */
/*   Updated: 2026/06/15 20:55:12 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "ConfigLocation.hpp"
#include <ctime>
# include <string>
# include <vector>
#include <unistd.h>
# include <cstdlib>	// std::strtol

# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class Client
{
	public:
		Client();
		Client(int fd);
		Client(Client const &copy);
		~Client();
		Client&	operator=(Client const &copy);

		int	GetFd( void );
		int	GetIdClient( void );
		// int	GetReadyRequest( void );
		int	GetFdConfigServer( void );
		std::string	GetRequest( void );
		int	GetIndexConfigServer( void );
		std::time_t	GetTime( void );
		int	GetIndexFdsStruct( void );
		
		
		// void	SetIndexFdsStruct(int index);
		void	SetTime(std::time_t value);
		void	SetFd(int fd);
		void	SetIdClient(int id);
		void	SetFdConfigServer(int fd);
		// void	SetReadyRequest(bool value);
		void	SetIndexConfigServer(int index);

		void	FillRequestClient(std::string const &str);
		bool	ClientRequestIsReady( void );
		void	ResetRequest( void );
		void	CloseFd( void );

		void	SetPipeFd(int fd);
		void	SetPidCgi(pid_t pid);
		pid_t	GetPidCgi( void );
		int		GetPipeFd( void );
		void	SetIsCgi(bool value);
		bool	GetIsCgi( void );
		std::string	GetCgiResponse( void );
		void	AppendCgiResponse(char *buff, int bytes);
		void	CleanCgiResponse( void );
		void	SetTimeCgi(std::time_t time);
		std::time_t	GetTimeCgi(void );

	private:
		int	_fd;
		int	_idClient;
		int _fdConfigServer;
		int	_indexConfigServer;
		std::string	_request;
		std::time_t	_time;
		// bool	_ready;
		// size_t	_remainingRead;

		bool	_isCgi;
		pid_t	_cgiPid;
		int		_pipeFd;
		std::time_t	_startCgi;
		std::string	_cgiResponse;

};

#endif