#pragma once

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

// Derivatives
inline expr numeric::d(expr dx) const { return zero; }
inline expr symbol::d(expr dx) const
{
	if(is<symbol>(dx) && as<symbol>(dx).name() == _name) return one;
	if(_value == empty)	return zero;
	return df(_value, dx);
}

template <> inline expr fn_base<fn_id>::d(expr dx) const { return df(_x, dx); }									// id(f)' ⇒ f'
template <> inline expr fn_base<fn_ln>::d(expr dx) const { return df(_x, dx) / _x; }							// ln(f)' ⇒ f'/x
template <> inline expr fn_base<fn_sin>::d(expr dx) const { return df(_x, dx) * cos(_x); }						// sin(f)' ⇒ f'∙cos(x)
template <> inline expr fn_base<fn_cos>::d(expr dx) const { return -df(_x, dx) * sin(_x); }						// cos(f)' ⇒ -f'∙sin(x)
template <> inline expr fn_base<fn_tg>::d(expr dx) const { return df(_x, dx) / (cos(_x) ^ two); }				// tg(f)' ⇒ f'/cos²(x)
template <> inline expr fn_base<fn_arcsin>::d(expr dx) const { return df(_x, dx) / ((1-(_x^2)) ^ half); }		// arcsin(f)' ⇒ f'/√(1-x²)
template <> inline expr fn_base<fn_arccos>::d(expr dx) const { return -df(_x, dx) / ((1 - (_x ^ 2)) ^ half); }	// arccos(f)' ⇒ -f'/√(1-x²)
template <> inline expr fn_base<fn_arctg>::d(expr dx) const { return df(_x, dx) / (1 + (_x ^ 2)); }				// arctg(f)' ⇒ f'/√(1+x²)
template <> inline expr fn_base<fn_int>::d(expr dx) const { return dx == (*this)[1] ? (*this)[0] : func{fn_dif{xset{**this, dx}}}; }
template<class F> expr fn_base<F>::d(expr dx) const { return func{fn_dif{xset{**this, dx}}}; }
inline expr fn_user::d(expr dx) const { return fn(name(), df(body(), dx), args()); }

inline expr power::d(expr dx) const { return (_x^_y) * (df(_y, dx)*ln(_x) + _y / _x*df(_x, dx)); }				// (fᵍ)' ⇒ fᵍ∙[g'∙ln(f)+g∙f'/f]
inline expr product::d(expr dx) const { return df(_left, dx) * _right + _left * df(_right, dx); }				// (f∙g)' ⇒ f'∙g + f∙g'
inline expr sum::d(expr dx) const { return df(_left, dx) + df(_right, dx); }									// (f+g)' ⇒ f' + g'
inline expr xset::d(expr dx) const {																			// {f, g}' ⇒ {f', g'}
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx](auto e) {return df(e, dx); });
	return{ret};
}

// Integrals

inline expr numeric::integrate(expr dx, expr c) const { return expr{_value} *dx + c; }
inline expr symbol::integrate(expr dx, expr c) const {
	return is<symbol>(dx) && _name == as<symbol>(dx).name() ? (dx ^ 2) / 2 + c : *this * dx + c;
}

template <> inline expr fn_base<fn_id>::integrate(expr dx, expr c) const { return cas::intf(_x, dx, c); }
template <> inline expr fn_base<fn_ln>::integrate(expr dx, expr c) const { auto a = df(_x, dx); return a != zero && (df(a, dx) == zero) ? _x / a * ln(_x) - dx + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_sin>::integrate(expr dx, expr c) const { return _x == dx ? -cos(_x) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_cos>::integrate(expr dx, expr c) const { return _x == dx ? sin(_x) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_tg>::integrate(expr dx, expr c) const { return _x == dx ? -ln(cos(_x)) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_arcsin>::integrate(expr dx, expr c) const { return _x == dx ? _x * arcsin(_x) + ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_arccos>::integrate(expr dx, expr c) const { return _x == dx ? _x * arccos(_x) - ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_arctg>::integrate(expr dx, expr c) const { return _x == dx ? _x * arctg(_x) - half * ln(1 + (_x ^ 2)) + c : make_integral(**this, dx) + c; }
template <> inline expr fn_base<fn_dif>::integrate(expr dx, expr c) const { return dx == (*this)[1] ? (*this)[0] : make_integral(**this, dx) + c; }
template<class F> expr fn_base<F>::integrate(expr dx, expr c) const { return make_integral(**this, dx) + c; }

inline expr fn_user::integrate(expr dx, expr c) const { return fn(name(), intf(body(), dx, c), args()); }

inline expr power::integrate(expr dx, expr c) const
{
	auto d_x = df(_x, dx);
	if(d_x == zero && _y == dx)	return (_x ^ _y) / ln(_x) + c;					// ∫ aˣ dx ⇒ aˣ / ln(a)
	auto d_y = df(_y, dx);
	if(d_y == zero && d_x == zero)	return *this * dx;							// ∫ aᵇ dx ⇒ aᵇ∙x
	if(d_y == zero && df(d_x, dx) == zero)	{
		return _y == minus_one ? 
			ln(_x) / d_x + c :													// ∫ 1/(ax+b) dx ⇒ ln(ax+b)/a
			(_x ^ (_y + 1)) / (d_x * (_y + 1)) + c;								// ∫ (ax+b)ⁿ dx ⇒ (ax+b)ⁿ⁺¹/a(n+1)
	}
	return make_integral(*this, dx) + c;
}

inline expr product::integrate(expr dx, expr c) const {
	if(df(_left, dx) == zero)	return _left * cas::intf(_right, dx, c);		// ∫ a∙f(x) dx ⇒ a∙∫ f(x) dx
	if(df(_right, dx) == zero)	return _right * cas::intf(_left, dx, c);		// ∫ f(x)∙a dx ⇒ a∙∫ f(x) dx

	product p{*this};
	for(auto it = p.begin(); it != p.end(); ++it) {
		if(df(*it, dx) == zero) {
			auto e = *it;
			p.erase(it);
			return e * p.integrate(dx, c);
		}
	}

	return make_integral(make_prod(p.left(), p.right()), dx) + c;
}

inline expr sum::integrate(expr dx, expr c) const { return cas::intf(_left, dx, c) + cas::intf(_right, dx, c); }

inline expr xset::integrate(expr dx, expr c) const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx, c](auto e) {return intf(e, dx, c); });
	return{ret};
}

}
