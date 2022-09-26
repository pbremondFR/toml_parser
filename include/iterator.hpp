/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iterator.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/26 04:05:30 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/27 00:07:31 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iterator>

namespace ft
{

// ============================================================================================== //
// ---------------------------------------- FT::ADVANCE ----------------------------------------- //
// ============================================================================================== //
 
template<class InputIt>
static void	_do_advance(InputIt& it, typename std::iterator_traits<InputIt>::difference_type n,
						std::input_iterator_tag)
{
	while (n-- > 0)
		++it;
}
 
template<class BidirIt>
static void	_do_advance(BidirIt& it, typename std::iterator_traits<BidirIt>::difference_type n,
						std::bidirectional_iterator_tag)
{
	while (n > 0) {
        --n;
        ++it;
    }
    while (n < 0) {
        ++n;
        --it;
    }
}
 
template<class RandomIt>
static void	_do_advance(RandomIt& it, typename std::iterator_traits<RandomIt>::difference_type n,
						std::random_access_iterator_tag)
{
	it += n;
}
 
template<class It, class Distance>
void advance(It& it, Distance n)
{
	_do_advance(it, typename std::iterator_traits<It>::difference_type(n),
		typename std::iterator_traits<It>::iterator_category());
}

// ============================================================================================== //
// ------------------------------------------ FT::PREV ------------------------------------------ //
// ============================================================================================== //

template<class BidirIt>
BidirIt	prev(BidirIt it, typename std::iterator_traits<BidirIt>::difference_type n = 1)
{
    ft::advance(it, -n);
    return it;
}

template<class BidirIt>
BidirIt	next(BidirIt it, typename std::iterator_traits<BidirIt>::difference_type n = 1)
{
    ft::advance(it, n);
    return it;
}

} // namespace ft
