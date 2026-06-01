/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:10:16 by strieste          #+#    #+#             */
/*   Updated: 2026/05/26 13:49:32 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

class Client
{
	public:
		Client();
		Client(int fd);
		Client(Client const &copy);
		~Client();
		Client&	operator=(Client const &copy);

		int	GetFd( void );
		int	GetIdClient( void );

		void	SetFd(int fd);
		void	SetIdClient(int id);

	private:
		int	_fd;
		int	_idClient;
};

#endif