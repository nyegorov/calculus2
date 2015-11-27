#pragma once

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

// Derivatives
expr error::d(expr dx) const { return _error == error_t::empty ? zero : *this; }

expr symbol::d(expr dx) const
{
	if(is<symbol>(dx) && as<symbol>(dx).name() == _name) return one;
	return df(_value, dx);
}

template <> expr fn_base<fn_id>::d(expr dx) const { return df(_x, dx); }
template <> expr fn_base<fn_ln>::d(expr dx) const { return df(_x, dx) / _x; }
template <> expr fn_base<fn_sin>::d(expr dx) const { return df(_x, dx) * cos(_x); }
template <> expr fn_base<fn_cos>::d(expr dx) const { return -df(_x, dx) * sin(_x); }
template <> expr fn_base<fn_tg>::d(expr dx) const { return df(_x, dx) / (cos(_x) ^ two); }
template <> expr fn_base<fn_arcsin>::d(expr dx) const { return df(_x, dx) / ((1-(_x^2)) ^ half); }
template <> expr fn_base<fn_arccos>::d(expr dx) const { return -df(_x, dx) / ((1 - (_x ^ 2)) ^ half); }
template <> expr fn_base<fn_arctg>::d(expr dx) const { return df(_x, dx) / (1 + (_x ^ 2)); }
template <> expr fn_base<fn_int>::d(expr dx) const { return dx == (*this)[1] ? (*this)[0] : fn_base<fn_dif>::make(list_t{**this, dx}); }
template<class F> expr fn_base<F>::d(expr dx) const { return fn_base<fn_dif>::make(list_t{**this, dx}); }

expr power::d(expr dx) const
{
	// (f^g)' = f^g * (g'*ln(f) + g*f'/f)
	return (_x^_y) * (df(_y, dx)*ln(_x) + _y / _x*df(_x, dx));
}

expr product::d(expr dx) const { return df(_left, dx) * _right + _left * df(_right, dx); }

expr sum::d(expr dx) const { return df(_left, dx) + df(_right, dx); }

expr xset::d(expr dx) const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx](auto e) {return df(e, dx); });
	return{ret};
}

// Integrals

expr make_integral(expr f, expr dx)
{
	return fn_base<fn_int>::make(list_t{f, dx});
}

//expr error::integrate(symbol dx, expr c) const { return _error == error_t::empty ? zero : *this; }
expr symbol::integrate(expr dx, expr c) const { 
	return is<symbol>(dx) && _name == as<symbol>(dx).name() ? (dx ^ 2) / 2 + c : *this * dx + c;
}

template <> expr fn_base<fn_id>::integrate(expr dx, expr c) const { return cas::intf(_x, dx, c); }
template <> expr fn_base<fn_ln>::integrate(expr dx, expr c) const { auto a = df(_x, dx); return a != zero && (df(a, dx) == zero) ? _x / a * ln(_x) - dx + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_sin>::integrate(expr dx, expr c) const { return _x == dx ? -cos(_x) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_cos>::integrate(expr dx, expr c) const { return _x == dx ? sin(_x) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_tg>::integrate(expr dx, expr c) const { return _x == dx ? -ln(cos(_x)) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arcsin>::integrate(expr dx, expr c) const { return _x == dx ? _x * arcsin(_x) + ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arccos>::integrate(expr dx, expr c) const { return _x == dx ? _x * arccos(_x) - ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arctg>::integrate(expr dx, expr c) const { return _x == dx ? _x * arctg(_x) - half * ln(1 + (_x ^ 2)) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_dif>::integrate(expr dx, expr c) const { return dx == (*this)[1] ? (*this)[0] : make_integral(**this, dx) + c; }
template<class F> expr fn_base<F>::integrate(expr dx, expr c) const { return make_integral(**this, dx) + c; }

expr power::integrate(expr dx, expr c) const
{
	auto d_x = df(_x, dx);
	if(d_x == zero && _y == dx)	return make_power(_x, _y) / ln(_x) + c;			// ∫ a^x dx => a^x / ln(a)
	if(df(_y, dx) == zero && df(d_x, dx) == zero)	{
		return _y == minus_one ? 
			ln(_x) / d_x + c :													// ∫ 1/(ax+b) dx => ln(ax+b)/a
			(_x ^ (_y + 1)) / (d_x * (_y + 1)) + c;								// ∫ (ax+b)^n dx => (ax+b)^(n+1)/a(n+1)
	}
	return make_integral(*this, dx) + c;
}

expr product::integrate(expr dx, expr c) const { 
	if(df(_left, dx) == zero)	return _left * cas::intf(_right, dx, c);		// ∫ af(x) dx => a S f(x) dx
	if(df(_right, dx) == zero)	return _right * cas::intf(_left, dx, c);		// ∫ f(x)a dx => a S f(x) dx

	product p{*this};
	for(auto it = p.begin(); it != p.end(); ++it) {
		if(df(*it, dx) == zero) {
			auto e = *it;
			p.erase(it);
			return e * p.integrate(dx, c);
		}
	}

	return make_integral(*p, dx) + c;
}

expr sum::integrate(expr dx, expr c) const { return cas::intf(_left, dx, c) + cas::intf(_right, dx, c); }

expr xset::integrate(expr dx, expr c) const { 
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [dx, c](auto e) {return intf(e, dx, c); });
	return{ret};
}

}
