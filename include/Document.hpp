/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/24 07:57:41 by pbremond         ###   ########.fr       */
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

class Document
{
	private:
		Value			_root;
		std::ifstream	_fs;
		bool			_isParsed;

		static inline bool	_isSpace(char c) { return (c == 0x09 || c == 0x20); }
		static inline void	_skipWhitespaces(std::string::const_iterator& it, std::string::const_iterator const& end)
		{
			for (; it != end && _isSpace(*it); ++it) ;
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
	
	public:
		Document() : _root(Value("")), _isParsed(false) {} // An empty key'd GROUP node is the root
		Document(std::string const& path) : _root(Value("")), _fs(path.c_str()), _isParsed(false) {}
		Document(const char *path)		  : _root(Value("")), _fs(path), _isParsed(false) {}
		~Document()
		{
			if (_fs.is_open())
				_fs.close();
		}

		Value&	operator[](int i)
		{
			return (_root._hashmap[i]);
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
	std::string	line;
	for (; std::getline(_fs, line); ++lineNum)
	{
		std::cout << lineNum << std::endl;
		std::string::const_iterator	it = line.begin();
		_skipWhitespaces(it, line.end());
		if (it == line.end()) { // ignore empty lines
			std::cout << "only whitespaces line" << std::endl;
			continue;
		}
		
		else if (*it == '[')
		{
			// TODO, group logic
		}
		// Insert quoted keys, table groups, etc, here
		else // key/value logic
		{
			_parseKeyValue(it, line, lineNum);
		}
		// if (line.empty())	
		// 	break;
	}
	return true;
}

void	Document::_parseKeyValue(std::string::const_iterator it, std::string const& line, std::size_t lineNum)
{
	// Get the key
	std::string::const_iterator	tmp_it = it;
	for (; tmp_it != line.end() && _isBareKeyChar(*tmp_it); ++tmp_it) ;
	if (!_isSpace(*tmp_it) && *tmp_it != '#' && *tmp_it != '=')
		throw (parse_error("Illegal key character at line ", lineNum));
	std::string	key(it, tmp_it);

	// Skip over the = sign but make sure it's there
	_skipWhitespaces(tmp_it, line.end());
	if (*tmp_it != '=')
		throw (parse_error("Missing `=' character after key at line ", lineNum));
	_skipWhitespaces(++tmp_it, line.end());

	// Get and parse the value
	switch (_guessValueType(tmp_it, line.end()))
	{
		case STRING:
		{ // TODO: unicode, yes or no ? And some more stuff
			std::string::const_iterator	last = std::find(++tmp_it,
					std::string::const_iterator(line.end()), '\"');
			if (last != line.end())
				_root.group_addValue( Value(key, std::string(tmp_it, last)) );
			else
				throw (parse_error("Missing `\"' at the end of a string at line ", lineNum));
			break;
		}
		case INT:
		{
			Value::int_type	val;
			std::sscanf(tmp_it.base(), "%ld", &val); // Just rewrite your own func, dude
			for (; tmp_it != line.end() && isdigit(*tmp_it); ++tmp_it) ;
			_skipWhitespaces(tmp_it, line.end());
			if (tmp_it != line.end())
				throw (parse_error("Illegal integer value", lineNum));
			_root.group_addValue(Value(key, val, Value::T_INT));
			break;
		}	
		case FLOAT:
			std::cout << "float case" << std::endl;
			break;
		case BOOL:
			std::cout << "bool case" << std::endl;
			break;
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
