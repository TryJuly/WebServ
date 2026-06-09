/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:10:16 by strieste          #+#    #+#             */
/*   Updated: 2026/06/08 14:08:56 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
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
		int	GetFdConfigServer( void );
		int	GetIndexConfigClient();
		
		void	SetFdConfigServer(int fd);

		void	SetFd(int fd);
		void	SetIdClient(int id);
		void	SetIndexConfigServer(int index);


	private:
		int	_fd;
		int	_idClient;
		int _fdConfigServer;
		int	_indexConfigServer;
		std::string	_buffer;
};

#endif