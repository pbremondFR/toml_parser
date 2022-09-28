/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Value.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 00:57:16 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 13:34:43 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #pragma once
#ifndef VALUE_HPP
# define VALUE_HPP

# include <string>
# include <vector>
# include <typeinfo>
# include <cassert>
# include "type_traits.hpp"

# ifndef P_TYPE
#  define P_TYPE(x) (typeToChar(x))
# endif

# if __cplusplus == 199711L
#  define noexcept	throw()
# endif

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
		typedef		Value				value_type;
		typedef 	std::size_t			size_type;
		typedef 	std::ptrdiff_t		difference_type;
		typedef		Value&				reference;
		typedef		Value const&		const_reference;
		typedef 	Value*				pointer;
		typedef 	Value const*		const_pointer;

		typedef		std::vector<Value>::iterator				array_iterator;
		typedef		std::vector<Value>::const_iterator			array_const_iterator;
		typedef		std::vector<Value>::reverse_iterator		array_reverse_iterator;
		typedef		std::vector<Value>::const_reverse_iterator	array_const_reverse_iterator;

		typedef 	std::string			string_type;
		typedef		__int64_t			int_type;
		typedef		double				float_type;
		typedef		bool				bool_type;
		typedef		Value				group_type;
		enum e_type
		{
			T_INT,
			T_FLOAT,
			T_BOOL,
			T_STRING,
			T_GROUP,
			T_DATE,
			T_ARRAY,
			T_UNDEF
		};

	private:
		friend class Document; // TODO: Remove me when everything is done, if possible
		template <class T>
		friend class DocumentIterator; // TODO: Remove me when everything is done, if possible
		
		Value&	operator=(Value const&) { assert(false); } // Disables the operator= assignment

		// Possible types that the value can be
		const e_type	_type;
		union
		{
			int_type	_int;
			double		_float;
			bool		_bool;
		};
		string_type		_str; // I want C++11 :'(
		std::vector<Value>	_array; // TODO: Merge me with _hashmap once you've got everything settled
		const e_type		_array_type;
		std::vector<Value>	_hashmap; // lol

		string_type		_key;

		bool	_undefinedGroup; // If the group has only been put there automatically,
								 // i.e if it was a subgroup in a [a.b.c] declaration

		// Returns pointer to inserted or found subtable. Does not protect against double insert.
		// Only meant to be used in Document class
		Value	*_getOrAddSubtable(Value const& group);

	public:
		explicit Value(string_type const& key, float_type floating, e_type type);
		explicit Value(string_type const& key, string_type const& string) : _type(T_STRING), _str(string), _array_type(T_UNDEF), _key(key) {}
		explicit Value(string_type const& key, e_type array_type) : _type(T_ARRAY), _array_type(array_type), _key(key) {}
		explicit Value(string_type const& key) : _type(T_GROUP), _array_type(T_UNDEF), _key(key), _undefinedGroup(true) { }

		inline e_type		type() const noexcept { return _type; }

		inline bool_type	isInt()		const noexcept { return (_type == T_INT);		}
		inline bool_type	isFloat()	const noexcept { return (_type == T_FLOAT);		}
		inline bool_type	isBool()	const noexcept { return (_type == T_BOOL);		}
		inline bool_type	isStr()		const noexcept { return (_type == T_STRING);	}
		inline bool_type	isArray()	const noexcept { return (_type == T_ARRAY);		}
		inline bool_type	isGroup()	const noexcept { return (_type == T_GROUP);		}

		inline bool_type	isFundamental() const noexcept { return (isInt() || isFloat()); }

		inline size_type	strSize()	const { if (isStr())	return _str.size();		throw (bad_type("Value::strSize()"));	}
		inline size_type	groupSize()	const { if (isGroup())	return _hashmap.size();	throw (bad_type("Value::groupSize()"));	}

		// TODO: Add Array()
		inline int_type&			Int()	{ if (isInt())	 return _int;	throw (bad_type("Value::Int()"));	}
		inline float_type&			Float()	{ if (isFloat()) return _float;	throw (bad_type("Value::Float()"));	}
		inline bool_type&			Bool()	{ if (isBool())	 return _bool;	throw (bad_type("Value::Bool()"));	}
		inline string_type&			Str()	{ if (isStr())	 return _str;	throw (bad_type("Value::Str()"));	}
		inline group_type&			Group()	{ if (isGroup()) return *this;	throw (bad_type("Value::Group()"));	}

		inline int_type const&		Int()	const { if (isInt())	return _int;	throw (bad_type("Value::Int()"));		}
		inline float_type const&	Float()	const { if (isFloat())	return _float;	throw (bad_type("Value::Float()"));	}
		inline bool_type const&		Bool()	const { if (isBool())	return _bool;	throw (bad_type("Value::Bool()"));		}
		inline string_type const&	Str()	const { if (isStr())	return _str;	throw (bad_type("Value::Str()"));		}
		inline group_type const&	Group()	const { if (isGroup())	return *this;	throw (bad_type("Value::Group()"));	}

		inline string_type&			key()		noexcept { return _key; }
		inline string_type const&	key() const	noexcept { return _key; }

		Value&			at(std::string const& key);
		Value const&	at(std::string const& key) const;
		Value&			at(size_type n);
		Value const&	at(size_type n) const;

		Value&			operator[](std::string const& key)		 noexcept;
		Value const&	operator[](std::string const& key) const noexcept;
		Value&			operator[](size_type n)		 noexcept;
		Value const&	operator[](size_type n) const noexcept;

		// TODO: When an incorrect set method is called, should it throw, or should it return false ?
		// NOTE: Ah fuck it's not typesafe at all
		bool_type	set(float_type floating) noexcept;
		bool_type	set(string_type const& string); // String assignment cannot guarantee a noexcept
		bool_type	set(group_type const& group); // String assignment cannot guarantee a noexcept

		Value	*group_addValue(Value const& val);
		void	array_addValue(Value const& val);

		array_iterator			begin()			{ return _array.begin(); }
		array_const_iterator	begin() const	{ return _array.begin(); }
		array_iterator			end()			{ return _array.end(); }
		array_const_iterator	end() const		{ return _array.end(); }

		array_reverse_iterator			rbegin()			{ return _array.rbegin(); }
		array_const_reverse_iterator	rbegin() const		{ return _array.rbegin(); }
		array_reverse_iterator			rend()				{ return _array.rend(); }
		array_const_reverse_iterator	rend() const		{ return _array.rend(); }

		friend std::ostream&	operator<<(std::ostream& out, Value const& val);
};

#include "Value.ipp"

} // namespace TOML

# if __cplusplus == 199711L
#  undef noexcept
# endif

#endif
