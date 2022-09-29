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
#include <stack>

#include "iterator.hpp"

#include "Value.hpp"

#include "ansi_color.h"

# if __cplusplus == 199711L
#  define noexcept	throw()
# endif

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
	public:
		typedef	Value									value_type;
		typedef std::size_t								size_type;
		typedef std::ptrdiff_t							difference_type;
		typedef	Value&									reference;
		typedef	Value const&							const_reference;
		typedef Value*									pointer;
		typedef Value const*							const_pointer;
		
		// No iterators for now. They're in their own branch until I figure out wtf am I doing.

	private:
		Value			_root;
		Value			*_currentGroup;
		std::string		_path;
		bool			_isParsed;

	private:
		typedef Value::string_type::const_iterator	str_const_it;
		
		static inline bool	_isSpace(char c) noexcept { return (c == 0x09 || c == 0x20); }
		// Skips all whitespaces and comments
		static inline void	_skipWhitespaces(std::string::const_iterator& it, std::string::const_iterator const& end) noexcept
		{
			for (; it != end && _isSpace(*it); ++it) ;
			if (*it == '#')
				it = end;
		}
		static inline bool _isBareKeyChar(char c) noexcept
		{
			return isascii(c) && (isupper(c) || islower(c) || isdigit(c) || c == '-' || c == '_');
		}
		static Value::e_type	_guessValueType(std::string::const_iterator it, std::string::const_iterator const& end);
		// Returns whether or not iterator range has got non-whitespace characters. Comments are ignored.
		static inline bool	_hasNonWhitespace(std::string::const_iterator first, std::string::const_iterator const& last)
		{
			_skipWhitespaces(first, last);
			return (first < last && *first != '#');
		}
		
		void	_parseGroup(str_const_it it, std::string const& line, std::size_t lineNum);
		void	_parseKeyValue(str_const_it it, std::string& line, std::size_t& lineNum, std::ifstream& fs);

		Value::int_type		_parseInt	(str_const_it it, str_const_it end, std::size_t lineNum) const;
		Value::float_type	_parseFloat	(str_const_it it, str_const_it end, std::size_t lineNum) const;
		Value::bool_type	_parseBool	(str_const_it it, str_const_it end, std::size_t lineNum) const;

		bool	_parseCompactEscapeSequence(std::string::iterator& it, std::string& raw_str, char escaped) const;
		void	_parseEscapedUnicode(std::string::iterator& it, std::string& raw_str, std::size_t lineNum) const;
		Value::string_type	_parseString(str_const_it it, str_const_it end, std::size_t lineNum) const;

		str_const_it	_nextArrayVal(str_const_it it, str_const_it end) const;
		str_const_it	_endofArrayIt(str_const_it it, str_const_it end) const;
		str_const_it	_getNextArrayLine(str_const_it it, std::string& line, std::size_t& lineNum,
							std::ifstream& fs) const;
		Value	_parseArray(std::string const& key, str_const_it& it, std::string& line,
					std::size_t& lineNum, std::ifstream& fs) const;
	
	public:
		Document() : _root(Value("")), _currentGroup(&_root), _isParsed(false) {} // An empty key'd GROUP node is the root
		Document(std::string const& path) : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(const char *path)		  : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(Value const& value)	  : _root(value),		_currentGroup(&_root), _path(),		_isParsed(true)
		{
			if (!value.isGroup())
				throw bad_type("Tried constructing document from non-Group value");
		} 
		~Document() {}

		Value&			at(std::string const& key);
		Value const&	at(std::string const& key) const;
		Value&			operator[](std::string const& key)		 noexcept;
		Value const&	operator[](std::string const& key) const noexcept;

		bool	parse(std::string const& path);
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
