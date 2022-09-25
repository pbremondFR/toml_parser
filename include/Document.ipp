/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.ipp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 14:04:10 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/25 14:09:36 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Document.hpp"

Document::e_value_type
	Document::_guessValueType(std::string::const_iterator it, std::string::const_iterator const& end)
{
	if (*it == '\"')
		return STRING;
	else if (*it == 't' || *it == 'f')
		return BOOL;
	else if (*it == '[')
		return ARRAY;
	else if (isdigit(*it) || *it == '-')
	{
		if (std::find(it, end, '.') != end)
			return FLOAT;
		else
			return INT;
	}
	else
		return UNDEF;
}

Value&			Document::at(std::string const& key)
{
	for (std::vector<Value>::iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	throw (std::out_of_range("TOML::Document::at(): std::out_of_range"));
}

Value&			Document::operator[](std::string const& key)
{
	std::vector<Value>::iterator it;
	for (it = _root._hashmap.begin(); it->_key != key && it != _root._hashmap.end(); ++it) ;
	return *it;
}

Value const&	Document::operator[](std::string const& key) const
{
	std::vector<Value>::const_iterator it;
	for (it = _root._hashmap.begin(); it->_key != key && it != _root._hashmap.end(); ++it) ;
	return *it;
}

bool	Document::parse(std::string const& path)
{
	if (_isParsed)
		return false;
	_path = path;
	return this->parse();
}

bool	Document::parse()
{
	if (_isParsed)
		return false;
	
	std::ifstream	fs(_path.c_str());
	if (!fs.is_open())
		return false; // TODO: Exception handling

	std::size_t	lineNum = 1;
	for (std::string line; std::getline(fs, line); ++lineNum)
	{
		std::string::const_iterator	it = line.begin();
		_skipWhitespaces(it, line.end());
		if (it == line.end() || *it == '#') { // ignore empty lines
			continue;
		}
		
		else if (*it == '[')
		{
			_parseGroup(++it, line, lineNum);
		}
		// Insert quoted keys, groups tables, etc, here
		else
		{
			_parseKeyValue(it, line, lineNum);
		}
	}
	_isParsed = true;
	return true;
}

// TODO: Finish me, TESTME
void	Document::_parseGroup(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
	typedef typename	std::string::const_iterator		str_const_it;
	
	str_const_it	it = std::find(src_it, line.end(), ']');
	if (it == line.end())
		throw(parse_error("Invalid group definition (missing `]')", lineNum));
	std::string	key(src_it, it);
	str_const_it keyIt;
	for (keyIt = key.begin(); keyIt != key.end(); ++keyIt)
	{
		if ((!_isBareKeyChar(*keyIt) && *keyIt != '.'))
			throw (parse_error("Illegal group key character", lineNum));
	}
	_currentGroup = &_root;
	keyIt = key.begin();
	str_const_it dot = std::find(keyIt, str_const_it(key.end()), '.');
	str_const_it ItLastKey
		= (key.find_last_of('.') == std::string::npos ? keyIt : key.begin() + key.find_last_of('.') + 1);

	// Go through all subkeys, automatically create subgroups when it's legal.
	while (keyIt != ItLastKey)
	{
		std::string	subkey(keyIt, dot);
		if (subkey.length() == 0)
			throw (parse_error("Empty key", lineNum));
		_currentGroup = _currentGroup->_getOrAddSubtable(Value(subkey));
		if (_currentGroup == NULL)
			throw (parse_error("Redeclared key", lineNum));
		keyIt = dot + 1;
		dot = std::find(keyIt, str_const_it(key.end()), '.');
	}
	// Parse last key
	std::string lastKey (ItLastKey, str_const_it(key.end())); // mfw no cend()
	if (lastKey.length() == 0)
		throw (parse_error("Empty key", lineNum));
	_currentGroup = _currentGroup->group_addValue(Value(lastKey));
	if (_currentGroup == NULL)
		throw (parse_error("Redeclared key", lineNum));
	_currentGroup->_undefinedGroup = false;
}

void	Document::_parseKeyValue(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
	typedef typename	std::string::const_iterator		str_const_it;
	
	// Get the key
	str_const_it	it = src_it;
	for (; it != line.end() && _isBareKeyChar(*it); ++it) ;
	if (!_isSpace(*it) && *it != '#' && *it != '=')
		throw (parse_error("Illegal key character at line ", lineNum));
	std::string	key(src_it, it);

	// Skip over the = sign but make sure it's there
	_skipWhitespaces(it, line.end());
	if (*it != '=')
		throw (parse_error("Missing `=' character after key at line ", lineNum));
	_skipWhitespaces(++it, line.end());

	// Get and parse the value
	switch (_guessValueType(it, line.end()))
	{
		case STRING:
		{ // TODO: unicode, yes or no ? Escaping as well
			str_const_it last = std::find(++it, str_const_it(line.end()), '\"');
			if (last != line.end())
				_currentGroup->group_addValue( Value(key, std::string(it, last)) );
			else
				throw (parse_error("Missing `\"' at the end of a string at line ", lineNum));
			_skipWhitespaces(last, line.end());
			if (last != line.end() && *last != '#' && *last != '\"')
				throw (parse_error("Illegal character after end of string", lineNum));
			break;
		}
		case INT:
		{
			Value::int_type	val;
			std::sscanf(it.base(), "%ld", &val); // Just rewrite your own func, dude
			for (; it != line.end() && (isdigit(*it) || *it == '-'); ++it) ;
			_skipWhitespaces(it, line.end());
			if (it != line.end() && *it != '#')
				throw (parse_error("Illegal integer value", lineNum));
			_currentGroup->group_addValue(Value(key, val, Value::T_INT));
			break;
		}
		case FLOAT:
		{
			Value::float_type	val;
			std::sscanf(it.base(), "%lf", &val); // Just rewrite your own func, dude
			for (; it != line.end() && (isdigit(*it) || *it == '-' || *it == '.'); ++it) ;
			_skipWhitespaces(it, line.end());
			if (it != line.end() && *it != '#')
				throw (parse_error("Illegal floating point value", lineNum));
			_currentGroup->group_addValue(Value(key, val, Value::T_FLOAT));
			break;
		}
		case BOOL:
		{ // FIXME, Shit code but I'm tired
			Value::bool_type	val;
			str_const_it last = std::find(it, str_const_it(line.end()), 'e');
			std::string	tmp(it, ++last);
			if (tmp != "true" && tmp != "false") // yeah that bugs
				throw (parse_error("Illegal boolean value", lineNum));
			_skipWhitespaces(last, line.end());
			if (last != line.end() && *last != '#')
				throw (parse_error("Illegal boolean value", lineNum));
			val = (tmp == "true" ? true : false);
			_currentGroup->group_addValue(Value(key, val, Value::T_BOOL));
			break;
		}
		case DATE:
			std::cout << "date case" << std::endl;
			break;
		case ARRAY:
			std::cout << "array case" << std::endl;
			break;
		case UNDEF:
			throw (parse_error("Illegal or missing value", lineNum));
	}
}