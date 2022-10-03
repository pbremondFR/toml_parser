/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 14:55:57 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <iterator>
#include <errno.h>
#include <cstring> // strerror

#include "exceptions.hpp"
#include "iterator.hpp"
#include "Value.hpp"

#include "ansi_color.h"

# if __cplusplus == 199711L
#  define noexcept	throw()
# endif

namespace TOML
{

class Document
{
	public:
		typedef	Value									value_type;
		typedef Value::size_type						size_type;
		typedef Value::difference_type					difference_type;
		typedef	Value&									reference;
		typedef	Value const&							const_reference;
		typedef Value*									pointer;
		typedef Value const*							const_pointer;

		typedef Value::int_type			int_type;
		typedef Value::float_type		float_type;
		typedef Value::bool_type		bool_type;
		typedef Value::string_type		string_type;
		typedef Value::group_type		group_type;
		typedef Value::array_type		array_type;
		
		// No iterators for now. They're in their own branch until I figure out wtf am I doing.

	private:
		Value			_root;
		Value			*_currentGroup;
		string_type		_path;
		bool			_isParsed;

	private:
		typedef string_type::const_iterator		str_const_it;
		
		static inline bool	_isSpace(char c) noexcept { return (c == 0x09 || c == 0x20); }
		// Skips all whitespaces and comments
		static inline void	_skipWhitespaces(string_type::const_iterator& it, string_type::const_iterator const& end) noexcept
		{
			for (; it != end && _isSpace(*it); ++it) ;
			if (*it == '#')
				it = end;
		}
		static inline bool _isBareKeyChar(char c) noexcept
		{
			return isascii(c) && (isupper(c) || islower(c) || isdigit(c) || c == '-' || c == '_');
		}
		static TOML::Type	_guessValueType(str_const_it it, str_const_it const& end);
		// Returns whether or not iterator range has got non-whitespace characters. Comments are ignored.
		static inline bool	_hasNonWhitespace(str_const_it first, str_const_it const& last)
		{
			_skipWhitespaces(first, last);
			return (first < last && *first != '#');
		}
		
		void	_parseGroup(str_const_it it, string_type const& line, size_type lineNum);
		void	_parseKeyValue(str_const_it it, string_type& line, size_type& lineNum, std::ifstream& fs);

		// Simple types parsing
		Value::int_type		_parseInt	(str_const_it it, str_const_it end, size_type lineNum) const;
		Value::float_type	_parseFloat	(str_const_it it, str_const_it end, size_type lineNum) const;
		Value::bool_type	_parseBool	(str_const_it it, str_const_it end, size_type lineNum) const;

		// String parsing
		bool	_parseCompactEscapeSequence(string_type::iterator& it, string_type& raw_str, char escaped) const;
		void	_parseEscapedUnicode(string_type::iterator& it, string_type& raw_str, size_type lineNum) const;
		Value::string_type	_parseString(str_const_it it, str_const_it end, size_type lineNum) const;

		// Array parsing
		str_const_it	_nextArrayVal(str_const_it it, str_const_it end) const;
		str_const_it	_endofArrayIt(str_const_it it, str_const_it end) const;
		str_const_it	_getNextArrayLine(str_const_it it, string_type& line, size_type& lineNum,
							std::ifstream& fs) const;
		Value	_parseArray(string_type const& key, str_const_it& it, string_type& line,
					size_type& lineNum, std::ifstream& fs) const;
		
		// Group array parsing
		void	_parseGroupArray(str_const_it it, str_const_it end, size_type lineNum);
	
	public:
		// An empty key'd group is the root
		Document()						  : _root(Value("")),	_currentGroup(&_root), _path(),		_isParsed(false) {}
		Document(string_type const& path) : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(const char *path)		  : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(Value const& value)	  : _root(value),		_currentGroup(&_root), _path(),		_isParsed(true)
		{
			if (!value.isGroup())
				throw bad_type("Tried constructing document from non-Group value");
		} 
		~Document() {}

		inline bool		has(string_type const& key) { return _root.has(key); }

		Value&			at(string_type const& key);
		Value const&	at(string_type const& key) const;
		Value			at_or(string_type const& key, Value) const noexcept;
		Value&			operator[](string_type const& key)		 noexcept;
		Value const&	operator[](string_type const& key) const noexcept;

		bool	parse(string_type const& path);
		bool	parse();
};

// ============================================================================================== //
// ------------------------------------------          ------------------------------------------ //
// ============================================================================================== //

#include "Document.ipp"

} // namespace TOML

# if __cplusplus == 199711L
#  undef noexcept
# endif
