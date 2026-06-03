/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:15:09 by strieste          #+#    #+#             */
/*   Updated: 2026/06/03 13:15:23 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <iostream>
#include <stdexcept>
#include "../header/Server.hpp"

void	PrintFullConfigServer(Server &server);

int	main(int ac, char **av)
{
	(void)ac;
	(void)av;
	try {
		if (ac > 2)
			throw (std::invalid_argument("Error: Too many arguments."));
		Server server(ac, av);
		std::cout << server.GetNumberConfig() << std::endl;

		PrintFullConfigServer(server);

		// server.StartServer();
	}
	catch(const std::exception& e) {
		std::cerr << RED << e.what() << RESET << std::endl;
		return (1);
	}
	std::cout << GREEN << "###	FINISH	###" << RESET << std::endl;
	return (0);
}

void	PrintFullConfigServer(Server &server)
{
	for (int i = 0; i < server.GetNumberConfig(); i++) {
		ConfigServer &conf = server.GetConfigServer(i);
		std::cout << "\n###	Config server : " << i + 1 << "	###\n" << std::endl;
		std::cout << "Port: :" << conf.GetPort() << ":" << std::endl;
		std::cout << "Socket: :" << conf.GetSocket() << ":" << std::endl;
		std::cout << "Max body: :" << conf.GetMaxBodySize() << ":" << std::endl;
		std::cout << "Num location: :" << conf.GetNumberLocation() << ":" << std::endl;
		std::cout << "Root path: :" << conf.GetRoot() << ":" << std::endl;
		std::cout << "Index: :" << conf.GetIndex() << ":" << std::endl;
		std::cout << "Name: :" << conf.GetServerName() << ":" << std::endl;

		std::cout << "\n###	Location part	###" << std::endl;

		for (int index = 0; index < conf.GetNumberLocation(); index++) {
			ConfigLocation &location = conf.GetConfigLocation(index);
			
			std::cout << "Path: :" << location.GetPath() << ":" << std::endl;
			std::cout << "root: :" << location.GetRoot() << ":" << std::endl;
			std::cout << "index: :" << location.Getindex() << ":" << std::endl;
			std::cout << "auto index: :" << location.GetAutoIndex() << ":" << std::endl;
			std::cout << "redir: :" << location.GetRedir() << ":" << std::endl;
			std::cout << "upload path: :" << location.GetUpload() << ":" << std::endl;
			std::cout << "methods get: " << location.GetBoolGet() << ":" << std::endl;
			std::cout << "methods post: " << location.GetBoolPost() << ":" << std::endl;
			std::cout << "methods delete: " << location.GetBoolDelete() << ":" << std::endl;
			std::cout << "\n###	END location: " << index + 1 << "	###\n" << std::endl;
		}
		std::cout << "\n###	END : ###\n" << std::endl;
	}
}