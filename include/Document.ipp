/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.ipp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 14:04:10 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/27 00:29:25 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Document.hpp"

// ============================================================================================== //
// ------------------------------------------ ITERATOR ------------------------------------------ //
// ============================================================================================== //

// Go through hasmap. If you find a group, go in it. If you're at the end of the hashmap,
// go to the parent group of the current group.
template < class T >
DocumentIterator<T>&	DocumentIterator<T>::operator++()
{
	assert(_root->isGroup());

	// std::cerr << HBLU"### ++ ###" << std::endl;
	++(_stack.top().second); // Increase iterator

	yolo:
	while (_stack.top().second < _stack.top().first->_hashmap.end()
		&& _stack.top().second->isGroup())
	{
		// std::cerr << "# ↓ Pushing stack" << std::endl;
		_stack.push( std::make_pair(_stack.top().second.operator->(),
				_stack.top().second->_hashmap.begin()) ); // Lord forgive me for I have sinned
		// std::cerr << "# Stack top after push: " << *_stack.top().first << std::endl;
	}
	while (_stack.top().second >= (_stack.top().first->_hashmap.end())
		&& _stack.size() > 1)
	{
		// std::cerr << "# Stack top BEFORE: " << *_stack.top().first << std::endl;
		// std::cerr << "# ↑ Popping stack" << std::endl;
		_stack.pop();
		if (_stack.size()) {
			++(_stack.top().second);
			// std::cerr << "# Stack top after pop: " << *_stack.top().first << std::endl;
		}
		goto yolo; // NOTE: "I have humiliated myself and will now commit suicide"
	}
	// std::cerr << "##########\n"RESET << std::endl;

	return *this; // tmp
}

template < class T >
DocumentIterator<T>	DocumentIterator<T>::operator++(int)
{
	DocumentIterator	tmp(*this);
	operator++();
	return (tmp);
}

template < class T >
DocumentIterator<T>&	DocumentIterator<T>::operator--()
{
	assert(_root->isGroup());

	std::cerr << HBLU"### -- ###" << std::endl;
	--(_stack.top().second); // Increase iterator

	yolo:
	while (_stack.top().second >= _stack.top().first->_hashmap.begin()
		&& _stack.top().second->isGroup())
	{
		std::cerr << "## Stack top BEFORE: " << *_stack.top().first << "\n## "
			<< *_stack.top().second << std::endl;
		std::cerr << "$$ " << ((_stack.top().second->_hashmap.begin())).operator->() << std::endl;
		
		std::cerr << BLUHB"# ↓ Pushing stack"HBLU << std::endl;
		
		if (_stack.top().second->groupSize())
			_stack.push( std::make_pair(_stack.top().second.operator->(),
					_stack.top().second->_hashmap.end() - 1) ); // Lord forgive me for I have sinned
		else
			--(_stack.top().second);					

		std::cerr << "## Stack top after push:\n## " << *_stack.top().first << "\n## ";
				// << *_stack.top().second << std::endl;
	}
	while (_stack.top().second < (_stack.top().first->_hashmap.begin()) // iterator >= _hashmap.end()
		&& _stack.size() > 1)
	{
		std::cerr << "# Stack top BEFORE: " << *_stack.top().first << std::endl;
		std::cerr << BLUHB"# ↑ Popping stack"HBLU << std::endl;

		_stack.pop();
		if (_stack.size()) {
			--(_stack.top().second);

			std::cerr << "## Stack top after pop:\n## " << *_stack.top().first << "\n## ";
				// << *_stack.top().second << std::endl;
		}
		goto yolo; // NOTE: "I have humiliated myself and will now commit suicide"
	}
	std::cerr << "##########\n"RESET << std::endl;

	return *this; // tmp
}

template < class T >
DocumentIterator<T>	DocumentIterator<T>::operator--(int)
{
	DocumentIterator	tmp(*this);
	operator--();
	return (tmp);
}

// ============================================================================================== //
// --------------------------------------- PUBLIC METHODS --------------------------------------- //
// ============================================================================================== //

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
			_parseKeyValue(it, line, lineNum);
		}
	}
	_isParsed = true;
	return true;
}

// TODO: Finish me, TESTME
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

void	Document::_parseKeyValue(std::string::const_iterator src_it, std::string const& line, std::size_t lineNum)
{
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
			#ifdef __linux
				std::sscanf(it.base(), "%ld", &val); // Just rewrite your own func, dude
			#else
				std::sscanf(it.base(), "%lld", &val); // Just rewrite your own func, dude
			#endif
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
