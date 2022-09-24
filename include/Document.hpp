/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Document.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 03:05:31 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/24 03:31:29 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include <iostream>
#include <fstream>

#include "Value.hpp"

namespace TOML
{

class Document
{
	private:
		Value	_root;
		std::ifstream	_fs;
	
	public:
		Document() : _root(Value("")) {} // An empty string GROUP node is the root
		Document(std::string const& path) : _root(Value("")), _fs(path.c_str()) {}
		Document(const char *path)		  : _root(Value("")), _fs(path) {}
		~Document()
		{
			if (_fs.is_open())
				_fs.close();
		}

		inline void	open(const char *path, std::ios_base::openmode mode = std::ios_base::in)
		{
			_fs.open(path, mode);
		}

		bool	parse()
		{
			// TODO
			return false;
		}
};

} // namespace TOML
