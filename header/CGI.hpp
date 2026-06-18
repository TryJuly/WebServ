/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:37:00 by seully            #+#    #+#             */
/*   Updated: 2026/06/18 10:41:06 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

#include <ctime>
# include <map>
# include <string>
# include <cerrno>
# include <cstddef>
# include <cstring>
# include <fstream>
# include <unistd.h>
# include <stdexcept>
# include <sys/wait.h>
# include "ConfigServer.hpp"
# include "ConfigLocation.hpp"
#include <fcntl.h>
#include <signal.h>

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

		// std::string	Execute(ConfigLocation const &config);
		void	ParseFirstLine(std::string const &request);
		void	ParseHeaders(std::string const &request);
		void	SetEnvpCGI(ConfigLocation const &config);
		void	SetTimeStart(std::time_t time);
		ssize_t	GetBodySize( void );

		void	LaunchCGI(ConfigLocation const &config);

		pid_t	GetPidCgi();
		int		GetPipeFd();

	private:
		std::map<std::string, std::string>	_stock;
		std::string	_inter;
		std::string	_scriptPath;
		char	**_envp;
		std::time_t	_cgiStart;

		pid_t	_pid;
		int		_pipeFd;
};

void	ClearSpace(std::string &str);
std::string	return_file_length(size_t length);

#endif