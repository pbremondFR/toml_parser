/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DocumentIterator.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/05 17:11:33 by pbremond          #+#    #+#             */
/*   Updated: 2022/10/05 20:07:48 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stack>
#include <vector>

namespace TOML
{

namespace __detail
{

// TODO: const iterators, can't be bothered right now...
template < class T >
class DocumentIterator
{
	public:
		typedef typename	std::ptrdiff_t						difference_type;
		typedef 			T									value_type;
		typedef				T*									pointer;
		typedef				T&									reference;
		typedef typename	std::forward_iterator_tag			iterator_category;

	private:
		typedef typename	std::vector<T>::iterator									hashmap_iterator;
		typedef typename	std::stack< std::pair<value_type&, hashmap_iterator> >		stack_type;
		
		stack_type	_stack;

		// DocumentIterator(stack_type const& src_stack)  {}
	
	public:
		DocumentIterator() {}
		DocumentIterator(value_type& group_of_it, hashmap_iterator it)
		{
			_stack.push(std::pair<value_type&, hashmap_iterator>(group_of_it, it));
		}
		DocumentIterator(DocumentIterator const& src) : _stack(src._stack) {}
		// template <class U>
		// DocumentIterator(DocumentIterator<U> const& src) : _stack(src._stack) {}

		// inline operator DocumentIterator<const T>() const
		// {
		// 	// return DocumentIterator(*this);
		// 	return DocumentIterator<const T>(_stack);
		// }

		DocumentIterator&	operator++(); // pre
		DocumentIterator	operator++(int); // post

		inline reference	operator* () const { return _stack.top().second.operator*();	}
		inline pointer		operator->() const { return _stack.top().second.operator->();	}

		inline bool	operator==(DocumentIterator const& rhs) { return (_stack.top().second == rhs._stack.top().second); }
		inline bool	operator!=(DocumentIterator const& rhs) { return (_stack.top().second != rhs._stack.top().second); }
};

#include "DocumentIterator.ipp"

} // namespace __detail

} // namespace TOML
