/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:37:00 by seully            #+#    #+#             */
/*   Updated: 2026/06/03 15:16:11 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <unistd.h>
# include "ConfigServer.hpp"

class CGI
{
	public:
		/*	Default	*/
		CGI();
		CGI(CGI const &copy);
		~CGI();
		CGI&	operator=(CGI const &copy);

		std::string	Execute(std::string const &request, ConfigServer const &config);
	private:
};

#endif