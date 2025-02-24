/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pipolint <pipolint@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 20:03:09 by pipolint          #+#    #+#             */
/*   Updated: 2025/02/24 20:11:45 by pipolint         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <algorithm>

void	capitalize(char& c)
{
	if (c >= 'A' && c <= 'Z')
		c += 32;
}

int main(void)
{
	std::string str("hello ThErE\n");
	std::cout << "Str: " << str << "\n";
	std::for_each(str.begin(), str.end(), capitalize);
	std::cout << "Str: " << str << "\n";
}