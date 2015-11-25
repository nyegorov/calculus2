#pragma once

#include "common.h"
#include "numeric.h"

namespace cas {

template<> static expr fn_base<fn_ln>::make(expr x);
template<> static expr fn_base<fn_sin>::make(expr x);
template<> static expr fn_base<fn_cos>::make(expr x);
template<> static expr fn_base<fn_tg>::make(expr x);
template<> static expr fn_base<fn_arcsin>::make(expr x);
template<> static expr fn_base<fn_arccos>::make(expr x);
template<> static expr fn_base<fn_arctg>::make(expr x);

expr ln(expr x) { return fn_ln::make(x); }
expr sin(expr x) { return fn_sin::make(x); }
expr cos(expr x) { return fn_cos::make(x); }
expr tg(expr x) { return fn_tg::make(x); }
expr arcsin(expr x) { return fn_arcsin::make(x); }
expr arccos(expr x) { return fn_arccos::make(x); }
expr arctg(expr x) { return fn_arctg::make(x); }

template<> static expr fn_base<fn_ln>::make(expr x)
{
	if(x == zero)		return minf;		// ln(0) => -inf
	if(x == one)		return zero;		// ln(1) => 0
	if(x == e)			return one;			// ln(e) => 1
	if(is<product>(x))	return ln(as<product>(x).left()) + ln(as<product>(x).right());	// ln(x*y) => ln(x)+ln(y)
	if(is<power>(x))	return as<power>(x).y() * ln(as<power>(x).x());					// ln(x^y) => y*ln(x)
	return func{fn_ln{x}};
}
template<> static expr fn_base<fn_sin>::make(expr x)
{
	if(x == zero || x == pi || x == 2*pi)	return zero;	// sin(0), sin(π), sin(2π) => 0
	if(x == pi/6 || x == 5*pi/6)	return half;			// sin(π/6), sin(5π/6) => 1/2
	if(x == pi/3 || x == 2*pi/3)	return (3 ^ half) / 2;	// sin(π/3), sin(2π/3) => π3/2
	if(x == 7*pi/6 || x == 11*pi/6)	return -half;			// sin(7π/6), sin(11π/6) => -1/2
	if(x == 4*pi/3 || x == 5*pi/3)	return -(3 ^ half) / 2;	// sin(4π/3), sin(5π/3) => -π3/2
	if(x == pi/4 || x == 3*pi/4)	return (2 ^ half) / 2;	// sin(π/4), sin(3π/4) => π2/2
	if(x == 5*pi/4 || x == 7*pi/4)	return -(2 ^ half) / 2;	// sin(5π/4), sin(7π/4) => -π2/2
	if(x == pi/2)					return one;				// sin(π/2) => 1
	if(x == 2*pi/2)					return minus_one;		// sin(3π/2) => -1
	if(is<product>(x) && as<product>(x).left() == minus_one)return -1 * make(as<product>(x).right());	// sin(-x) => -sin(x)
	if(is<func>(x) && is<fn_arcsin>(as<func>(x).f()))		return as<fn_arcsin>(as<func>(x).f()).x();	// sin(arcsin(x)) => x
	return func{fn_sin{x}};
}
template<> static expr fn_base<fn_cos>::make(expr x)
{
	if(x == pi/2 || x == 3*pi/2)	return zero;			// cos(π/2), cos(3π/2) => 0
	if(x == pi/3 || x == 5*pi/3)	return half;			// cos(π/3), cos(5π/3) => 1/2
	if(x == pi/6 || x == 11*pi/6)	return (3 ^ half) / 2;	// cos(π/6), cos(11π/6) => π3/2
	if(x == 2*pi/3 || x == 4*pi/3)	return -half;			// cos(2π/3), cos(4π/3) => -1/2
	if(x == 5*pi/6 || x == 7*pi/6)	return -(3 ^ half) / 2;	// cos(5π/6), cos(7π/6) => -π3/2
	if(x == pi/4 || x == 7*pi/4)	return (2 ^ half) / 2;	// cos(π/4), cos(7π/4) => π2/2
	if(x == 3*pi/4 || x == 5*pi/4)	return -(2 ^ half) / 2;	// cos(3π/4), cos(5π/4) => -π2/2
	if(x == zero || x == 2*pi)		return one;				// cos(0), cos(2π) => 1
	if(x == pi)						return minus_one;		// cos(π) => -1
	if(is<product>(x) && as<product>(x).left() == minus_one)return make(as<product>(x).right());		// cos(-x) => cos(x)
	if(is<func>(x) && is<fn_arccos>(as<func>(x).f()))		return as<fn_arccos>(as<func>(x).f()).x();	// cos(arccos(x)) => x
	return func{fn_cos{x}};
}

template<> static expr fn_base<fn_tg>::make(expr x) {
	if(is<product>(x) && as<product>(x).left() == minus_one)return -1 * make(as<product>(x).right());	// tg(-x) => -tg(x)
	return func{fn_tg{x}};
}
template<> static expr fn_base<fn_arcsin>::make(expr x)	{
	return func{fn_arcsin{x}};
}
template<> static expr fn_base<fn_arccos>::make(expr x)	{
	return func{fn_arccos{x}};
}
template<> static expr fn_base<fn_arctg>::make(expr x)	{
	return func{fn_arctg{x}};
}

expr apply_fun(expr x, real_t rfun(real_t x), complex_t cfun(complex_t x))
{
	if(is<real>(x))		return make_real(rfun(as<real>(x).value()));
	if(is<complex>(x))	return make_complex(cfun(as<complex>(x).value()));
	return x;
}
template<class F> expr fn_base<F>::subst(pair<expr, expr> s) const { return expr{func{F{_x}}} == s.first ? s.second : make(_x | s); }
template<> expr fn_base<fn_id>::approx() const { return ~_x; }
template<> expr fn_base<fn_ln>::approx() const { return apply_fun(~_x, log, [](complex_t x) {return log(x); }); }
template<> expr fn_base<fn_sin>::approx() const { return apply_fun(~_x, std::sin, [](complex_t x) {return sin(x); }); }
template<> expr fn_base<fn_cos>::approx() const { return apply_fun(~_x, std::cos, [](complex_t x) {return cos(x); }); }
template<> expr fn_base<fn_tg>::approx() const { return apply_fun(~_x, tan, [](complex_t x) {return tan(x); }); }
template<> expr fn_base<fn_arcsin>::approx() const { return apply_fun(~_x, asin, [](complex_t x) {return asin(x); }); }
template<> expr fn_base<fn_arccos>::approx() const { return apply_fun(~_x, acos, [](complex_t x) {return acos(x); }); }
template<> expr fn_base<fn_arctg>::approx() const { return apply_fun(~_x, atan, [](complex_t x) {return atan(x); }); }
template<class F> match_result fn_base<F>::match(function_t f, match_result res) const { 
	return f.type() == typeid(F) ? 
		cas::match(boost::get<F>(f).x(), _x, res) : 
		(res.found = false, res);
}

template<class F> static expr operator *(F f) { return f.make(f.x()); }
expr operator *(fn_int f) { return make_integral(f.f(), f.dx()); }

expr fn_int::d(expr dx) const { return dx == _dx ? _fun : make_err(error_t::not_implemented); }
expr fn_int::integrate(expr dx, expr c) const { return make_integral(func{*this}, dx); }
expr fn_int::subst(pair<expr, expr> s) const { return make_integral(_fun | s, _dx | s); }
expr fn_int::approx() const { return func{*this}; }
match_result fn_int::match(function_t f, match_result res) const { 
	return f.type() == typeid(fn_int) ? 
		cas::match(boost::get<fn_int>(f).dx(), _dx, cas::match(boost::get<fn_int>(f).f(), _fun, res)) :
		(res.found = false, res); 
}


expr operator * (func f) { return boost::apply_visitor([](auto f) { return *f; }, f.f()); }						 
expr operator + (func lh, func rh) { return make_sum(*lh, *rh); }
expr operator * (func lh, func rh) { return make_prod(*lh, *rh); }
expr operator ^ (func lh, func rh) { return make_power(*lh, *rh); }

expr func::subst(pair<expr, expr> s) const { return boost::apply_visitor([s](auto x) { return x.subst(s); }, _func);}
expr func::approx() const { return boost::apply_visitor([](auto x) { return x.approx(); }, _func); }
match_result func::match(expr e, match_result res) const { 
	return is<func>(e) ? 
		boost::apply_visitor([fun = as<func>(e).f(), &res](auto f) { return f.match(fun, res); }, _func) :
		(res.found = false, res);
}

}