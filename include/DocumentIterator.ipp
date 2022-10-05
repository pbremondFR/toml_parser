/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DocumentIterator.ipp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/05 17:11:46 by pbremond          #+#    #+#             */
/*   Updated: 2022/10/05 18:24:40 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "DocumentIterator.hpp"

using namespace TOML::__detail;

template <class T>
inline
DocumentIterator<T>&	DocumentIterator<T>::operator++()
{
	++(_stack.top().second);
	while (_stack.size() > 1
		&& _stack.top().second >= _stack.top().first._hashmap.end())
	{
		_stack.pop();
		++(_stack.top().second);
	}
	return *this;
}

template <class T>
inline
DocumentIterator<T>		DocumentIterator<T>::operator++(int)
{
	DocumentIterator	tmp(*this);
	this->operator++();
	return tmp;
}
