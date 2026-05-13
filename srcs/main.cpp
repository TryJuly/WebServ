/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:15:09 by strieste          #+#    #+#             */
/*   Updated: 2026/05/12 18:01:01 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/types.h>	// Listen function()
#include <poll.h>	// poll() function like epoll() for Linux or kqueue() for Macos

int	main(int ac, char **av)
{
	try {
		if (ac != 2)
			throw (std::invalid_argument("Error: Invalide arguments."));
		while (true) {
		
	}
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}