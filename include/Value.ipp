/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Value.ipp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/25 13:52:14 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/26 05:54:47 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Value.hpp"

// ============================================================================================== //
// --------------------------------------- PUBLIC METHODS --------------------------------------- //
// ============================================================================================== //

Value::Value(string_type const& key, float_type floating, e_type type) : _type(type), _key(key)
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

Value&			Value::at(std::string const& key)
{
	if (!isGroup())
		throw (bad_type("Value::operator[] called with non-Group type"));
	std::vector<Value>::iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}
Value const&	Value::at(std::string const& key) const
{
	if (!isGroup())
		throw (bad_type("Value::operator[] called with non-Group type"));
	std::vector<Value>::const_iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}

Value&			Value::operator[](std::string const& key) noexcept
{
	std::vector<Value>::iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}
Value const&	Value::operator[](std::string const& key) const noexcept
{
	std::vector<Value>::const_iterator it;
	for (it = _hashmap.begin(); it->_key != key && it != _hashmap.end(); ++it) ;
	return *it;
}

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

Value::bool_type	Value::set(string_type const& string)
{
	if (isStr())
		_str = string;
	else
		return false;
	return true;
}

Value::bool_type	Value::set(group_type const& group)
{
	if (isGroup())
		*this = group;
	else
		return false;
	return true;
}

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

// ============================================================================================== //
// ---------------------------------------- OUT OF CLASS ---------------------------------------- //
// ============================================================================================== //

std::ostream&	operator<<(std::ostream& out, Value const& val)
{
	if (val.isInt())
		out << val._int;
	else if (val.isFloat())
		out << val._float;
	else if (val.isBool())
		out << (val._bool == true ? "true" : "false");
	else if (val.isStr())
		out << val._str;
	else if (val.isGroup())
	{
		out << "Group " << val._key << "("<<val._hashmap.size()<<")" << "->{";
		for (std::vector<Value>::const_iterator it = val._hashmap.begin();
			it != val._hashmap.end();
			++it)
		{
			out << *it << ", ";
		}
		out << "}";
	}
	else
		out << "TODO";
	return out;
}

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
		case TOML::Value::T_UNDEF:
			return ("Undefined");
		default:
			return ("unknown type");
	}
}
