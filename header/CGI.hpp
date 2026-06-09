/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:37:00 by seully            #+#    #+#             */
/*   Updated: 2026/06/09 14:10:26 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <string>
# include <cerrno>
# include <cstddef>
# include <unistd.h>
# include <cstring>
# include <stdexcept>
# include <sys/wait.h>
# include "ConfigServer.hpp"
# include "ConfigLocation.hpp"

class CGI
{
	public:
		/*	Default	*/
		CGI();
		CGI(std::string const &request);
		CGI(CGI const &copy);
		~CGI();
		CGI&	operator=(CGI const &copy);

		std::string	Execute(ConfigLocation const &config);

	private:
		// char **_envp;
		std::map<std::string, std::string>	_stock;
};

void	ClearSpace(std::string &str);

#endif