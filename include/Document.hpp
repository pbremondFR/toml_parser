/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/24 18:51:21 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "Value.hpp"

namespace TOML
{

class parse_error : public std::exception
{
	private:
		std::string			_msg;
	public:
		parse_error(std::string const& what, std::size_t line = 0)
		{
			std::stringstream ss;
			ss << what;
			if (line != 0)
				ss << " at line " << line;
			_msg = ss.str();
		}
		parse_error(parse_error const&) {}
		virtual const char	*what() const throw() { return _msg.c_str(); }
		virtual ~parse_error() throw() {}
};

// TODO: iterators
class Document
{
	private:
		Value			_root;
		Value			*_currentGroup;
		std::ifstream	_fs;
		bool			_isParsed;

	private:
		static inline bool	_isSpace(char c) { return (c == 0x09 || c == 0x20); }
		static inline void	_skipWhitespaces(std::string::const_iterator& it, std::string::const_iterator const& end)
		{
			for (; it != end && _isSpace(*it); ++it) ;
			// do { ++it; } while (it != end && _isSpace(*it));
			
		}
		static inline bool	_isBareKeyChar(char c) { return isascii(c) && (isupper(c) || islower(c) || isdigit(c) || c == '-' || c == '_'); }

		enum e_value_type {
			STRING,
			INT,
			FLOAT,
			BOOL,
			DATE,
			ARRAY,
			UNDEF
		};
		static inline e_value_type	_guessValueType(std::string::const_iterator it, std::string::const_iterator const& end)
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
		void	_parseKeyValue(std::string::const_iterator it, std::string const& line, std::size_t lineNum);
		void	_parseGroup(std::string::const_iterator it, std::string const& line, std::size_t lineNum);
	
	public:
		Document() : _root(Value("")), _currentGroup(&_root), _isParsed(false) {} // An empty key'd GROUP node is the root
		Document(std::string const& path) : _root(Value("")),	_currentGroup(&_root), _fs(path.c_str()),	_isParsed(false) {}
		Document(const char *path)		  : _root(Value("")),	_currentGroup(&_root), _fs(path),			_isParsed(false) {}
		Document(Value const& value)	  : _root(value),		_currentGroup(&_root), _fs(), _isParsed(true) {} 
		~Document()
		{
			if (_fs.is_open())
				_fs.close();
		}

		Value&			at(std::string const& key)
		{
			std::vector<Value>::iterator it;
			for (it = _root._hashmap.begin();
				it != _root._hashmap.end();
				++it)
			{
				if (it->_key == key)
					return *it;
			}
			throw (std::out_of_range("TOML::Document::at -> std::out_of_range"));
		}
		Value const&	at(std::string const& key) const
		{
			return (at(key)); // TESTME: make sure this isn't recursive...
		}
		Value&	operator[](std::string const& key)
		{
			std::vector<Value>::iterator it;
			for (it = _root._hashmap.begin();
				it->_key != key && it != _root._hashmap.end();
				++it) ;
			return *it;
		}

		inline void	open(const char *path, std::ios_base::openmode mode = std::ios_base::in)
		{
			_fs.open(path, mode);
		}
		bool	parse();
};

// ============================================================================================== //
// ------------------------------------------          ------------------------------------------ //
// ============================================================================================== //

bool	Document::parse()
{
	if (_isParsed)
		return false;
	
	std::size_t	lineNum = 1;
	for (std::string line; std::getline(_fs, line); ++lineNum)
	{
		std::string::const_iterator	it = line.begin();
		_skipWhitespaces(it, line.end());
		if (it == line.end() || *it == '#') { // ignore empty lines
			continue;
		}
		
		else if (*it == '[')
		{
			// TODO, group logic
			_parseGroup(++it, line, lineNum);
		}
		// Insert quoted keys, table groups, etc, here
		else // key/value logic
		{
			_parseKeyValue(it, line, lineNum);
		}
	}
	_isParsed = true;
	return true;
}

void	Document::_parseGroup(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
	std::string::const_iterator	it = std::find(src_it, line.end(), ']');
	if (it == line.end())
		throw(parse_error("Invalid group definition (missing `]')", lineNum));
	std::string	key(src_it, it);
	if (key.length() == 0)
		throw (parse_error("Empty group declaration", lineNum));
	std::string::const_iterator keyIt;
	for (keyIt = key.begin(); keyIt != key.end(); ++keyIt) {
		if ((!_isBareKeyChar(*keyIt) && *keyIt != '.'))
			throw (parse_error("Illegal group key character", lineNum));
	}
	keyIt = key.begin();
	std::string::const_iterator	dot = std::find(keyIt, std::string::const_iterator(key.end()), '.');
	do
	{
		std::string	subkey(keyIt, dot);
		std::cout << "#" << subkey << std::endl;
		_currentGroup = _currentGroup->group_addValue(Value(subkey));
		if (_currentGroup == NULL)
			throw (parse_error("Illegal group declaration", lineNum));
		keyIt = (dot == key.end() ? dot : dot + 1);
		dot = std::find(keyIt, std::string::const_iterator(key.end()), '.');
		// std::cout << "DEBUG ME" << std::endl;
	} while (keyIt != key.end());
	// std::cout << "DEBUG ME 2" << std::endl;
	_currentGroup = &_root;
}

void	Document::_parseKeyValue(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
	// Get the key
	std::string::const_iterator	it = src_it;
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
			std::string::const_iterator	last = std::find(++it,
					std::string::const_iterator(line.end()), '\"');
			if (last != line.end())
				_root.group_addValue( Value(key, std::string(it, last)) );
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
			_root.group_addValue(Value(key, val, Value::T_INT));
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
			_root.group_addValue(Value(key, val, Value::T_FLOAT));
			break;
		}
		case BOOL:
		{ // FIXME, Shit code but I'm tired
			Value::bool_type	val;
			std::string::const_iterator	last = std::find(it,
					std::string::const_iterator(line.end()), 'e');
			std::string	tmp(it, ++last);
			if (tmp != "true" && tmp != "false") // yeah that bugs
				throw (parse_error("Illegal boolean value", lineNum));
			_skipWhitespaces(last, line.end());
			if (last != line.end() && *last != '#')
				throw (parse_error("Illegal boolean value", lineNum));
			val = (tmp == "true" ? true : false);
			_root.group_addValue(Value(key, val, Value::T_BOOL));
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

} // namespace TOML
