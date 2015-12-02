#pragma once

#include <algorithm>
#include <numeric>

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

expr make_power(expr x, expr y) {
	if(is<error>(x))	return x;
	if(is<error>(y))	return y;
	if(is<numeric>(x) && is<numeric>(y))	return powr(as<numeric>(x).value(), as<numeric>(y).value());
	if(y == zero)		return one;					// x⁰ ⇒ 1
	if(x == zero)		return zero;				// 0ⁿ ⇒ 0
	if(y == one)		return x;					// x¹ ⇒ x
	if(x == one)		return one;					// 1ⁿ ⇒ 1
	return power{x, y};
}

expr make_sum(expr left, expr right) {
	if(is<error>(left))	return left;
	if(is<error>(right))return right;
	if(is<numeric>(left) && is<numeric>(right))	return add(as<numeric>(left).value(), as<numeric>(right).value());
	if(left == zero)	return right;				// 0+x ⇒ x
	if(right == zero)	return left;				// x+0 ⇒ x
	if(right == empty)	return left;				// x+0 ⇒ x
	if(left == right)	return two * left;			// x+x ⇒ 2x

	if(left > right)	std::swap(left, right);
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

expr make_prod(expr left, expr right)
{
	if(is<error>(left))	return left;
	if(is<error>(right))return right;
	if(is<numeric>(left) && is<numeric>(right))	return mul(as<numeric>(left).value(), as<numeric>(right).value());
	if(left == zero || right == zero) return zero;	// 0∙x ⇒ 0
	if(left == one)		return right;				// 1∙x ⇒ x
	if(right == one)	return left;				// x∙1 ⇒ x
	if(left == right)	return left ^ two;			// x∙x ⇒ x²

	if(left > right)	std::swap(left, right);
	if(is<power>(right)) {							// x∙xⁿ ⇒ xⁿ⁺¹
		power &pr = as<power>(right);
		if(pr.x() == left)	return pr.x() ^ (one + pr.y());
		if(is<power>(left)) {
			power &pl = as<power>(left);			// xⁿ∙xᵐ ⇒ xⁿ⁺ᵐ
			if(pl.x() == pr.x())	return pl.x() ^ (pl.y() + pr.y());
		}
	}

	return product{left, right};
}

expr make_xset(std::initializer_list<expr> items)
{
	if(items.size() == 1)	return *items.begin();
	return xset{items};
}

expr product::op(const expr& lh, const expr& rh) { return lh * rh; }
expr sum::op(const expr& lh, const expr& rh) { return lh + rh; }

template<typename T, typename U> typename std::enable_if<!is_expr<T>::value && !is_expr<U>::value, expr>::type operator + (T lh, U rh) { return make_sum(lh, rh); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator + (T e, sum s) { s.append(e); return s.left() == zero ? s.right() : s; }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator + (sum s, T e) { s.append(e); return s.left() == zero ? s.right() : s; }
expr operator + (sum s, sum a) { for(auto& e : a) s.append(e);	return s.left() == zero ? s.right() : s; }
expr operator + (numeric n1, numeric n2) { return add(n1.value(), n2.value()); }
expr operator + (power lh, power rh) {
	if(lh.y() == two && rh.y() == two) {
		// sin²(x)+cos²(x)=1
		if(is<func, fn_sin>(lh.x()) && is<func, fn_cos>(rh.x()) && as<func, fn_sin>(lh.x()).x() == as<func, fn_cos>(rh.x()).x())	return one;
		if(is<func, fn_sin>(rh.x()) && is<func, fn_cos>(lh.x()) && as<func, fn_sin>(rh.x()).x() == as<func, fn_cos>(lh.x()).x())	return one;
	}
	return make_sum(lh, rh);
}

template<typename T, typename U> typename std::enable_if<!is_expr<T>::value && !is_expr<U>::value, expr>::type operator * (T lh, U rh) { return make_prod(lh, rh); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator * (T e, product s) { s.append(e); return s.left() == one ? s.right() : s; }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator * (product s, T e) { s.append(e); return s.left() == one ? s.right() : s; }
expr operator * (product s, product a) { for(auto& e : a) s.append(e);	return s.left() == one ? s.right() : s; }
expr operator * (numeric n1, numeric n2) { return mul(n1.value(), n2.value()); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator * (T e, sum s) { return e * s.left() + e * s.right(); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator * (sum s, T e) { return s.left() * e + s.right() * e; }
expr operator * (product p, sum s) { return p * s.left() + p * s.right(); }
expr operator * (sum s, product p) { return s.left() * p + s.right() * p; }
expr operator * (sum lh, sum rh) { return lh.left() * rh.left() + lh.left() * rh.right() + lh.right() * rh.left() + lh.right() * rh.right(); }

//template<typename T, typename U, class = typename enable_if<!is_expr<T>::value && !is_expr<U>::value>> expr operator ^ (T lh, U rh) { return one; /* make_power(lh, rh);*/ }
//template<typename T, class = typename enable_if<!is_expr<T>::value>::type> expr operator ^ (power p, T e) { return make_power(p.x(), p.y() * expr {e}); }
template<typename T, typename U> typename std::enable_if<!is_expr<T>::value, expr>::type operator ^ (T x, U y) { return make_power(x, y); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator ^ (power p, T y) { return make_power(p.x(), p.y() * expr{y}); }
template<typename T> typename std::enable_if<!is_expr<T>::value, expr>::type operator ^ (product p, T y) { return (p.left() ^ y) * (p.right() ^ y); }
expr operator ^ (numeric x, numeric y) { return powr(x.value(), y.value()); }
expr operator ^ (sum s, numeric num) {
	if(num.value().type() != typeid(int_t) || num.value() == numeric_t{0} || num.has_sign())	return make_power(s, num);
	int_t n = boost::get<int_t>(num.value());
	expr res(0);
	for(int_t k = 0; k <= n; k++) {
		res = res + binomial(n, k) * (s.left() ^ (n - k)) * (s.right() ^ k);	// (a+b)ⁿ ⇒ Σ C(n,k)∙aⁿ⁻ᵏ∙bᵏ
	}
	return res;
}

/*expr operator * (symbol op) { return op.name().empty() ? op.value() : op; }
expr operator * (power op) { return make_power(op.x(), op.y()); }
expr operator * (product op) { return make_prod(op.left(), op.right()); }
expr operator * (sum op) { return op.left() + op.right(); }*/
/*
expr operator + (power lh, power rh) { 
	if(lh.y() == two && rh.y() == two) {
		// sin²(x)+cos²(x)=1
		if(is<func, fn_sin>(lh.x()) && is<func, fn_cos>(rh.x()) && as<func, fn_sin>(lh.x()).x() == as<func, fn_cos>(rh.x()).x())	return one;
		if(is<func, fn_sin>(rh.x()) && is<func, fn_cos>(lh.x()) && as<func, fn_sin>(rh.x()).x() == as<func, fn_cos>(lh.x()).x())	return one;
	}
	return make_sum(*lh, *rh); 
}*/
/*
expr operator + (product lh, product rh) {
	if(lh == rh) return two * *lh;
	if(is_numeric(lh.left()) && lh.right() == expr{rh})	return (one + lh.left()) * lh.right();
	if(is_numeric(rh.left()) && rh.right() == expr{lh})	return (one + rh.left()) * rh.right();
	if(is_numeric(lh.left()) && is_numeric(rh.left()) && lh.right() == rh.right())	return (lh.left() + rh.left()) * lh.right();
	return make_sum(*lh, *rh);
}*/

/*expr operator * (symbol lh, symbol rh) { return make_prod(*lh, *rh); }
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
*/

/*expr operator ^ (symbol lh, symbol rh) { return make_power(*lh, *rh); }
expr operator ^ (power lh, power rh) { return make_power(lh.x(), lh.y() * *rh); }
expr operator ^ (product lh, product rh) { 
	if(lh.left() == one)	return make_power(lh.right(), *rh);
	return (lh.left() ^ *rh) * (lh.right() ^ *rh); 
}
expr operator ^ (sum lh, numeric n) {
	if(n <= 0)	return make_power(lh, n);
	expr s;
	for(int k = 0; k <= n.value(); k++) {
		s = s + binomial(n.value(), k) * (lh.left() ^ (n.value() - k)) * (lh.right() ^ k);	// C(n,k) * a^(n-k) * b^k
	}
	return s;
}
expr operator ^ (sum lh, sum rh) { return make_power(*lh, *rh); }
expr operator ^ (xset lh, xset rh) { return make_err(error_t::syntax); }
*/

struct op_add : public boost::static_visitor<expr>
{
/*	expr operator()(error lh, error rh) const { return lh; }
	template <typename T> expr operator()(error lh, T rh) const { return lh; }
	template <typename T> expr operator()(T lh, error rh) const { return rh; }
	expr operator()(sum, error rh) const { return rh; }
	expr operator()(error lh, sum) const { return lh; }
	expr operator()(numeric lh, numeric rh) const { return add(lh.value(), rh.value()); }
	expr operator()(sum lh, sum rh) const { for(auto& e : rh) lh.append(e);	return lh.left() == zero ? lh.right() : lh; }
	expr operator()(product lh, product rh) const { return lh + rh; }
	template <typename T> expr operator()(sum lh, T rh) const { lh.append(rh); return lh.left() == zero ? lh.right() : lh; }
	template <typename T> expr operator()(T lh, sum rh) const { rh.append(lh); return rh.left() == zero ? rh.right() : rh; }
	template <typename T, typename U> expr operator()(T lh, U rh) const { return make_sum(lh, rh); }*/
	template <typename T, typename U> expr operator()(T lh, U rh) const { return make_sum(lh, rh); }
	expr operator ()(sum s, sum a) const { return s + a; }
	//expr operator ()(numeric n1, numeric n2) const { return n1 + n2; }
	expr operator ()(power lh, power rh) const { return lh + rh; }
	template<typename T> expr operator()(T e, sum s) const { return e + s; }
	template<typename T> expr operator()(sum s, T e) const { return s + e; }
};

struct op_mul : public boost::static_visitor<expr>
{
	/*expr operator()(error lh, error rh) const { return lh; }
	template <typename T> expr operator()(error lh, T rh) const { return lh; }
	template <typename T> expr operator()(T lh, error rh) const { return rh; }
	expr operator()(product, error rh) const { return rh; }
	expr operator()(error lh, product) const { return lh; }
	expr operator()(numeric lh, numeric rh) const { return mul(lh.value(), rh.value()); }
	expr operator()(product lh, product rh) const { for(auto& e : rh) lh.append(e);	return lh.left() == one ? lh.right() : lh; }
	template <typename T> expr operator()(product lh, T rh) const { lh.append(rh); return lh.left() == one ? lh.right() : lh; }
	template <typename T> expr operator()(T lh, product rh) const { rh.append(lh); return rh.left() == one ? rh.right() : rh; }
	template <typename T, typename U> expr operator()(T lh, U rh) const { return make_prod(lh, rh); }*/
	template<typename T, typename U> expr operator()(T lh, U rh) const { return make_prod(lh, rh); }
	template<typename T> expr operator()(T lh, product rh) const { return lh * rh; }
	template<typename T> expr operator()(product lh, T rh) const { return lh * rh; }
	expr operator()(product lh, product rh) const { return lh * rh; }
	//expr operator()(numeric lh, numeric rh) const { return lh * rh; }
	template<typename T> expr operator()(T lh, sum rh) const { return lh * rh; }
	template<typename T> expr operator()(sum lh, T rh) const { return lh * rh; }
	expr operator()(product lh, sum rh) const { return lh * rh; }
	expr operator()(sum lh, product rh) const { return lh * rh; }
	expr operator()(sum lh, sum rh) const { return lh * rh; }

};

struct op_pwr : public boost::static_visitor<expr>
{
	template <typename T, typename U> expr operator()(T lh, U rh) const { return make_power(lh, rh); }
	template <typename T> expr operator()(error lh, T rh) const { return lh; }
	template <typename T> expr operator()(T lh, error rh) const { return rh; }
	expr operator()(error lh, error rh) const { return lh; }
	//expr operator()(numeric lh, numeric rh) const { return powr(lh.value(), rh.value()); }
};

//template<typename T> expr operator + (expr e, T t) { return e + expr{t}; }
//template<typename T> expr operator + (T t, expr e) { return expr{t} +e; }
//template<typename T> expr operator * (expr e, T t) { return e * expr{t}; }
//template<typename T> expr operator * (T t, expr e) { return expr{t} *e; }
//expr operator + (expr op1, expr op2) { return boost::apply_visitor(op_add(), op1, op2); }
//expr operator * (expr op1, expr op2) { return boost::apply_visitor(op_mul(), op1, op2); }
expr operator + (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x + y; }, op1, op2); }
expr operator * (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x * y; }, op1, op2); }
expr operator ^ (expr op1, expr op2) { return boost::apply_visitor([](auto x, auto y) {return x ^ y; }, op1, op2); }
//expr operator ^ (expr op1, expr op2) { return boost::apply_visitor(op_pwr(), op1, op2); }

}