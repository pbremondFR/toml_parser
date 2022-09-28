/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/28 07:07:53 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

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
		// enum e_value_type // FIXME: Use same enum across Value and Document
		// {
		// 	INT,
		// 	FLOAT,
		// 	BOOL,
		// 	STRING,
		// 	DATE,
		// 	ARRAY,
		// 	UNDEF
		// };
		static inline bool	_isSpace(char c) noexcept { return (c == 0x09 || c == 0x20); }
		static inline void	_skipWhitespaces(std::string::const_iterator& it, std::string::const_iterator const& end) noexcept
		{
			for (; it != end && _isSpace(*it); ++it) ;
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
			return (first != last && *first != '#');
		}
		
		void	_parseGroup(std::string::const_iterator it, std::string const& line, std::size_t lineNum);
		void	_parseKeyValue(std::string::const_iterator it, std::string const& line, std::size_t& lineNum,
			std::ifstream& fs);

		void	_parseString(std::string const& key, std::string::const_iterator it, std::string const& line, std::size_t lineNum);
		void	_parseCompactEscapeSequence(std::string::iterator& it, std::string& raw_str, char escaped) const;
		void	_parseEscapedUnicode(std::string::iterator& it, std::string& raw_str, std::size_t lineNum) const;

		void	_parseArray(std::string const& key, std::string::const_iterator& it,
			std::string const& line, std::size_t& lineNum, std::ifstream& fs);
	
	public:
		Document() : _root(Value("")), _currentGroup(&_root), _isParsed(false) {} // An empty key'd GROUP node is the root
		Document(std::string const& path) : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(const char *path)		  : _root(Value("")),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(Value const& value)	  : _root(value),			_currentGroup(&_root), _path(),		_isParsed(true) {} 
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
