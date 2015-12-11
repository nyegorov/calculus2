#pragma once

#include <iterator>

namespace cas {
struct match_result;
namespace detail {

template<class L> class expr_list_const_iterator : public std::iterator<std::forward_iterator_tag, typename L::value_type, ptrdiff_t, typename L::const_pointer, typename L::const_reference>
{
protected:
	const L*_plist;
	bool	_atend;
public:
	typedef typename L::const_reference reference;
	expr_list_const_iterator(const L *plist) : _plist(plist), _atend(false) {}
	expr_list_const_iterator() : _plist(nullptr), _atend(true) {}
	bool operator == (const expr_list_const_iterator& it) { return _atend == it._atend && _plist == it._plist; }
	bool operator != (const expr_list_const_iterator& it) { return !(*this == it); }
	reference operator *() const { return _atend ? _plist->_right : _plist->_left; }
	const L* operator->() const { return _plist; }
	expr_list_const_iterator& operator++() {
		if(_plist == nullptr)	return *this;
		else if(_plist->_right.type() == typeid(L::cont_type))	_plist = &boost::get<L::cont_type>(_plist->_right);
		else if(!_atend)	_atend = true;
		else				_plist = nullptr;
		return *this;
	}
};

template<class L> class expr_list_iterator : public std::iterator<std::forward_iterator_tag, typename L::value_type>
{
protected:
	L*		_plist;
	L*		_pprev;
	bool	_atend;
public:
	typedef typename L::value_type value_type;
	typedef typename L::reference reference;
	expr_list_iterator(L *plist) : _plist(plist), _pprev(nullptr), _atend(false) {}
	expr_list_iterator() : _plist(nullptr), _pprev(nullptr), _atend(true) {}
	bool at_last() const { return _atend; }
	L* prev() const { return _pprev; }
	bool operator == (const expr_list_iterator& it) { return _atend == it._atend && _plist == it._plist; }
	bool operator != (const expr_list_iterator& it) { return !(*this == it); }
	reference operator *() const { return _atend ? _plist->_right : _plist->_left; }
	L* operator->() const { return _plist; }
	expr_list_iterator& operator++() {
		if(_plist == nullptr)	return *this;
		else if(_plist->_right.type() == typeid(L::cont_type))	_pprev = _plist, _plist = &boost::get<L::cont_type>(_plist->_right);
		else if(!_atend)	_atend = true;
		else				_plist = nullptr;
		return *this;
	}
};

template<class T, class Expr, class Pred = std::less<Expr>>
class expr_list
{
protected:
	Pred _comp;
	Expr _left;
	Expr _right;
	void set(Expr& to, Expr& from) { Expr tmp = from; to = tmp; }

public:
	typedef Expr value_type;
	typedef T cont_type;
	typedef Pred key_comp;
	typedef Expr* pointer;
	typedef Expr& reference;
	typedef const Expr* const_pointer;
	typedef const Expr& const_reference;
	typedef expr_list_iterator<expr_list<T, Expr, Pred>> iterator;
	typedef expr_list_const_iterator<expr_list<T, Expr, Pred>> const_iterator;
	friend class iterator;
	friend class const_iterator;

	expr_list(Expr left, Expr right) : _left(left), _right(right), _comp(Pred()) {}
	Expr left() { return _left; }
	Expr right() { return _right; }

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(); }
	const_iterator begin() const { return const_iterator(this); }
	const_iterator end() const { return const_iterator(); }

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

	iterator insert(iterator it, const Expr& e)
	{
		insert(e);
		return it;
	}

	void insert(const Expr& e)
	{
		if(_right == T::unit())		_right = e;
		else if(_left == T::unit()) { _left = e; if(!_comp(_left, _right)) swap(_left, _right); }
		else if(_comp(e, _left))	_right = T{_left, _right}, _left = e;
		else if(_right.type() == typeid(T))	boost::get<T>(_right).insert(e);
		else if(_comp(e, _right))	_right = T{e, _right};
		else						_right = T{_right, e};
	}

	void append(Expr e)
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

	bool match(Expr e, match_result& res) const {
		const T& pp = static_cast<const T&>(*this);
		if(!is<T>(e)) return cas::match(T{T::unit(), e}, pp, res);
		const T& pe = as<T>(e);
		for(auto p_it = pp.begin(); p_it != pp.end(); ++p_it) {
			for(auto e_it = pe.begin(); e_it != pe.end(); ++e_it) {
				match_result mr = res;
				if(cas::match(*e_it, *p_it, mr)) {
					T p_rest{T::unit(),T::unit()}, e_rest{T::unit(),T::unit()};
					copy_if(pp.begin(), pp.end(), std::inserter(p_rest, p_rest.end()), [p_it](auto e) {return e != *p_it; });
					copy_if(pe.begin(), pe.end(), std::inserter(e_rest, e_rest.end()), [e_it](auto e) {return e != *e_it; });
					if(cas::match(e_rest.left() == T::unit() ? e_rest.right() : e_rest, p_rest.left() == T::unit() ? p_rest.right() : p_rest, mr))	return res = mr;
				}
			}
		}
		return res.found = false;
	};
};
}
}