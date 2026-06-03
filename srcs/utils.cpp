/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: strieste <strieste@student.42.ch>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 17:50:30 by strieste          #+#    #+#             */
/*   Updated: 2026/06/01 14:33:31 by strieste         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ConfigLocation.hpp"
#include <string>

std::string	trim(std::string const &str)
{
	size_t	start = str.find_first_not_of(" \t");
	size_t	end = str.find_last_not_of(" \t");
	if (start == std::string::npos)
		return ("");
	return (str.substr(start, end - start + 1));
}

void	ClearSpace(std::string &str)
{
	int i = 0;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	std::string res;
	for (int j = i; str[j]; j++){
		res.push_back(str[j]);
	}
	str = res;
	return ;
}

int	EndChunk(std::vector<std::string> &fileArray, unsigned int index)
{
	int	balance = 0;
	for (unsigned int i = index; i < fileArray.size(); i++) {
		if (fileArray[i][0] == '#')
			continue ;
		for (int j = 0; fileArray[i][j]; j++) {
			if (fileArray[i][j] == '{')
				balance++;
			else if (fileArray[i][j] == '}') {
				balance--;
				if (balance == 0)
					return (i + 1);
			}
		}
	}
	return (-1);
}
