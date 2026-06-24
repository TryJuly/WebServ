/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seully <seully@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 17:50:30 by strieste          #+#    #+#             */
/*   Updated: 2026/06/22 09:45:32 by seully           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <string>
# include <cstddef>
# include "../header/Server.hpp"
# include "../header/ConfigLocation.hpp"

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

std::string return_file_length(size_t length) {
    std::ostringstream len;
    len << length;
    return (len.str());
}

/*	Set Content Type for response	*/
std::string MimeType(std::string const &path)
{
	size_t	pos = path.rfind('.');
	if (pos == std::string::npos)
		return ("application/octet-stream");
	std::string ext = path.substr(pos);
	if (ext == ".html" || ext == "htm")
		return ("text/html");
	if (ext == ".css")
		return ("test/css");
	if (ext == ".js")
		return ("application/javascript");
	if (ext == ".png")
		return ("image/png");
	if (ext == ".jpg")
		return ("image.jpg");
	if (ext == ".ico")
		return ("image/x-icon");
	return ("application/octet-stream");
}