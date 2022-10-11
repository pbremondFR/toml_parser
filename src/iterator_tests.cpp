/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iterator_tests.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/05 17:30:12 by pbremond          #+#    #+#             */
/*   Updated: 2022/10/11 02:35:33 by pbremond         ###   ########.fr       */
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

void	print_recursive(TOML::Document const& doc, std::string ident = "")
{
	for (Document::const_iterator it = doc.begin(); it != doc.end(); ++it)
	{
		if (it->isGroup()) {
			cout << ident + '\t' << "[" + it->key() + "]" << endl;
			print_recursive(Document(*it), ident + '\t');
		}
		else if (it->isArray() && it->Array().type() == T_GROUP)
		{
			for (Document::array_type::const_iterator arr_it = it->Array().begin();
				arr_it != it->Array().end();
				++arr_it)
			{
				cout << ident + '\t' << "[[" + it->key() + "]]" << endl;
				print_recursive(Document(*arr_it), ident + '\t');
				cout << endl;
			}
		}
		else
			std::cout << ident << it->key() << " = " << *it << std::endl;
	}
}

// TODO: switch to const_iterators when possible
 void	_insert_toml_doc_in_group(TOML::Value& target, TOML::Document& src)
{
	for (TOML::Document::iterator it = src.begin(); it != src.end(); ++it)
	{
		if (target.group_addValue(*it) == NULL)
			throw TOML::parse_error("Duplicate value when trying to include TOML file");
	}
}

bool	include_directive(TOML::Value& target, TOML::Value include)
{
	assert(target.type() == TOML::T_GROUP);

	if (!include.isArray() || include.Array().type() != TOML::T_STRING)
		throw std::runtime_error("Configuration file: illegal include directive"); // TODO: Check the type of exception thrown

	for (TOML::Value::array_type::iterator it = include.Array().begin();
		it != include.Array().end();
		++it)
	{
		TOML::Document	include_doc(it->Str());
		try {
			include_doc.parse();
			_insert_toml_doc_in_group(target, include_doc);
		}
		catch (std::exception const& e) {
			std::cerr << "Error when trying to include " << *it << ": " << e.what() << std::endl;
			return false;
		}
	}
	target.erase(include.key());
	return true;
}

void	print_iterative(TOML::Document& doc);
void	print_value(TOML::Value& val);


void	print_array(TOML::Value& array)
{
	cout << REDB << array.key() << ": " << RESET << endl;
	for (Document::array_type::iterator j = array.Array().begin();
		j != array.Array().end();
		++j)
	{
		print_value(*j);
	}
	cout << REDB << "END " << array.key() << RESET << endl;
}

void	print_value(TOML::Value& val)
{
	if (val.type() == TOML::T_ARRAY)
	{
		print_array(val);
	}
	else if (val.type() == TOML::T_GROUP)
	{
		if (val.has("include"))
		{
			assert(include_directive(val, val["include"]) == true);
		}
		for (Value::group_iterator it = val.group_begin();
			it != val.group_end();
			++it)
		{
			print_value(*it);
		}
	}
	// else if (val.isFundamental()) {
	// 	cout << P_TYPE(val.type()) << endl;
	// 	val.set(1234);
	// 	cout << "\e[4m" << val.key() << RESET << " = " << val << endl << endl;
	// }
	else {
		cout << P_TYPE(val.type()) << endl;
		cout << "\e[4m" << val.key() << RESET << " = " << val << endl << endl;
	}
}

void	print_iterative(TOML::Document& doc)
{
	// Document::recursive_iterator it = doc.rec_begin();
	Document::iterator it = doc.begin();
	for (; it != doc.end(); ++it)
	{
		print_value(*it);
		// if (it->type() == TOML::T_ARRAY)
		// {
		// 	cout << it->key();
		// 	print_array(*it);
		// }
		// else
		// 	cout << it->key() << " = " << *it << endl << endl;
	}
}

int	iterator_tests(int argc, char const *argv[])
{
	if (argc < 3) {
		std::cerr << F_BRED("Bruh") << std::endl;
		return 1;
	}
	newtest("Document::iterator");
	{
		Document	doc(argv[2]);
		doc.parse();

		Document::iterator	it = doc.begin();
		for (; it != doc.end(); ++it)
		{
			cout << *it << endl << endl;
		}
	}
	newtest("Document::const_iterator");
	{
		Document	doc(argv[1]);
		doc.parse();

		Document::const_iterator	it = doc.begin();
		for (; it != doc.end(); ++it)
		{
			cout << *it << endl << endl;
		}
	}
	newtest("Erase");
	{
		Document	doc(argv[2]);
		doc.parse();

		if (doc.has("main"))
			doc.erase("main");
		else
			doc.erase(doc.begin()->key());
		Document::iterator	it = doc.begin();
		for (; it != doc.end(); ++it)
		{
			cout << *it << endl << endl;
		}
	}
	newtest("Print recursive");
	{
		Document	doc(argv[2]);
		doc.parse();

		print_recursive(doc);
	}
	newtest("Recursive iterators");
	{
		Document	doc(argv[2]);
		doc.parse();

		Document::recursive_iterator it = doc.rec_begin();
		for (; it != doc.rec_end();)
		{
			cout << it->key() << " = " << *it << endl << endl;
			++it;
		}
	}
	newtest("Why is that a float ?!");
	{
		Document	doc(argv[2]);
		doc.parse();

		cout << P_TYPE( doc["http"]["server"][0]["listen_port"].type() ) << endl;
		cout << doc["http"]["server"][0]["listen_port"] << endl;
	}
	newtest("Print with recursive iterators");
	{
		Document	doc(argv[2]);
		doc.parse();

		print_iterative(doc);
		print_recursive(doc);
	}
	return 0;
}
