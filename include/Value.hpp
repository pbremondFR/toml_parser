/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Value.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 00:57:16 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/25 14:02:32 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #pragma once
#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <vector>
#include <typeinfo>
#include <cassert>
#include "type_traits.hpp"

#ifndef P_TYPE
# define P_TYPE(x) (typeToChar(x))
#endif

namespace TOML
{

class Document;

class bad_type : public std::exception
{
	private:
		const char	*_msg;

	public:
		bad_type(const char *msg) : _msg(msg) {}
		virtual const char	*what() const throw() { return (_msg); }
};

// TODO: comparison operators ? Only compare keys ?
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
		typedef typename	std::size_t		size_type;

	private:
		friend class Document; // TODO: Remove me when everything is done, if possible
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

		bool	_undefinedGroup; // If the group has only been put there automatically,
								 // i.e if it was a subgroup in a [a.b.c] declaration

		// Returns pointer to inserted or found subtable. Does not protect against double insert.
		// Only meant to be used in Document class
		Value	*_getOrAddSubtable(Value const& group);

	public:
		explicit Value(string_type const& key, float_type floating, e_type type);
		         Value(string_type const& key, string_type const& string) : _type(T_STRING), _str(string), _key(key) {}
		explicit Value(string_type const& key) : _type(T_GROUP), _key(key), _undefinedGroup(true) {}
		// TODO: operator=, copy constructor? They're compiler provided for now, test them.

		inline e_type		type() const { return _type; }

		inline bool_type	isInt()		const { return (_type == T_INT);	}
		inline bool_type	isFloat()	const { return (_type == T_FLOAT);	}
		inline bool_type	isBool()	const { return (_type == T_BOOL);	}
		inline bool_type	isStr()		const { return (_type == T_STRING);	}
		inline bool_type	isGroup()	const { return (_type == T_GROUP);	}

		inline bool_type	isFundamental() const { return (isInt() || isFloat()); }

		inline size_type	strSize()	const { if (isStr())	return _str.size();		throw (bad_type("Value::strSize()"));	}
		inline size_type	groupSize()	const { if (isGroup())	return _hashmap.size();	throw (bad_type("Value::groupSize()"));	}

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

		Value&			operator[](std::string const& key);
		Value const&	operator[](std::string const& key) const;

		// TODO: When an incorrect set method is called, should it throw, or should it return false ?
		// NOTE: Ah fuck it's not typesafe at all
		bool_type	set(float_type floating);
		bool_type	set(string_type const& string);
		bool_type	set(group_type const& group);

		Value	*group_addValue(Value const& val); // TESTME: logic

		friend std::ostream&	operator<<(std::ostream& out, Value const& val);
};

#include "Value.ipp"

} // namespace TOML

#endif
