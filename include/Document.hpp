/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/25 19:50:30 by pbremond         ###   ########.fr       */
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

#include "Value.hpp"

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

template < class T >
class DocumentIterator
{
	public:
		typedef typename	std::iterator_traits<T>::difference_type	difference_type;
		typedef typename	std::iterator_traits<T>::value_type			value_type;
		typedef typename	std::iterator_traits<T>::pointer			pointer;
		typedef typename	std::iterator_traits<T>::reference			reference;
		typedef typename	std::bidirectional_iterator_tag				iterator_category;
	
	private: // TODO: Use iterators
		value_type*		_root;
		value_type*		_current;
	
	public:
		DocumentIterator(reference value) : _root(&value), _current(_root->_hashmap[0]) {}

		inline operator DocumentIterator<const T>() const { return _current; }

		inline reference	operator* () const { return *_current;	}
		inline pointer		operator->() const { return _current;	}

		DocumentIterator&	operator++(); // pre
		DocumentIterator	operator++(int); // post
		DocumentIterator&	operator--();
		DocumentIterator	operator--(int);

		inline bool	operator==(DocumentIterator const& rhs)	{ return (_current == rhs._current);	}
		inline bool	operator!=(DocumentIterator const& rhs)	{ return (_current != rhs._current);	}
};

// TODO: iterators
class Document
{
	public:
		typedef	Value				value_type;
		typedef std::size_t			size_type;
		typedef std::ptrdiff_t		difference_type;
		typedef	Value&				reference;
		typedef	Value const&		const_reference;
		typedef Value*				pointer;
		typedef Value const*		const_pointer;

	private:
		Value			_root;
		Value			*_currentGroup;
		std::string		_path;
		bool			_isParsed;

	private:
		enum e_value_type
		{
			STRING,
			INT,
			FLOAT,
			BOOL,
			DATE,
			ARRAY,
			UNDEF
		};
		static inline bool	_isSpace(char c) { return (c == 0x09 || c == 0x20); }
		static inline void	_skipWhitespaces(std::string::const_iterator& it, std::string::const_iterator const& end)
		{
			for (; it != end && _isSpace(*it); ++it) ;
		}
		static inline bool	_isBareKeyChar(char c) { return isascii(c) && (isupper(c) || islower(c) || isdigit(c) || c == '-' || c == '_'); }
		static e_value_type	_guessValueType(std::string::const_iterator it, std::string::const_iterator const& end);
		
		void	_parseKeyValue(std::string::const_iterator it, std::string const& line, std::size_t lineNum);
		void	_parseGroup(std::string::const_iterator it, std::string const& line, std::size_t lineNum);
	
	public:
		Document() : _root(Value("", NULL)), _currentGroup(&_root), _isParsed(false) {} // An empty key'd GROUP node is the root
		Document(std::string const& path) : _root(Value("", NULL)),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(const char *path)		  : _root(Value("", NULL)),	_currentGroup(&_root), _path(path),	_isParsed(false) {}
		Document(Value const& value)	  : _root(value),			_currentGroup(&_root), _path(),		_isParsed(true) {} 
		~Document() {}

		Value&				at(std::string const& key);
		inline Value const&	at(std::string const& key) const;
		Value&			operator[](std::string const& key);
		Value const&	operator[](std::string const& key) const;

		bool	parse(std::string const& path);
		bool	parse();
};

// ============================================================================================== //
// ------------------------------------------          ------------------------------------------ //
// ============================================================================================== //

#include "Document.ipp"

} // namespace TOML
