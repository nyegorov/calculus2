#pragma once

#include "common.h"

namespace {
	using namespace cas;
	expr pow(int_t lh, int_t rh);
	rational_t pow(rational_t lh, int_t rh);
	expr pow(int_t x, rational_t rh);
	real_t pow(rational_t lh, real_t rh);
	real_t pow(real_t lh, rational_t rh);
	complex_t pow(rational_t lh, complex_t rh);
	complex_t pow(complex_t lh, rational_t rh);
	expr pow(rational_t lh, rational_t rh);
}

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

	const expr zero = numeric{ 0 };
	const expr one = numeric{ 1 };
	const expr two = numeric{ 2 };
	const expr half = numeric{1, 2};
	const expr minus_one = numeric{ -1 };
	const expr inf = numeric{ 1, 0 };
	const expr minf = numeric{ -1, 0 };

	expr make_num(expr value) { return value; }
	expr make_num(rational_t value) { return make_num(value.numer(), value.denom()); }
	expr make_num(int_t value) { return numeric_t{value}; }
	expr make_num(int_t numer, int_t denom) {
		normalize(numer, denom);
		if(numer == denom)	return 1;
		if(denom == 1) 		return{numer};
		if(denom < 0)		numer = -numer, denom = -denom;
		return numeric_t{rational_t{numer, denom}};
	}
	expr make_num(real_t value) { if(value - (int_t)value == 0) return numeric_t{(int_t)value}; else return numeric_t{value}; }
	expr make_num(real_t real, real_t imag) { return make_num(complex_t{real, imag}); }
	expr make_num(complex_t value) {
		if(abs(value.imag()) <= std::numeric_limits<real_t>::epsilon())	return make_num(value.real());
		return numeric_t{value};
	}

	rational_t::rational_t(int_t numer, int_t denom) : _numer(numer), _denom(denom) { normalize(_numer, _denom); }

/*
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
	expr complex::approx() const { return *this; };*/

	expr numeric::approx() const { return _value.type() == typeid(rational_t) ? make_num(boost::get<rational_t>(_value).value()) : *this; };
	expr numeric::subst(pair<expr, expr> s) const { return{*this}; }
	bool numeric::match(expr e, match_result& res) const { if(e != expr{*this}) res.found = false; return res; };

	rational_t operator + (rational_t lh, int_t rh) { return {lh.numer() + lh.denom() * rh, lh.denom()}; }
	rational_t operator + (int_t lh, rational_t rh) { return {rh.numer() + rh.denom() * lh, rh.denom()}; }
	real_t operator + (rational_t lh, real_t rh) { return lh.value() + rh; }
	real_t operator + (real_t lh, rational_t rh) { return lh + rh.value(); }
	complex_t operator + (rational_t lh, complex_t rh) { return lh.value() + rh; }
	complex_t operator + (complex_t lh, rational_t rh) { return lh + rh.value(); }
	rational_t operator + (rational_t lh, rational_t rh) { return{lh.numer() * rh.denom() + rh.numer() * lh.denom(), lh.denom() * rh.denom()}; }
	complex_t operator + (complex_t lh, int_t rh) { return lh + (real_t)rh; }
	complex_t operator + (int_t lh, complex_t rh) { return (real_t)lh + rh; }

	rational_t operator * (rational_t lh, int_t rh) { return {lh.numer() * rh, lh.denom()}; }
	rational_t operator * (int_t lh, rational_t rh) { return {rh.numer() * lh, rh.denom()}; }
	real_t operator * (rational_t lh, real_t rh) { return lh.value() * rh; }
	real_t operator * (real_t lh, rational_t rh) { return lh * rh.value(); }
	complex_t operator * (rational_t lh, complex_t rh) { return lh.value() * rh; }
	complex_t operator * (complex_t lh, rational_t rh) { return lh * rh.value(); }
	rational_t operator * (rational_t lh, rational_t rh) { return{lh.numer() * rh.numer(), lh.denom() * rh.denom()}; }
	complex_t operator * (complex_t lh, int_t rh) { return lh * (real_t)rh; }
	complex_t operator * (int_t lh, complex_t rh) { return (real_t)lh * rh; }

	struct num_less : public boost::static_visitor<bool>
	{
	public:
		template <typename T, typename U> bool operator()(T lh, U rh) const { return lh < rh; }
		template <typename T> bool operator()(complex_t lh, T rh) const { return lh.real() < (real_t)rh; }
		template <typename T> bool operator()(T lh, complex_t rh) const { return (real_t)lh < rh.real(); }
		bool operator()(complex_t lh, complex_t rh) const { return abs(lh) < abs(rh); }
	};

	expr add(numeric_t op1, numeric_t op2) { return boost::apply_visitor([](auto x, auto y) {return make_num(x + y); }, op1, op2); }
	expr mul(numeric_t op1, numeric_t op2) { return boost::apply_visitor([](auto x, auto y) {return make_num(x * y); }, op1, op2); }
	expr powr(numeric_t op1, numeric_t op2) { return boost::apply_visitor([](auto x, auto y) {return make_num(pow(x, y)); }, op1, op2); }
	bool less(numeric_t op1, numeric_t op2) { return boost::apply_visitor(num_less(), op1, op2); }
}

namespace {
	using namespace cas;
	expr pow(int_t lh, int_t rh) { return rh < 0 ? make_num(1, pwr(lh, -rh)) : pwr(lh, rh); }
	rational_t pow(rational_t lh, int_t rh) { return rh < 0 ? rational_t{pwr(lh.denom(), -rh), pwr(lh.numer(), -rh)} : rational_t{pwr(lh.numer(), rh), pwr(lh.denom(), rh)}; }
	expr pow(int_t x, rational_t rh) {
		int_t e, a = rh.numer(), b = rh.denom();
		if(x == 0)	return zero;
		if(x == 1)	return one;
		if(x == -1)	return b % 2 ? minus_one : numeric_t{complex_t{-1.0, 0.0}} ^ rh;
		powerize(x, e);	 e *= abs(a);
		normalize(e, b); x = pwr(x, e);
		if(b == 1)	return a < 0 ? numeric_t{rational_t{1, x}} : numeric_t{x};

		return power{x, make_num(a < 0 ? -1 : 1, b)};
	}
	real_t pow(rational_t lh, real_t rh) { return std::pow(lh.value(), rh); }
	real_t pow(real_t lh, rational_t rh) { return std::pow(lh, rh.value()); }
	complex_t pow(rational_t lh, complex_t rh) { return std::pow(lh.value(), rh); }
	complex_t pow(complex_t lh, rational_t rh) { return std::pow(lh, rh.value()); }
	expr pow(rational_t lh, rational_t rh) { return (expr{lh.numer()} ^ rh) / (expr{lh.denom()} ^ rh); }
}
