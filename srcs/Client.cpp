/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 13:12:35 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 10:32:38 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../header/Client.hpp"

Client::Client()
{
	_fd = -1;
	return ;
}

Client::Client(int fd)
{
	_fd = fd;
	return ;
}

Client::Client(Client const &copy)
{
	(*this) = copy;
	return ;
}

Client::~Client()
{}

Client&	Client::operator=(Client const &copy)
{
	if (this != &copy)
		_fd = copy._fd;
	return (*this);
}

void	Client::SetIndexConfigServer(int index)
{
	_indexConfigServer = index;
	return ;
}

int	Client::GetIndexConfigClient()
{ return (_indexConfigServer); }

int	Client::GetFd()
{ return (_fd); }

void	Client::SetFd(int fd)
{
	_fd = fd;
	return ;
}
void	Client::SetIdClient(int id)
{
	_idClient = id;
	return ;
}

int	Client::GetIdClient( void )
{ return (_idClient); }
