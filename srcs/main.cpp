/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:15:09 by strieste          #+#    #+#             */
/*   Updated: 2026/05/18 15:25:14 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <iostream>
#include "../header/Server.hpp"

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