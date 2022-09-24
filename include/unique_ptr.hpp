/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unique_ptr.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbremond <pbremond@student.42nice.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/23 17:10:59 by pbremond          #+#    #+#             */
/*   Updated: 2022/09/23 19:19:42 by pbremond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

namespace ft
{

template <class T>
struct default_delete
{
	default_delete() {}

	void	operator()(T* ptr) const
	{
		// static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
		delete ptr;
	}
};

template <class T>
struct default_delete<T[]>
{
	default_delete() {}

	void	operator()(T* ptr) const
	{
		// static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
		delete[] ptr;
	}
};

template < class T, class Deleter = default_delete<T> >
class unique_ptr
{
	public:
		typedef	T*			pointer;
		typedef	T			element_type;
		typedef	Deleter		deleter_type;
	
	private:
		pointer			_ptr;
		deleter_type	_deleter;
	
	public:
		unique_ptr() : _ptr(NULL), _deleter(deleter_type()) {}
		explicit unique_ptr(pointer p) : _ptr(p), _deleter(deleter_type()) {}
		// I'm not in C++11 and I'm not gonna bother trying to implement its features in there
		unique_ptr(pointer p, Deleter const& deleter) : _ptr(p), _deleter(deleter) {}
		// TODO: other stuff

		~unique_ptr()
		{
			if (_ptr)
				_deleter(_ptr);
		}
	
	public:
		pointer	release()
		{
			pointer	retval = _ptr;
			_ptr = NULL;
			return (retval);
		}
		void	reset(pointer ptr = pointer())
		{
			pointer	old_ptr = _ptr;
			_ptr = old_ptr;
			if (old_ptr)
				_deleter(old_ptr);
		}
		void	swap(unique_ptr& other)
		{
			std::swap(this->_ptr, other._ptr);
			std::swap(this->_deleter, other._deleter);
		}
		pointer	get() const { return (_ptr); }

		Deleter&		get_deleter()		{ return (_deleter); }
		Deleter const&	get_deleter() const { return (_deleter); }

		operator bool() const { return (bool(_ptr)); }

		// ah fuck
		pointer	operator->() const { return (_ptr); }

};

} // namespace ft
