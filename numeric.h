#pragma once

#include "common.h"

namespace cas {

	template<class T> T div(T x, T y, T& r) { T d = x / y; r = x - d*y; return d; }
	template<class T> T pwr(T x, T y) { T t; return y == 0 ? 1 : y % 2 == 0 ? t = pwr(x, y / 2), t*t : x*pwr(x, y - 1); }
	int_t gcd(int_t a, int_t b) { return b == 0 ? labs(a) : gcd(b, a % b); }
	void normalize(int_t& a, int_t& b) { auto d = gcd(a, b); if(d) a /= d, b /= d; }
	template<class T> T isqrt(T n) {
		T b = 0;
		while(n >= 0) { n = n - b; b = b + 1; n = n - b; }
		return b - 1;
	}
	template<class T> void powerize(T& x, T&e) {
		T n = isqrt(x), y = x;
		for(x = 2; x <= n; x++)
			for(e = n + 1; e>1; e--)	if(pwr(x, e) == y)	return;
		x = y; e = 1;
	}
	int_t binomial(int_t n, int_t k) {
		int_t a = 1, b = 1;
		for(int_t l = 1; l <= k; l++)	a *= (n - l + 1), b *= l;
		return a / b;
	}

	const expr zero = integer{ 0 };
	const expr one = integer{ 1 };
	const expr two = integer{ 2 };
	const expr half = rational{1, 2};
	const expr minus_one = integer{ -1 };
	const expr inf = rational{ 1, 0 };
	const expr minf = rational{ -1, 0 };

	expr make_err(error_t err) { return error{ err }; }
	expr make_int(int_t value) { return integer{ value }; }
	expr make_rat(int_t numer, int_t denom) {
		normalize(numer, denom);
		if(numer == denom)	return one;
		if(denom == 1) 		return{ numer };
		if(denom < 0)		numer = -numer, denom = -denom;
		return rational{ numer, denom };
	}
	expr make_real(real_t value) { if(value - (int_t)value == 0) return integer{ (int_t)value }; else return real{ value, 1.0 }; }
	expr make_complex(complex_t value) {
		if(abs(value.imag()) <= std::numeric_limits<real_t>::epsilon())	return make_real(value.real());
		return complex{ value };
	}

	expr operator + (error lh, error rh) { return lh; }
	expr operator + (integer lh, integer rh) { return make_int(lh.value() + rh.value()); }
	expr operator + (rational lh, rational rh) { return make_rat(lh.numer() * rh.denom() + rh.numer() * lh.denom(), lh.denom() * rh.denom()); }
	expr operator + (real lh, real rh) { return make_real(lh.value() + rh.value()); }
	expr operator + (complex lh, complex rh) { return make_complex(lh.value() + rh.value()); }
	expr operator * (error lh, error rh) { return lh; }
	expr operator * (integer lh, integer rh) { return make_int(lh.value() * rh.value()); }
	expr operator * (rational lh, rational rh) { return make_rat(lh.numer() * rh.numer(), lh.denom() * rh.denom()); }
	expr operator * (real lh, real rh) { return make_real(lh.value() * rh.value()); }
	expr operator * (complex lh, complex rh) { return make_complex(lh.value() * rh.value()); }
	expr operator ^ (error lh, error rh) { return lh; }
	expr operator ^ (integer lh, integer rh) { return rh.value() < 0 ? make_rat(1, pwr(lh.value(), -rh.value())) : make_int(pwr(lh.value(), rh.value())); }
	expr operator ^ (rational lh, integer rh) { return rh.value() < 0 ? make_rat(pwr(lh.denom(), -rh.value()), pwr(lh.numer(), -rh.value())) : make_rat(pwr(lh.numer(), rh.value()), pwr(lh.denom(), rh.value())); }
	expr operator ^ (integer lh, rational rh) {
		int_t x = lh.value(), e, a = rh.numer(), b = rh.denom();
		if(x == 1)	return one;
		if(x == -1)	return b % 2 ? minus_one : expr{ complex{ { -1.0, 0.0 } } } ^ rh;
		powerize(x, e);	 e *= abs(a);
		normalize(e, b); x = pwr(x, e);
		if(b == 1)	return a < 0 ? make_rat(1, x) : make_int(x);

		return make_power(make_int(x), make_rat(a < 0 ? -1 : 1, b));
	}
	expr operator ^ (rational lh, rational rh) { return (expr(lh.numer()) ^ rh) / (expr(lh.denom()) ^ rh); }
	expr operator ^ (real lh, real rh) { return make_real(pow(lh.value(), rh.value())); }
	expr operator ^ (complex lh, complex rh) { return make_complex(pow(lh.value(), rh.value())); }

	expr integer::approx() const { return *this; };
	expr rational::approx() const { return make_real(_denom ? (real_t)_numer / (real_t)_denom : _numer > 0 ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity()); };
	expr real::approx() const { return *this; };
	expr complex::approx() const { return *this; };

}