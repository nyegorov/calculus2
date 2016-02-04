#pragma once

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

inline bool is_linear(expr e, expr x, expr& a, expr& b) {
	a = df(e, x), b = e - a * x;
	return df(a, x) == zero;
}

// Derivatives
inline expr numeric::d(expr dx) const { return zero; }
inline expr symbol::d(expr dx) const
{
	if(is<symbol>(dx) && as<symbol>(dx).name() == _name) return one;
	if(_value == empty)	return zero;
	return df(_value, dx);
}

inline expr power::d(expr dx) const { return (_x^_y) * (df(_y, dx)*ln(_x) + _y / _x*df(_x, dx)); }				// (fᵍ)' ⇒ fᵍ∙[g'∙ln(f)+g∙f'/f]
inline expr product::d(expr dx) const { return df(_left, dx) * _right + _left * df(_right, dx); }				// (f∙g)' ⇒ f'∙g + f∙g'
inline expr sum::d(expr dx) const { return df(_left, dx) + df(_right, dx); }									// (f+g)' ⇒ f' + g'
inline expr xset::d(expr dx) const {																			// {f, g}' ⇒ {f', g'}
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx](auto e) {return df(e, dx); });
	return{ret};
}

// Integrals

inline expr numeric::integrate(expr dx, expr c) const { return expr{_value} *dx + c; }							// ∫ a dx ⇒ ax
inline expr symbol::integrate(expr dx, expr c) const {															
	return is<symbol>(dx) && _name == as<symbol>(dx).name() ? (dx ^ 2) / 2 + c : *this * dx + c;				// ∫ x dx ⇒ x²/2
}

inline expr power::integrate(expr dx, expr c) const
{
	auto d_x = df(_x, dx);
	if(d_x == zero && _y == dx)	return (_x ^ _y) / ln(_x) + c;													// ∫ aˣ dx ⇒ aˣ / ln(a)
	auto d_y = df(_y, dx);
	if(d_y == zero && d_x == zero)	return *this * dx + c;														// ∫ aᵇ dx ⇒ aᵇ∙x
	if(d_y == zero && df(d_x, dx) == zero)	{
		return _y == minus_one ? 
			ln(_x) / d_x + c :																					// ∫ 1/(ax+b) dx ⇒ ln(ax+b)/a
			(_x ^ (_y + 1)) / (d_x * (_y + 1)) + c;																// ∫ (ax+b)ⁿ dx ⇒ (ax+b)ⁿ⁺¹/a(n+1)
	}
	return make_integral(*this, dx) + c;
}

inline expr product::integrate(expr dx, expr c) const {
	if(df(_left, dx) == zero)	return _left * cas::intf(_right, dx) + c;										// ∫ a∙f(x) dx ⇒ a∙∫ f(x) dx
	if(df(_right, dx) == zero)	return _right * cas::intf(_left, dx) + c;										// ∫ f(x)∙a dx ⇒ a∙∫ f(x) dx

	product p{*this};
	for(auto it = p.begin(); it != p.end(); ++it) {																// ∫ Πaᵢ∙f(x) dx ⇒ Πaᵢ∙∫ f(x) dx
		if(df(*it, dx) == zero) {
			auto e = *it;
			p.erase(it);
			return e * p.integrate(dx, zero) + c;
		}
	}

	symbol y{"y"}, x{"x", dx}, n{"n"};
	expr a, b;
	match_result mr;

	// Integrals with Logarithms
	if(_left == ln(dx) && _right == 1/dx)	return half * (ln(dx) ^ 2) + c;										// ∫ ln(x)/x dx ⇒ 1/2∙ln²(x)
	if((mr = cas::match(*this, (x^n)*ln(x))) && is<numeric, int_t>(b = mr[n]))									// ∫ xⁿ∙ln(x) dx ⇒ xⁿ⁺¹∙[ln(x)/(n+1)-1/(n+1)²], n≠-1
		return (dx ^ (b + 1))*(ln(dx) / (b + 1) - ((b + 1) ^ -2)) + c;
	if(_left == dx && is_func(_right, S_LN) && is_linear(as<func>(_right).x(), dx, a, b))						// ∫ x∙ln(ax+b) dx ⇒ (a²x²-b²)∙ln(ax+b)/2a²-x∙(ax-2b)/4a
		return (((a*dx) ^ 2) - (b ^ 2))*_right / (2 * (a ^ 2)) - dx*(a*dx - 2 * b) / (4 * a) + c;

	// Integrals with Exponents
	if((mr = cas::match(*this, x*(e ^ (y*x)))) && df(a = mr[y], dx) == zero) return (x / a - (a^-2))*_right + c;// ∫ x∙eᵃˣ dx ⇒ (x/a-1/a²)∙eᵃˣ
	if((mr = cas::match(*this, (x^n)*(e^(y*x)))) && df(a=mr[y], dx) == zero &&	is<numeric, int_t>(b = mr[n]) && b > zero)
		return (dx^b)*(e^a*x)/a - b/a*intf((dx ^ (b - 1))*(e^a*x), dx) + c;										// ∫ xⁿ∙eᵃˣ dx ⇒ xⁿ∙eᵃˣ/a - n/a ∫ xⁿ⁻¹∙eᵃˣ dx

	return make_integral(make_prod(p.left(), p.right()), dx) + c;
}

inline expr sum::integrate(expr dx, expr c) const { 
	return cas::intf(_left, dx) + cas::intf(_right, dx) + c;													// ∫ f(x)+g(x) dx ⇒ ∫ f(x) dx + ∫ g(x) dx
}

inline expr xset::integrate(expr dx, expr c) const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx, c](auto e) {return intf(e, dx, c); });
	return{ret};
}

}
