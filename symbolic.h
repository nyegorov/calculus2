#pragma once

#include <algorithm>
#include <numeric>

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

expr make_power(expr x, expr y) {
	if(is<numeric>(x) && is<numeric>(y))	return powr(as<numeric>(x).value(), as<numeric>(y).value());
	if(y == zero)		return one;				// x^0 => 1
	if(x == zero)		return zero;			// 0^y => 0
	if(y == one)		return x;				// x^1 => x
	if(x == one)		return one;				// 1^y => 1
	return power{ x, y };
}

expr make_sum(expr left, expr right) {
	if(is<numeric>(left) && is<numeric>(right))	return add(as<numeric>(left).value(), as<numeric>(right).value());
	if(left == zero)	return right;			// 0+x => x
	if(right == zero)	return left;			// x+0 => x
	if(right == empty)	return left;			// x+0 => x
	if(left == right)	return two * left;		// x+x => 2x
	if(left > right)	std::swap(left, right);
	return sum{ left, right };
}

expr make_prod(expr left, expr right)
{
	if(is<numeric>(left) && is<numeric>(right))	return mul(as<numeric>(left).value(), as<numeric>(right).value());
	if(left == zero || right == zero)	return zero;	// 0*x => 0
	if(left == one)		return right;			// 1*x => x
	if(right == one)	return left;			// x*1 => x
	if(left == right)	return left ^ two;		// x*x => x²
	if(left > right)	std::swap(left, right);
	return product{ left, right };
}

expr make_xset(std::initializer_list<expr> items)
{
	if(items.size() == 1)	return *items.begin();
	return xset{items};
}

expr product::op(const expr& lh, const expr& rh) { return lh * rh; }
expr sum::op(const expr& lh, const expr& rh) { return lh + rh; }

expr operator * (symbol op) { return op.name().empty() ? op.value() : op; }
expr operator * (power op) { return make_power(op.x(), op.y()); }
expr operator * (product op) { return make_prod(op.left(), op.right()); }
expr operator * (sum op) { return make_sum(op.left(), op.right()); }
expr operator + (symbol lh, symbol rh) { return make_sum(*lh, *rh); }
expr operator + (power lh, power rh) { 
	if(lh.y() == two && rh.y() == two) {
		// sin²(x)+cos²(x)=1
		if(is<func, fn_sin>(lh.x()) && is<func, fn_cos>(rh.x()) && as<func, fn_sin>(lh.x()).x() == as<func, fn_cos>(rh.x()).x())	return one;
		if(is<func, fn_sin>(rh.x()) && is<func, fn_cos>(lh.x()) && as<func, fn_sin>(rh.x()).x() == as<func, fn_cos>(lh.x()).x())	return one;
	}
	return make_sum(*lh, *rh); 
}
expr operator + (product lh, product rh) {
	if(lh == rh) return two * *lh;
	if(is_numeric(lh.left()) && lh.right() == expr{rh})	return (one + lh.left()) * lh.right();
	if(is_numeric(rh.left()) && rh.right() == expr{lh})	return (one + rh.left()) * rh.right();
	if(is_numeric(lh.left()) && is_numeric(rh.left()) && lh.right() == rh.right())	return (lh.left() + rh.left()) * lh.right();
	return make_sum(*lh, *rh);
}
expr operator + (sum lh, sum rh) {
	for(auto& e : rh) lh.append(e);
	return *lh;
}
expr operator + (xset lh, xset rh) {
	list_t items(lh.items()); 
	items.insert(items.end(), rh.items().begin(), rh.items().end());
	return lh;
}

expr operator * (symbol lh, symbol rh) { return make_prod(*lh, *rh); }
expr operator * (power lh, power rh) {
	// (x^a)*(x^b) => x^(a+b)
	if(lh.x() == rh.x())	return make_power(lh.x(), lh.y() + rh.y());
	return make_prod(*lh, *rh);
}
expr operator * (product lh, product rh) {
	for(auto& e : rh) lh.append(e);
	return *lh;
}
expr operator * (sum lh, sum rh) {
	return lh.left() * rh.left() + lh.left() * rh.right() + lh.right() * rh.left() + lh.right() * rh.right();
}
expr operator * (xset lh, xset rh) { return make_err(error_t::syntax); }

expr operator ^ (symbol lh, symbol rh) { return make_power(*lh, *rh); }
expr operator ^ (power lh, power rh) { return make_power(lh.x(), lh.y() * *rh); }
expr operator ^ (product lh, product rh) { 
	if(lh.left() == one)	return make_power(lh.right(), *rh);
	return (lh.left() ^ *rh) * (lh.right() ^ *rh); 
}
expr operator ^ (sum lh, numeric n) {
/*	if(n <= 0)	return make_power(lh, n);
	expr s;
	for(int k = 0; k <= n.value(); k++) {
		s = s + binomial(n.value(), k) * (lh.left() ^ (n.value() - k)) * (lh.right() ^ k);	// C(n,k) * a^(n-k) * b^k
	}
	return s;*/
	return zero;
}
expr operator ^ (sum lh, sum rh) { return make_power(*lh, *rh); }
expr operator ^ (xset lh, xset rh) { return make_err(error_t::syntax); }

expr symbol::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : *this; }
expr power::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_x | s) ^ (_y | s); }
expr product::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) * (_right | s); }
expr sum::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) + (_right | s); }
expr xset::subst(pair<expr, expr> s) const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [s](auto e) {return e | s; });
	return {ret};
}

expr symbol::approx() const { return _value == empty ? expr{*this} : ~_value; }
expr power::approx() const { return ~_x ^ ~_y; }
expr product::approx() const { return ~_left * ~_right; }
expr sum::approx() const { return ~_left + ~_right; }
expr xset::approx() const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [](auto e) {return ~e; });
	return {ret};
}

bool symbol::match(expr e, match_result& res) const {
	auto it = find(res.matches.begin(), res.matches.end(), *this);
	if(it == res.matches.end()) {
		res.matches.push_back({_name, e});
	} else {
		if(e != it->value())	res.found = false;
	}
	return res.found;
}
bool power::match(expr e, match_result& res) const { return is<power>(e) ? cas::match(as<power>(e).y(), _y, res) && cas::match(as<power>(e).x(), _x, res) : res.found = false; }
bool xset::match(expr e, match_result& res) const {
	if(!is<xset>(e) || as<xset>(e).items().size() != _items.size())	return res.found = false;
	auto pe = as<xset>(e).items().begin();
	for(auto item : _items)	if(!cas::match(*pe++, item, res)) break;
	return res.found;
}

}