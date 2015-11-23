#pragma once

#include <iterator>

namespace cas {
namespace detail {


template<class L> class expr_list_iterator : public std::iterator<std::forward_iterator_tag, typename L::expr_type>
{
	typedef typename L::expr_type expr_type;
	L*		_plist;
	L*		_pprev;
	bool	_atend;
public:
	expr_list_iterator(L *plist) : _plist(plist), _pprev(nullptr), _atend(false) {}
	expr_list_iterator() : _plist(nullptr), _pprev(nullptr), _atend(true) {}
	bool at_last() const { return _atend; }
	L* prev() const { return _pprev; }
	bool operator == (const expr_list_iterator& it) { return _atend == it._atend && _plist == it._plist; }
	bool operator != (const expr_list_iterator& it) { return !(*this == it); }
	expr_type& operator *() const { return _atend ? _plist->_right : _plist->_left; }
	L* operator->() const { return _plist; }
	expr_list_iterator& operator++() {
		if(_plist->_right.type() == typeid(L::value_type))	_pprev = _plist, _plist = &boost::get<L::value_type>(_plist->_right);
		else if(!_atend)	_atend = true;
		else				_plist = nullptr;
		return *this;
	}
};

template<class T, class E>
class expr_list
{
protected:
	E _left;
	E _right;
	void set(E& to, E& from) { E tmp = from; to = tmp;  }

public:
	typedef T value_type;
	typedef E expr_type;
	typedef expr_list_iterator<expr_list<T, E>> iterator;
	friend class iterator;

	expr_list(E left, E right) : _left(left), _right(right) {}
	E left() { return _left; }
	E right() { return _right; }

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(); }

	void erase(iterator it)
	{
		expr tmp;
		if(it.at_last()) {
			if(it.prev())	set(it.prev()->_right, it->_left);
			else			_right = it->_left, _left = T::unit();
		} else {
			if(it.prev())	set(it.prev()->_right, it->_right);
			else if((++it).at_last())	_left = T::unit();
			else 			_left = it->_left, set(_right, it->_right);
		}
	}

	void insert(const E& e)
	{
		if(_right == T::unit())	_right = e;
		else if(_left == T::unit() && e < T::unit())	_left = e;
		else if(e < _left)		_right = T{_left, _right}, _left = e;
		else if(_right.type() == typeid(T))	boost::get<T>(_right).insert(e);
		else if(e < _right)		_right = T{e, _right};
		else					_right = T{_right, e};
	}

	void append(E e)
	{
		expr tmp;
		while(e != T::unit()) {
			iterator it;
			for(it = begin(); it != end(); ++it) {
				if(*it == T::unit())		continue;
				tmp = T::op(*it, e);
				if(tmp.type() != typeid(T))	break;
			}
			if(it == end()) {
				insert(e);
				return;
			}
			erase(it);
			e = tmp;
		}
	}
};

}
}