/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:10:16 by strieste          #+#    #+#             */
/*   Updated: 2026/05/26 13:15:43 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

class Client
{
	public:
		Client();
		Client(Client const &copy);
		~Client();
		Client&	operator=(Client const &copy);

		int	GetFd();
		void	SetFd(int fd);

	private:
		int	_fd;
};

#endif