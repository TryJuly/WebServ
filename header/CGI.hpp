/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:37:00 by seully            #+#    #+#             */
/*   Updated: 2026/06/22 08:53:03 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <ctime>
# include <string>
# include <cerrno>
# include <fcntl.h>
# include <cstddef>
# include <cstring>
# include <fstream>
# include <signal.h>
# include <unistd.h>
# include <stdexcept>
# include <sys/wait.h>
# include "ConfigServer.hpp"
# include "ConfigLocation.hpp"

# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define RESET   "\033[0m"

class CGI
{
	public:
		/*	Default	*/
		CGI();
		CGI(std::string const &request);
		CGI(CGI const &copy);
		~CGI();
		CGI&	operator=(CGI const &copy);

		ssize_t	GetBodySize( void );
		void	SetTimeStart(std::time_t time);
		void	ParseHeaders(std::string const &request);
		void	ParseFirstLine(std::string const &request);
		void	SetEnvpCGI(ConfigLocation const &config, ConfigServer &configServer);

		void	LaunchCGI(ConfigLocation const &config, ConfigServer &configServer);

		pid_t		GetPidCgi();
		int			GetPipeFd();
		int			GetPipeInFd();
		std::string	GetCgiBody();

	private:
		pid_t	_pid;
		char	**_envp;
		int		_pipeFd;
		int		_pipeInFd;
		std::string	_inter;
		std::time_t	_cgiStart;
		std::string	_scriptPath;
		std::map<std::string, std::string>	_stock;

};

void	ClearSpace(std::string &str);
std::string	return_file_length(size_t length);

#endif