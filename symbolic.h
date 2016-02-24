#pragma once

#include <algorithm>
#include <numeric>

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

using std::enable_if;
using std::is_same;

static int_t binomial(int_t n, int_t k) {
	int_t a = 1, b = 1;
	for(int_t l = 1; l <= k; l++)	a *= (n - l + 1), b *= l;
	return a / b;
}

inline expr product::op(const expr& lh, const expr& rh) { return lh * rh; }
inline expr sum::op(const expr& lh, const expr& rh) { return lh + rh; }

inline expr make_err(error_t err) { return error{err}; }

inline expr make_power(expr x, expr y) {
	if(is<error>(x))	return x;
	if(is<error>(y))	return y;
	if(is<numeric>(x) && is<numeric>(y))	return as<numeric>(x).value() ^ as<numeric>(y).value();
	if(y == zero)		return one;					// x⁰ ⇒ 1
	if(x == zero)		return zero;				// 0ⁿ ⇒ 0
	if(y == one)		return x;					// x¹ ⇒ x
	if(x == one)		return one;					// 1ⁿ ⇒ 1
	if(x == e && is_func(y, S_LN))	return as<func>(y).x();	// eˡⁿ⁽ˣ⁾ ⇒ x
	return power{x, y};
}

inline expr make_sum(expr left, expr right) {
	if(is<error>(left))	return left;
	if(is<error>(right))return right;
	if(is<numeric>(left) && is<numeric>(right))	return as<numeric>(left).value() + as<numeric>(right).value();
	if(left == zero)	return right;				// 0+x ⇒ x
	if(right == zero)	return left;				// x+0 ⇒ x
	if(right == empty)	return left;				// x+0 ⇒ x
	if(left == right)	return two * left;			// x+x ⇒ 2x

	sum::key_comp comp;
	if(!comp(left, right))	std::swap(left, right);
	if(is<product>(right)) {
		product &pr = as<product>(right);			// x+Ax ⇒ (A+1)x 
		if(is<numeric>(pr.left()) && pr.right() == left)	return (one + pr.left()) * left;
		if(is<product>(left)) {
			product &pl = as<product>(left);		// Ax+Bx ⇒ (A+B)x
			if(is<numeric>(pl.left()) && is<numeric>(pr.left()) && pl.right() == pr.right())	return (pl.left() + pr.left()) * pl.right();
		}
	}

	return sum{left, right};
}

inline expr make_prod(expr left, expr right)
{
	if(is<error>(left))	return left;
	if(is<error>(right))return right;
	if(is<numeric>(left) && is<numeric>(right))	return as<numeric>(left).value() * as<numeric>(right).value();
	if(left == zero || right == zero) return zero;	// 0∙x ⇒ 0
	if(left == one)		return right;				// 1∙x ⇒ x
	if(right == one)	return left;				// x∙1 ⇒ x
	if(left == right)	return left ^ two;			// x∙x ⇒ x²

	product::key_comp comp;
	if(!comp(left, right))	std::swap(left, right);
	if(is<power>(right)) {							// x∙xⁿ ⇒ xⁿ⁺¹
		power &pr = as<power>(right);
		if(pr.x() == left && !is<numeric>(left))	return pr.x() ^ (one + pr.y());
		if(is<power>(left)) {
			power &pl = as<power>(left);			// xⁿ∙xᵐ ⇒ xⁿ⁺ᵐ
			if(pl.x() == pr.x())	return pl.x() ^ (pl.y() + pr.y());
		}
	}

	return product{left, right};
}

inline expr make_xset(std::initializer_list<expr> items)
{
	if(items.size() == 1)	return *items.begin();
	return xset{items};
}


template<class T> struct is_algebraic		: std::false_type {};
template<> struct is_algebraic<int_t>		: std::true_type  {};
template<> struct is_algebraic<real_t>		: std::true_type {};
template<> struct is_algebraic<rational_t>	: std::true_type {};
template<> struct is_algebraic<complex_t>	: std::true_type {};
template<> struct is_algebraic<string>		: std::true_type {};
template<> struct is_algebraic<error>		: std::true_type {};
template<> struct is_algebraic<numeric>		: std::true_type {};
template<> struct is_algebraic<symbol>		: std::true_type {};
template<> struct is_algebraic<func>		: std::true_type {};
template<> struct is_algebraic<power>		: std::true_type {};
template<> struct is_algebraic<product>		: std::true_type {};
template<> struct is_algebraic<sum>			: std::true_type {};
template<> struct is_algebraic<xset>		: std::true_type {};

// Addition
template<typename T, typename U, typename = std::enable_if_t<is_algebraic<T>::value && is_algebraic<U>::value>> expr operator + (T lh, U rh) { return make_sum(lh, rh); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator + (T e, sum s) { return s.append(e); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator + (sum s, T e) { return s.append(e); }
inline expr operator + (sum s, sum a) { return s.append(a.left()) + a.right(); }
inline expr operator + (power lh, power rh) {
	if(lh.y() == two && rh.y() == two) {			// sin²(x)+cos²(x)=1
		if(is_func(lh.x(), S_SIN) && is_func(rh.x(), S_COS) && as<func>(lh.x()).x() == as<func>(rh.x()).x())	return one;
		if(is_func(lh.x(), S_COS) && is_func(rh.x(), S_SIN) && as<func>(lh.x()).x() == as<func>(rh.x()).x())	return one;
	}
	return make_sum(lh, rh);
}

// Product
template<typename T, typename U, typename = std::enable_if_t<is_algebraic<T>::value && is_algebraic<U>::value>> expr operator * (T lh, U rh) { return make_prod(lh, rh); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value && !is_same<T, sum>::value>> expr operator * (T e, product s) { s.append_old(e); return s.left() == one ? s.right() : s; }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value && !is_same<T, sum>::value>> expr operator * (product s, T e) { s.append_old(e); return s.left() == one ? s.right() : s; }
inline expr operator * (product s, product a) { for(auto& e : a) s.append_old(e);	return s.left() == one ? s.right() : s; }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator * (T e, sum s) { return e * s.left() + e * s.right(); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator * (sum s, T e) { return s.left() * e + s.right() * e; }
inline expr operator * (sum lh, sum rh) { return lh.left() * rh.left() + lh.left() * rh.right() + lh.right() * rh.left() + lh.right() * rh.right(); }
inline expr operator * (func lh, power rh) {
	if(is_func(lh, S_SIN) && is_func(rh.x(), S_COS) && as<func>(lh).x() == as<func>(rh.x()).x() && rh.y() == minus_one)	return tg(as<func>(lh).x());
	if(is_func(lh, S_COS) && is_func(rh.x(), S_SIN) && as<func>(lh).x() == as<func>(rh.x()).x() && rh.y() == minus_one)	return 1/tg(as<func>(lh).x());
	return make_prod(lh, rh);
}

// Power
template<typename T, typename U, typename = std::enable_if_t<is_algebraic<T>::value>> expr operator ^ (T x, U y) { return make_power(x, y); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator ^ (power p, T y) { return make_power(p.x(), p.y() * expr{y}); }
template<typename T, typename = std::enable_if_t<!is_same<T, expr>::value>> expr operator ^ (product p, T y) { return (p.left() ^ y) * (p.right() ^ y); }
inline expr operator ^ (sum s, numeric num) {
	if(num.value().type() != typeid(int_t) || num.value() == numeric_t{0} || num.has_sign())	return make_power(s, num);
	int_t n = boost::get<int_t>(num.value());
	expr res(0);
	for(int_t k = 0; k <= n; k++) {					// (a+b)ⁿ ⇒ Σ C(n,k)∙aⁿ⁻ᵏ∙bᵏ
		res = res + binomial(n, k) * (s.left() ^ (n - k)) * (s.right() ^ k);
	}
	return res;
}

inline expr operator + (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x + y; }, op1, op2); }
inline expr operator * (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x * y; }, op1, op2); }
inline expr operator ^ (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x ^ y; }, op1, op2); }
inline expr operator - (expr op1, expr op2) { return op1 + -op2; }
inline expr operator / (expr op1, expr op2) { return op1 * (op2 ^ minus_one); }
inline expr operator - (expr op1) { return op1 * minus_one; }

}