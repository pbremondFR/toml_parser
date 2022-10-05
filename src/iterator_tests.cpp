/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iterator_tests.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/05 17:30:12 by pbremond          #+#    #+#             */
/*   Updated: 2022/10/05 20:07:11 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include "tests.hpp"

#include "../include/ansi_color.h"
#include "../include/Document.hpp"

using namespace TOML;

using std::cout;
using std::cerr;
using std::endl;

int	iterator_tests(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << F_BRED("Bruh") << std::endl;
		return 1;
	}
	newtest("Document::iterator");
	{
		Document	doc(argv[1]);
		doc.parse();

		Document::iterator	it = doc.begin();
		for (; it != doc.end(); ++it)
		{
			cout << *it << endl << endl;
		}
	}
	// newtest("Document::const_iterator");
	// {
	// 	Document	doc(argv[1]);
	// 	doc.parse();

	// 	Document::const_iterator	it = doc.begin();
	// 	for (; it != doc.end(); ++it)
	// 	{
	// 		cout << *it << endl << endl;
	// 	}
	// }
	return 0;
}
