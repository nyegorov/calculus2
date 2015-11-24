#pragma once

#include <algorithm>

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

expr make_power(expr x, expr y) {
	if(y == zero)		return one;				// x^0 => 1
	if(x == zero)		return zero;			// 0^y => 0
	if(y == one)		return x;				// x^1 => x
	if(x == one)		return one;				// 1^y => 1
	return power{ x, y };
}

expr make_sum(expr left, expr right) {
	if(left == zero)	return right;			// 0+x => x
	if(right == zero)	return left;			// x+0 => x
	if(right == empty)	return left;			// x+0 => x
	if(left == right)	return two * left;		// x+x => 2*x
	if(left > right)	std::swap(left, right);
	return sum{ left, right };
}

expr make_prod(expr left, expr right)
{
	if(left == zero || right == zero)	return zero;	// 0*x => 0
	if(left == one)		return right;			// 1*x => x
	if(right == one)	return left;			// x*1 => x
	if(left == right)	return left ^ two;		// x*x => x^2
	if(left > right)	std::swap(left, right);
	return product{ left, right };
}

expr product::op(const expr& lh, const expr& rh) { return lh * rh; }
expr sum::op(const expr& lh, const expr& rh) { return lh + rh; }

expr operator * (symbol op) { return op.name().empty() ? op.value() : op; }
expr operator * (power op) { return make_power(op.x(), op.y()); }
expr operator * (product op) { return make_prod(op.left(), op.right()); }
expr operator * (sum op) { return make_sum(op.left(), op.right()); }
expr operator + (symbol lh, symbol rh) { return make_sum(*lh, *rh); }
expr operator + (power lh, power rh) { 
	if(lh.y() == two && rh.y() == two && is<func>(lh.x()) && is<func>(rh.x())) {
		// sin(x)^2+cos(x)^2=1
		if(is<fn_sin>(as<func>(lh.x()).f()) && is<fn_cos>(as<func>(rh.x()).f()) && as<fn_sin>(as<func>(lh.x()).f()).x() == as<fn_cos>(as<func>(rh.x()).f()).x())	return one;
		if(is<fn_sin>(as<func>(rh.x()).f()) && is<fn_cos>(as<func>(lh.x()).f()) && as<fn_sin>(as<func>(rh.x()).f()).x() == as<fn_cos>(as<func>(lh.x()).f()).x())	return one;
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

expr operator ^ (symbol lh, symbol rh) { return make_power(*lh, *rh); }
expr operator ^ (power lh, power rh) { return make_power(lh.x(), lh.y() * *rh); }
expr operator ^ (product lh, product rh) { 
	if(lh.left() == one)	return make_power(lh.right(), *rh);
	return (lh.left() ^ *rh) * (lh.right() ^ *rh); 
}
expr operator ^ (sum lh, integer n) {
	expr s;
	for(int k = 0; k <= n.value(); k++) {
		s = s + binomial(n.value(), k) * (lh.left() ^ (n.value() - k)) * (lh.right() ^ k);	// C(n,k) * a^(n-k) * b^k
	}
	return s;
}
expr operator ^ (sum lh, sum rh) { return make_power(*lh, *rh); }

expr symbol::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : *this; }
expr power::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_x | s) ^ (_y | s); }
expr product::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) * (_right | s); }
expr sum::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) + (_right | s); }

expr symbol::approx() const { return _value == empty ? expr{*this} : ~_value; }
expr power::approx() const { return ~_x ^ ~_y; }
expr product::approx() const { return ~_left * ~_right; }
expr sum::approx() const { return ~_left + ~_right; }

match_result symbol::match(expr e, match_result res) const { 
	auto it = find(res.matches.begin(), res.matches.end(), *this);
	if(it == res.matches.end()) {
		res.matches.push_back({_name, e});
	} else {
		if(e != it->value())	res.found = false;
	}
	return res; 
}
match_result power::match(expr e, match_result res) const { return is<power>(e) ? cas::match(as<power>(e).y(), _y, cas::match(as<power>(e).x(), _x, res)) : (res.found = false, res); }
match_result product::match(expr e, match_result res) const { return is<product>(e) ? cas::match(as<product>(e).right(), _right, cas::match(as<product>(e).left(), _left, res)) : (res.found = false, res); }
match_result sum::match(expr e, match_result res) const { return is<sum>(e) ? cas::match(as<sum>(e).right(), _right, cas::match(as<sum>(e).left(), _left, res)) : (res.found = false, res); }

}