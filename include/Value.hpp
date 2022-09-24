/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Value.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 00:57:16 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/24 02:58:58 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include "type_traits.hpp"

class bad_type : public std::exception
{
	private:
		const char	*_msg;

	public:
		bad_type(const char *msg) : _msg(msg) {}
		virtual const char	*what() const throw() { return (_msg); }
};

class Value
{
	public:
		typedef typename	std::string		string_type;
		typedef				__int64_t		int_type;
		typedef				double			float_type;
		typedef				bool			bool_type;
		typedef				Value			group_type;
		enum e_type
		{
			T_INT,
			T_FLOAT,
			T_BOOL,
			T_STRING,
			T_GROUP,
			T_UNDEF
		};

	private:
		// Possible types that the value can be
		e_type	_type;
		union
		{
			int_type	_int;
			double		_float;
			bool		_bool;
		};
		string_type		_str; // I want C++11 :'(
		std::vector<Value>	_hashmap; // lol

		string_type		_key;

	public:
		explicit Value(string_type const& key, float_type floating, e_type type) : _type(type), _key(key)
		{
			if (_type == T_INT)
				_int = static_cast<int_type>(floating);
			else if (_type == T_BOOL)
				_bool = static_cast<bool_type>(floating);
			else if (_type == T_FLOAT)
				_float = floating;
			else
				throw (bad_type("tried to construct fundamental value with non-fundamental type"));
		}
		         Value(string_type const& key, string_type const& string) : _type(T_STRING), _str(string), _key(key) {}
		explicit Value(string_type const& key) : _type(T_GROUP), _key(key) {}
		// TODO: operator=, copy constructor ?

		inline e_type		type() const { return _type; }

		inline bool_type	isInt()		const { return (_type == T_INT);		}
		inline bool_type	isFloat()	const { return (_type == T_FLOAT);	}
		inline bool_type	isBool()	const { return (_type == T_BOOL);		}
		inline bool_type	isStr()		const { return (_type == T_STRING);	}
		inline bool_type	isGroup()	const { return (_type == T_GROUP);	}

		inline bool_type	isFundamental() const { return (isInt() || isFloat()); }

		inline int_type&			Int()	{ if (isInt())	 return _int;	throw (bad_type("Value::Int()"));	}
		inline float_type&			Float()	{ if (isFloat()) return _float;	throw (bad_type("Value::Float()"));	}
		inline bool_type&			Bool()	{ if (isBool())	 return _bool;	throw (bad_type("Value::Bool()"));	}
		inline string_type&			Str()	{ if (isStr())	 return _str;	throw (bad_type("Value::Str()"));	}
		inline group_type&			Group()	{ if (isGroup()) return *this;	throw (bad_type("Value::Group()"));	}

		inline int_type const&		Int()	const { if (isInt())	return _int;	throw (bad_type("Int()"));		}
		inline float_type const&	Float()	const { if (isFloat())	return _float;	throw (bad_type("Float()"));	}
		inline bool_type const&		Bool()	const { if (isBool())	return _bool;	throw (bad_type("Bool()"));		}
		inline string_type const&	Str()	const { if (isStr())	return _str;	throw (bad_type("Str()"));		}
		inline group_type const&	Group()	const { if (isGroup())	return *this;	throw (bad_type("Group()"));	}

		inline string_type&			key()		{ return _key; }
		inline string_type const&	key() const	{ return _key; }

		// TODO: When an incorrect set method is called, should it throw, or should it return false ?
		// NOTE: Ah fuck it's not typesafe at all
		inline bool_type	set(float_type floating)
		{
			if (isInt())
				_int = static_cast<int_type>(floating);
			else if (isBool())
				_bool = static_cast<bool_type>(floating);
			else if (isFloat())
				_float = floating;
			else
				return false;
				// throw (bad_type("tried to set fundamental Value with non-fundamental variable"));
			return true;
		}
		inline bool_type	set(string_type const& string)
		{
			if (isStr())
				_str = string;
			else
				return false;
			return true;
			// throw (bad_type("tried to set non-string Value to string"));
		}
		inline bool_type	set(group_type const& group)
		{
			if (isGroup())
				*this = group;
			else
				return false;
			return true;
			// throw (bad_type("tried to set non-group Value to group"));
		}

		friend std::ostream&	operator<<(std::ostream& out, Value const& val);
};

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
	else
		out << "TODO";
	return out;
}

const char* typeToChar(Value::e_type type)
{
	switch (type)
	{
		case Value::T_INT:
			return ("Int");
		case Value::T_FLOAT:
			return ("Float");
		case Value::T_BOOL:
			return ("Bool");
		case Value::T_STRING:
			return ("String");
		case Value::T_GROUP:
			return ("Group");
		case Value::T_UNDEF:
			return ("Undefined");
		default:
			return ("unknown type");
	}
}
