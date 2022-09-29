/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Value.ipp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 13:52:14 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 13:22:25 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Value.hpp"

// ============================================================================================== //
// --------------------------------------- PUBLIC METHODS --------------------------------------- //
// ============================================================================================== //

inline
Value::Value(string_type const& key, float_type floating, e_type type) : _type(type), _array_type(T_UNDEF),
	_key(key)
{
	if (_type == T_INT)
		_int = static_cast<int_type>(floating);
	else if (_type == T_BOOL)
		_bool = static_cast<bool_type>(floating);
	else if (_type == T_FLOAT)
		_float = floating;
	else
		throw (bad_type("Value: tried to construct fundamental value with non-fundamental type"));
}

inline
Value*	Value::_getOrAddSubtable(Value const& group)
{
	assert(isGroup() && group.isGroup());

	for (std::vector<Value>::iterator it = _hashmap.begin(); it != _hashmap.end(); ++it)
	{
		if (group._key == it->_key && it->isGroup()) // Found subgroup
			return (it.operator->());
		else if (group._key == it->_key) // Subgroup key conflicts with other key
			return NULL;
	}
	_hashmap.push_back(group);
	return (_hashmap.end() - 1).operator->();
}

inline
Value&			Value::at(std::string const& key)
{
	if (!isGroup())
		throw (bad_type("Value::at(Value::string_type const&) called with non-Group type"));
	std::vector<Value>::iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	if (it == _hashmap.end())
		throw std::out_of_range("Value::at: std::out_of_range");
	return *it;
}

inline
Value const&	Value::at(std::string const& key) const
{
	if (!isGroup())
		throw (bad_type("Value::at(Value::string_type const&) called with non-Group type"));
	std::vector<Value>::const_iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	if (it == _hashmap.end())
		throw std::out_of_range("Value::at: std::out_of_range");
	return *it;
}

inline
Value&			Value::at(size_type n)
{
	if (!isArray())
		throw (bad_type("Value::at(Value::size_type) called with non-Group type"));
	if (n > _array.size())
		throw std::out_of_range("Value::at: std::out_of_range");
	return _array[n];
}

inline
Value const&	Value::at(size_type n) const
{
	if (!isArray())
		throw (bad_type("Value::at(Value::size_type) called with non-Group type"));
	if (n > _array.size())
		throw std::out_of_range("Value::at: std::out_of_range");
	return _array[n];
}

inline
Value	Value::at_or(std::string const& key, Value val) const noexcept
{
	std::vector<Value>::const_iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	if (it == _hashmap.end())
		return val;
	return *it;
}

inline
Value	Value::at_or(size_type n, Value val) const noexcept
{
	if (!isArray() || n > _array.size())
		return val;
	return _array[n];
}

inline
Value&			Value::operator[](std::string const& key) noexcept
{
	std::vector<Value>::iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}

inline
Value const&	Value::operator[](std::string const& key) const noexcept
{
	std::vector<Value>::const_iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}

inline
Value&			Value::operator[](size_type n) noexcept
{
	return _array[n];
}

inline
Value const&	Value::operator[](size_type n) const noexcept
{
	return _array[n];
}

inline
Value::bool_type	Value::set(float_type floating) noexcept
{
	if (isInt())
		_int = static_cast<int_type>(floating);
	else if (isBool())
		_bool = static_cast<bool_type>(floating);
	else if (isFloat())
		_float = floating;
	else
		return false;
	return true;
}

inline
Value::bool_type	Value::set(string_type const& string)
{
	if (isStr())
		_str = string;
	else
		return false;
	return true;
}

inline
Value::bool_type	Value::set(group_type const& group)
{
	if (isGroup())
		*this = group;
	else
		return false;
	return true;
}

inline
Value*	Value::group_addValue(Value const& val) // TESTME
{
	if (!this->isGroup()) {
		throw (bad_type("Called group_addValue() on a TOML::Value that's not a group"));
	}
	for (std::vector<Value>::iterator it = _hashmap.begin(); it != _hashmap.end(); ++it)
	{
		if (it->_key == val.key())
		{
			if (val._type == T_GROUP && it->_type == T_GROUP && it->_undefinedGroup == true)
				return it.operator->();
			else
				return NULL;
		}
	}
	_hashmap.push_back(val);
	return (_hashmap.end() - 1).operator->();
}

inline
void	Value::array_addValue(Value const& value)
{
	if (!isArray())
		throw bad_type("Called array_addValue() on a TOML::Value that's not an array");
	_array.push_back(value);
}

// ============================================================================================== //
// ---------------------------------------- OUT OF CLASS ---------------------------------------- //
// ============================================================================================== //

inline
std::ostream&	operator<<(std::ostream& out, Value const& val)
{
	switch (val.type())
	{
		case Value::T_INT:
			out << val._int;
			break;
		case Value::T_FLOAT:
			out << val._float;
			break;
		case Value::T_BOOL:
			out << (val._bool == true ? "true" : "false");
			break;
		case Value::T_STRING:
			out << val._str;
			break;
		case Value::T_GROUP:
		{
			out << "Group " << val._key << "("<<val._hashmap.size()<<")" << "->{ ";
			for (std::vector<Value>::const_iterator it = val._hashmap.begin(); it != val._hashmap.end(); ++it)
				out << *it << (it == val._hashmap.end() - 1 ? " " : ", ");
			out << "}";
			break;
		}
		case Value::T_ARRAY:
		{
			out << '[';
			for (std::vector<Value>::const_iterator it = val._array.begin(); it != val._array.end(); ++it)
				out << *it << (it == val._array.end() - 1 ? "" : ", ");
			out << ']';
			break;
		}
		default:
			out << "TODO";
			break;
	}
	return out;
}

inline
const char* typeToChar(TOML::Value::e_type type)
{
	switch (type)
	{
		case TOML::Value::T_INT:
			return ("Int");
		case TOML::Value::T_FLOAT:
			return ("Float");
		case TOML::Value::T_BOOL:
			return ("Bool");
		case TOML::Value::T_STRING:
			return ("String");
		case TOML::Value::T_GROUP:
			return ("Group");
		case TOML::Value::T_ARRAY:
			return ("Array");
		case TOML::Value::T_UNDEF:
			return ("Undefined");
		default:
			return ("unknown type");
	}
}
