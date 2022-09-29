/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.ipp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 14:04:10 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 14:20:39 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Document.hpp"

// ============================================================================================== //
// --------------------------------------- PUBLIC METHODS --------------------------------------- //
// ============================================================================================== //

inline
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

inline
Value&			Document::at(std::string const& key)
{
	for (std::vector<Value>::iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	throw (std::out_of_range("TOML::Document::at(): std::out_of_range"));
}

inline
Value const&	Document::at(std::string const& key) const
{
	for (std::vector<Value>::const_iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	throw (std::out_of_range("TOML::Document::at(): std::out_of_range"));
}

inline
Value	Document::at_or(std::string const& key, Value val) const noexcept
{
	for (std::vector<Value>::const_iterator it = _root._hashmap.begin(); it != _root._hashmap.end(); ++it)
		if (it->_key == key)
			return *it;
	return val;
}

inline
Value&			Document::operator[](std::string const& key) noexcept
{
	std::vector<Value>::iterator it;
	for (it = _root._hashmap.begin(); it->_key != key && it != _root._hashmap.end(); ++it) ;
	return *it;
}

inline
Value const&	Document::operator[](std::string const& key) const noexcept
{
	std::vector<Value>::const_iterator it;
	for (it = _root._hashmap.begin(); it->_key != key && it != _root._hashmap.end(); ++it) ;
	return *it;
}

inline
bool	Document::parse(std::string const& path)
{
	if (_isParsed)
		return false;
	_path = path;
	return this->parse();
}

inline
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

inline
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

inline
void	Document::_parseKeyValue(std::string::const_iterator src_it, std::string& line, std::size_t& lineNum,
	std::ifstream& fs)
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
		case Value::T_STRING:
			_currentGroup->group_addValue( Value(key, _parseString(it, line.end(), lineNum)) );
			break;
		case Value::T_INT:
			_currentGroup->group_addValue( Value(key, _parseInt(it, line.end(), lineNum), Value::T_INT) );
			break;
		case Value::T_FLOAT:
			_currentGroup->group_addValue( Value(key, _parseFloat(it, line.end(), lineNum), Value::T_FLOAT) );
			break;
		case Value::T_BOOL:
			_currentGroup->group_addValue( Value(key, _parseBool(it, line.end(), lineNum), Value::T_BOOL) );
			break;
		case Value::T_DATE:
			std::cout << "date case" << std::endl;
			break;
		case Value::T_ARRAY:
			// std::cout << "Uncomment me you dipshit" << std::endl;
			_currentGroup->group_addValue( _parseArray(key, it, line, lineNum, fs) );
			break;
		// case Value::T_UNDEF:
		default:
			throw (parse_error("Illegal or missing value", lineNum));
	}
}

inline
Value::int_type	Document::_parseInt(str_const_it it, str_const_it end, std::size_t lineNum) const
{
	Value::int_type	val;
	#ifdef __linux
		std::sscanf(it.base(), "%ld", &val);
	#else
		std::sscanf(it.base(), "%lld", &val); // Just rewrite your own func, dude
	#endif
	for (; it != end && (isdigit(*it) || *it == '-'); ++it) ;
	if (_hasNonWhitespace(it, end))
		throw (parse_error("Illegal integer value", lineNum));
	return val;
}

inline
Value::float_type	Document::_parseFloat(str_const_it it, str_const_it end, std::size_t lineNum) const
{
	Value::float_type	val;
	std::sscanf(it.base(), "%lf", &val); // Just rewrite your own func, dude
	for (; it != end && (isdigit(*it) || *it == '-' || *it == '.'); ++it) ;
	if (_hasNonWhitespace(it, end))
		throw (parse_error("Illegal floating point value", lineNum));
	return val;
}

inline
Value::bool_type	Document::_parseBool(str_const_it it, str_const_it end, std::size_t lineNum) const
{ // FIXME, Shit code but I'm tired
	typedef	std::string::const_iterator		str_const_it;
	
	Value::bool_type	val;
	str_const_it last = std::find(it, str_const_it(end), 'e');
	std::string	tmp(it, ++last);
	if ((tmp != "true" && tmp != "false") || _hasNonWhitespace(last, end))
		throw (parse_error("Illegal boolean value", lineNum));
	val = (tmp == "true" ? true : false);
	return val;
}

inline
bool	Document::_parseCompactEscapeSequence(std::string::iterator& it, std::string& str,
	char escaped) const
{
	const char	c[] =		{ '\"',  'b',  't',  'n',  'f',  'r', '/', '\\' };
	const char*	replace[] =	{ "\"", "\b", "\t", "\n", "\f", "\r", "/", "\\" };
	const std::size_t n =	(sizeof(c) / sizeof(c[0]));

	std::size_t	i;
	for (i = 0; i < n; ++i) {
		if (escaped == c[i]) {
			str.replace(it, it + 2, replace[i]);
			break;
		}
	}
	return i < n; // Return true if character has been replaced
}

// TODO: One the one hand, this is missing unicode escaping capabilities.
// On the other hand, this is very funny. And you can just use your emoji keyboard.
inline
void	Document::_parseEscapedUnicode(std::string::iterator& it, std::string& raw_str, std::size_t lineNum) const
{
	if (!isxdigit(*(it + 2)) || !isxdigit(*(it + 3)) || !isxdigit(*(it + 4)) || !isxdigit(*(it + 5)))
		throw (parse_error("Illegal unicode escape sequence", lineNum));
	raw_str.replace(it, it + 6, "🗿");
	// Unicode escaping is not currently supported. As such, any unicode escape code will translate to
	// the 🗿 emoji.
}

inline
Value::string_type	Document::_parseString(str_const_it src_it, str_const_it end, std::size_t lineNum) const
{
	std::string	newstr(++src_it, end);
	std::string::iterator it = newstr.begin();
	for (; it != newstr.end() && *it != '\"'; ++it)
	{
		if (*it != '\\' || _parseCompactEscapeSequence(it, newstr, *(it + 1)) == true)
			continue;
		else if (*(it + 1) == 'u') // Unicode escaping
			_parseEscapedUnicode(it, newstr, lineNum);
		else
			throw (parse_error(std::string("Illegal escaped character (\\")+ *(it+1) + ')', lineNum));
	}
	if (it == newstr.end())
		throw parse_error("Missing `\"' at the end of string", lineNum);
	else if (_hasNonWhitespace(it + 1, newstr.end()))
		throw parse_error("Found non-space character after string", lineNum);
	return (Value::string_type(newstr.begin(), it));
}

inline
Document::str_const_it	Document::_nextArrayVal(str_const_it it, str_const_it end) const
{
	for (; it != end && *it != ',' && *it != ']'; ++it) ; // Skip to next array value
	if (*it == ',')
		++it;
	_skipWhitespaces(it, end);
	return it;
}

inline
Document::str_const_it	Document::_endofArrayIt(str_const_it it, str_const_it end) const
{
	for (; it != end && *it != ',' && *it != ']'; ++it) ; // Skip to next array value
	return it;
}

inline
Document::str_const_it	Document::_getNextArrayLine(str_const_it it, std::string& line,
	std::size_t& lineNum, std::ifstream& fs) const
{
	std::getline(fs, line);
	lineNum++;
	it = line.begin();
	_skipWhitespaces(it, line.end());
	return (it);
}

// Expects it to be pointing to a '['
inline
Value	Document::_parseArray(std::string const& key, std::string::const_iterator& it,
	std::string& line, std::size_t& lineNum, std::ifstream& fs) const
{
	_skipWhitespaces(++it, line.end()); // Skip first whitespaces in array
	while (it == line.end()) // Skip empty/commented lines
		it = _getNextArrayLine(it, line, lineNum, fs);
	const Value::e_type	array_type = _guessValueType(it, _endofArrayIt(it, line.end()));
	assert(array_type != Value::T_UNDEF);

	Value	array(key, array_type);
	while (*it != ']')
	{
		for (; it != line.end() && *it != ']'; it = _nextArrayVal(it, line.end()))
		{
			const Value::e_type	type = _guessValueType(it, _endofArrayIt(it, line.end()));
			if (type != array_type)
				throw parse_error("Array contains different types", lineNum);
			switch (array_type)
			{
				case Value::T_INT:
					array.array_addValue( Value( "",
							_parseInt(it, _endofArrayIt(it, line.end()), lineNum), Value::T_INT ) );
					break;
				case Value::T_FLOAT:
					array.array_addValue( Value( "",
							_parseFloat(it, _endofArrayIt(it, line.end()), lineNum), Value::T_FLOAT ) );
					break;
				case Value::T_BOOL:
					array.array_addValue( Value( "",
							_parseBool(it, _endofArrayIt(it, line.end()), lineNum), Value::T_BOOL ) );
					break;
				case Value::T_STRING:
					array.array_addValue( Value( "",
							_parseString(it, _endofArrayIt(it, line.end()), lineNum) ) );
					break;
				case Value::T_DATE:
					; // TODO
					break;
				case Value::T_ARRAY:
					array.array_addValue( _parseArray("", it, line, lineNum, fs) );
					++it;
					break;
				default:
					throw parse_error("Unexpected item type in array", lineNum);
			}
		}
		if (it == line.end()) // If at the end of line but no ']' yet
			it = _getNextArrayLine(it, line, lineNum, fs);
	}
	return (array);
}
