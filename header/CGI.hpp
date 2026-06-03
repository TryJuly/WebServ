/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:37:00 by seully            #+#    #+#             */
/*   Updated: 2026/06/02 18:17:36 by seully           ###   ########.fr       */
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