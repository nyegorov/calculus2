#pragma once

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

// Derivatives
expr error::derive(expr dx) const { return _error == error_t::empty ? zero : *this; }

expr symbol::derive(expr dx) const
{
	if(is<symbol>(dx) && as<symbol>(dx).name() == _name) return one;
	return _value || dx;
}

expr func::derive(expr dx) const { return boost::apply_visitor([dx](auto x) { return x.derive(dx); }, _func); }

template <> expr fn_base<fn_id>::derive(expr dx) const { return _x || dx; }
template <> expr fn_base<fn_ln>::derive(expr dx) const { return (_x || dx) / _x; }
template <> expr fn_base<fn_sin>::derive(expr dx) const { return (_x || dx) * cos(_x); }
template <> expr fn_base<fn_cos>::derive(expr dx) const { return -(_x || dx) * sin(_x); }
template <> expr fn_base<fn_tg>::derive(expr dx) const { return (_x || dx) / (cos(_x) ^ two); }
template <> expr fn_base<fn_arcsin>::derive(expr dx) const { return (_x || dx) / ((1-(_x^2)) ^ half); }
template <> expr fn_base<fn_arccos>::derive(expr dx) const { return -(_x || dx) / ((1 - (_x ^ 2)) ^ half); }
template <> expr fn_base<fn_arctg>::derive(expr dx) const { return (_x || dx) / (1 + (_x ^ 2)); }

expr power::derive(expr dx) const
{
	// (f^g)' = f^g * (g'*ln(f) + g*f'/f)
	return (_x^_y) * ((_y || dx)*ln(_x) + _y / _x*(_x || dx));
}

expr product::derive(expr dx) const { return (_left||dx) * _right + _left * (_right||dx); }

expr sum::derive(expr dx) const { return (_left||dx) + (_right||dx); }

// Integrals

expr make_integral(expr f, expr dx) {
	if((f || dx) == zero)	return f * dx;												// S y dx => yx
	match_result mr;
	symbol x{"x", dx}, y{"y"};
	//if((mr = cas::match(f, x*ln(x))))	return (dx ^ 2)*ln(x) / 2 - (dx ^ 2) / 4;		// S xln(x) dx => (x² lnx)/2 - x²/4
	//if((mr = cas::match(f, ln(x) / x)))	return half * (ln(dx) ^ 2);						// S ln(x)/x dx => 1/2 ln(x)²

	if(mr = cas::match(f, x*ln(y))) {													// S xln(ax+b) dx => (a²x²-b²)ln(ax+b)/2a²-x(ax-2b)/4a
		auto a = mr[y] || dx, b = mr[y] - a*x;
		if((a || dx) == zero)			return ((a^2)*(x^2)-(b^2))*ln(a*x+b)/(2*(a^2))-x*(a*x-2*b)/(4*a);
	}
	return func{fn_int{f, dx}};
}

//expr error::integrate(symbol dx, expr c) const { return _error == error_t::empty ? zero : *this; }
expr symbol::integrate(expr dx, expr c) const { 
	return is<symbol>(dx) && _name == as<symbol>(dx).name() ? (dx ^ 2) / 2 + c : *this * dx + c;
}

expr func::integrate(expr dx, expr c) const { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, _func); }

template <> expr fn_base<fn_id>::integrate(expr dx, expr c) const { return cas::integrate(_x, dx, c); }
template <> expr fn_base<fn_ln>::integrate(expr dx, expr c) const { auto a = _x || dx; return a != zero && ((a || dx) == zero) ? _x / a * ln(_x) - dx + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_sin>::integrate(expr dx, expr c) const { return _x == dx ? -cos(_x) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_cos>::integrate(expr dx, expr c) const { return _x == dx ? sin(_x) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_tg>::integrate(expr dx, expr c) const { return _x == dx ? -ln(cos(_x)) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arcsin>::integrate(expr dx, expr c) const { return _x == dx ? _x * arcsin(_x) + ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arccos>::integrate(expr dx, expr c) const { return _x == dx ? _x * arccos(_x) - ((1 - (_x ^ 2)) ^ half) + c : make_integral(**this, dx) + c; }
template <> expr fn_base<fn_arctg>::integrate(expr dx, expr c) const { return _x == dx ? _x * arctg(_x) - half * ln(1 + (_x ^ 2)) + c : make_integral(**this, dx) + c; }

expr power::integrate(expr dx, expr c) const
{
	if((_x || dx) == zero && _y == dx)	return make_power(_x, _y) / ln(_x) + c;		// S a^x dx => a^x / ln(a)
	auto df = _x || dx;
	if((_y || dx) == zero && (df || dx) == zero)	{
		return _y == minus_one ? 
			ln(_x) / df + c :														// S 1/(ax+b) dx => ln(ax+b)/a
			(_x ^ (_y + 1)) / (df * (_y + 1)) + c;									// S (ax+b)^n dx => (ax+b)^(n+1)/a(n+1)
	}
	return make_integral(*this, dx) + c;
}

expr product::integrate(expr dx, expr c) const { 
	if((_left  || dx) == zero)	return _left * cas::integrate(_right, dx, c);		// S af(x) dx => a S f(x) dx
	if((_right || dx) == zero)	return _right * cas::integrate(_left, dx, c);		// S f(x)a dx => a S f(x) dx

	product p{*this};
	for(auto it = p.begin(); it != p.end(); ++it) {
		if((*it || dx) == zero) {
			auto e = *it;
			p.erase(it);
			return e * p.integrate(dx, c);
		}
	}

	return make_integral(*p, dx) + c;
}

expr sum::integrate(expr dx, expr c) const { return cas::integrate(_left, dx, c) + cas::integrate(_right, dx, c); }

}
