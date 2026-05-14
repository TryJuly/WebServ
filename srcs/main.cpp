/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:15:09 by strieste          #+#    #+#             */
/*   Updated: 2026/05/14 17:07:32 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <Server.hpp>
#include <iostream>

int	main(int ac, char **av)
{
	(void)ac;
	(void)av;
	try {
		if (ac != 2)
			throw (std::invalid_argument("Error: Invalide arguments."));
		Server	server;
		server.StartServer();
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}