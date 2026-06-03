/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 15:35:11 by seully            #+#    #+#             */
/*   Updated: 2026/06/02 18:17:45 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Cgi.hpp"

CGI::CGI()
{}

CGI::CGI(CGI const &copy)
{}

CGI::~CGI()
{}

CGI&	CGI::operator=(CGI const &copy)
{}

std::string	CGI::Execute(std::string const &request, ConfigServer const &config, char **envp)
{}