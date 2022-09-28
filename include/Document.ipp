/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.ipp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 14:04:10 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 07:10:33 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Document.hpp"

// ============================================================================================== //
// --------------------------------------- PUBLIC METHODS --------------------------------------- //
// ============================================================================================== //

Value::e_type
	Document::_guessValueType(std::string::const_iterator it, std::string::const_iterator const& end)
{
	if (*it == '\"')
		return Value::T_STRING;
	else if (*it == 't' || *it == 'f')
		return Value::T_BOOL;
	else if (*it == '[')
		return Value::T_ARRAY;
	else if (isdigit(*it) || *it == '-')
	{
		if (std::find(it, end, '.') != end)
			return Value::T_FLOAT;
		else
			return Value::T_INT;
	}
	else
		return Value::T_UNDEF;
}

Value&			Document::at(std::string const& key)
{
	for (std::vector<Value>::iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	throw (std::out_of_range("TOML::Document::at(): std::out_of_range"));
}

Value const&	Document::at(std::string const& key) const
{
	for (std::vector<Value>::const_iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	throw (std::out_of_range("TOML::Document::at(): std::out_of_range"));
}

Value&			Document::operator[](std::string const& key) noexcept
{
	std::vector<Value>::iterator it;
	for (it = _root._hashmap.begin(); it->_key != key && it != _root._hashmap.end(); ++it) ;
	return *it;
}

Value const&	Document::operator[](std::string const& key) const noexcept
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
			_parseKeyValue(it, line, lineNum, fs);
		}
	}
	_isParsed = true;
	return true;
}

void	Document::_parseGroup(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
	typedef	std::string::const_iterator		str_const_it;
	
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

void	Document::_parseKeyValue(std::string::const_iterator src_it, std::string const& line, std::size_t& lineNum,
	std::ifstream& fs)
{
	(void)fs;
	typedef	std::string::const_iterator		str_const_it;
	
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
		case Value::T_STRING:
			_parseString(key, it, line, lineNum);
			break;
		case Value::T_INT:
		{
			Value::int_type	val;
			#ifdef __linux
				std::sscanf(it.base(), "%ld", &val); // Just rewrite your own func, dude
			#else
				std::sscanf(it.base(), "%lld", &val); // Just rewrite your own func, dude
			#endif
			for (; it != line.end() && (isdigit(*it) || *it == '-'); ++it) ;
			if (_hasNonWhitespace(it, line.end()))
				throw (parse_error("Illegal floating point value", lineNum));
			_currentGroup->group_addValue(Value(key, val, Value::T_INT));
			break;
		}
		case Value::T_FLOAT:
		{
			Value::float_type	val;
			std::sscanf(it.base(), "%lf", &val); // Just rewrite your own func, dude
			for (; it != line.end() && (isdigit(*it) || *it == '-' || *it == '.'); ++it) ;
			if (_hasNonWhitespace(it, line.end()))
				throw (parse_error("Illegal floating point value", lineNum));
			_currentGroup->group_addValue(Value(key, val, Value::T_FLOAT));
			break;
		}
		case Value::T_BOOL:
		{ // FIXME, Shit code but I'm tired
			Value::bool_type	val;
			str_const_it last = std::find(it, str_const_it(line.end()), 'e');
			std::string	tmp(it, ++last);
			if ((tmp != "true" && tmp != "false") || _hasNonWhitespace(last, line.end()))
				throw (parse_error("Illegal boolean value", lineNum));
			val = (tmp == "true" ? true : false);
			_currentGroup->group_addValue(Value(key, val, Value::T_BOOL));
			break;
		}
		case Value::T_DATE:
			std::cout << "date case" << std::endl;
			break;
		case Value::T_ARRAY: // TODO: Do that next. It's the most important.
			std::cout << "array case" << std::endl;
			break;
		// case Value::T_UNDEF:
		default:
			throw (parse_error("Illegal or missing value", lineNum));
	}
}

void	Document::_parseCompactEscapeSequence(std::string::iterator& it, std::string& raw_str,
	char escaped) const
{
	const char	c[] =		{ '\"',  'b',  't',  'n',  'f',  'r', '/', '\\' };
	const char*	replace[] =	{ "\"", "\b", "\t", "\n", "\f", "\r", "/", "\\" };

	int	i;
	for (i = 0; i < 8; ++i)
		if (escaped == c[i])
			break;
	raw_str.replace(it, it + 2, replace[i]);
	if (c[i] == '\\')
		++it;
}

// TODO: One the one hand, this is missing unicode escaping capabilities.
// On the other hand, this is very funny. And you can just use your emoji keyboard.
void	Document::_parseEscapedUnicode(std::string::iterator& it, std::string& raw_str, std::size_t lineNum) const
{
	if (!isxdigit(*(it + 2)) || !isxdigit(*(it + 3)) || !isxdigit(*(it + 4)) || !isxdigit(*(it + 5)))
		throw (parse_error("Illegal unicode escape sequence", lineNum));
	raw_str.replace(it, it + 6, "🗿");
	// Unicode escaping is not currently supported. As such, any unicode escape code will translate to
	// the 🗿 emoji.
}

void	Document::_parseString(std::string const& key, std::string::const_iterator it,
	std::string const& line, std::size_t lineNum)
{
	typedef	std::string::const_iterator		str_const_it;

	std::string	raw_str(it, line.end());
	// Loop through all backslashes, replacing them each time
	for (std::string::iterator it = std::find(raw_str.begin(), raw_str.end(), '\\');
		it != raw_str.end();
		it = std::find(it, raw_str.end(), '\\'))
	{
		if (const char *c = std::strchr("\"btnfr/\\", *(it + 1))) // If \", \b, \t, \n, \f, \r, \/, or \\.
			_parseCompactEscapeSequence(it, raw_str, *c);
		else if (*(it + 1) == 'u') // Unicode escaping
			_parseEscapedUnicode(it, raw_str, lineNum);
		else
			throw (parse_error(std::string("Illegal escaped character (\\")+ *(it+1) + ')', lineNum));
	}
	
	str_const_it last_quote = raw_str.begin() + raw_str.find_last_of('\"');
	if (last_quote == raw_str.end())
		throw (parse_error("Missing `\"' at the end of a string", lineNum));
	if (_hasNonWhitespace(++last_quote, raw_str.end()))
		throw (parse_error("Illegal character after end of string", lineNum));
	raw_str.erase(last_quote - 1, raw_str.end());
	raw_str.erase(0, 1);

	_currentGroup->group_addValue( Value(key, raw_str) );
}

// Expects it to be pointing to a '['
// void	_parseArray(std::string const& key, std::string::const_iterator& it,
// 	std::string const& line, std::size_t& lineNum, std::ifstream& fs)
// {
// 	_skipWhitespaces(++it, line.end());
// }
