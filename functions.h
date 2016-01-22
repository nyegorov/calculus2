#pragma once

#include "common.h"
#include "numeric.h"

namespace cas {

expr ln(expr x);
expr sin(expr x);
expr cos(expr x);
expr tg(expr x);
expr arcsin(expr x);
expr arccos(expr x);
expr arctg(expr x);
expr fn(string name, expr body, list_t args);

template<> static expr fn_base<fn_ln>::make(expr x)
{
	if(x == zero)		return minf;																// ln(0) ⇒ -∞
	if(x == one)		return zero;																// ln(1) ⇒ 0
	if(x == e)			return one;																	// ln(e) ⇒ 1
	if(x == inf)		return inf;																	// ln(∞) ⇒ ∞
	if(is<product>(x))	return ln(as<product>(x).left()) + ln(as<product>(x).right());				// ln(x∙y) ⇒ ln(x)+ln(y)
	if(is<power>(x))	return as<power>(x).y() * ln(as<power>(x).x());								// ln(xʸ) ⇒ y∙ln(x)
	return func{fn_ln{x}};
}
template<> static expr fn_base<fn_sin>::make(expr x)
{
	if(x == zero || x == pi || x == 2*pi)	return zero;											// sin(0), sin(π), sin(2π) ⇒ 0
	if(x == pi/6 || x == 5*pi/6)	return half;													// sin(π/6), sin(5π/6) ⇒ 1/2
	if(x == pi/3 || x == 2*pi/3)	return (3 ^ half) / 2;											// sin(π/3), sin(2π/3) ⇒ √3/2
	if(x == 7*pi/6 || x == 11*pi/6)	return -half;													// sin(7π/6), sin(11π/6) ⇒ -1/2
	if(x == 4*pi/3 || x == 5*pi/3)	return -(3 ^ half) / 2;											// sin(4π/3), sin(5π/3) ⇒ -√3/2
	if(x == pi/4 || x == 3*pi/4)	return (2 ^ half) / 2;											// sin(π/4), sin(3π/4) ⇒ √2/2
	if(x == 5*pi/4 || x == 7*pi/4)	return -(2 ^ half) / 2;											// sin(5π/4), sin(7π/4) ⇒ -√2/2
	if(x == pi/2)					return one;														// sin(π/2) ⇒ 1
	if(x == 2*pi/2)					return minus_one;												// sin(3π/2) ⇒ -1
	if(is<product>(x) && as<product>(x).left() == minus_one) return -sin(as<product>(x).right());	// sin(-x) ⇒ -sin(x)
	if(is<func, fn_arcsin>(x))		return as<func, fn_arcsin>(x).x();								// sin(arcsin(x)) ⇒ x
	return func{fn_sin{x}};
}
template<> static expr fn_base<fn_cos>::make(expr x)
{
	if(x == pi/2 || x == 3*pi/2)	return zero;													// cos(π/2), cos(3π/2) ⇒ 0
	if(x == pi/3 || x == 5*pi/3)	return half;													// cos(π/3), cos(5π/3) ⇒ 1/2
	if(x == pi/6 || x == 11*pi/6)	return (3 ^ half) / 2;											// cos(π/6), cos(11π/6) ⇒ √3/2
	if(x == 2*pi/3 || x == 4*pi/3)	return -half;													// cos(2π/3), cos(4π/3) ⇒ -1/2
	if(x == 5*pi/6 || x == 7*pi/6)	return -(3 ^ half) / 2;											// cos(5π/6), cos(7π/6) ⇒ -√3/2
	if(x == pi/4 || x == 7*pi/4)	return (2 ^ half) / 2;											// cos(π/4), cos(7π/4) ⇒ √2/2
	if(x == 3*pi/4 || x == 5*pi/4)	return -(2 ^ half) / 2;											// cos(3π/4), cos(5π/4) ⇒ -√2/2
	if(x == zero || x == 2*pi)		return one;														// cos(0), cos(2π) ⇒ 1
	if(x == pi)						return minus_one;												// cos(π) ⇒ -1
	if(is<product>(x) && as<product>(x).left() == minus_one)return cos(as<product>(x).right());		// cos(-x) ⇒ cos(x)
	if(is<func, fn_arccos>(x))		return as<func, fn_arccos>(x).x();								// cos(arccos(x)) ⇒ x
	return func{fn_cos{x}};
}

template<> static expr fn_base<fn_tg>::make(expr x) {
	if(is<product>(x) && as<product>(x).left() == minus_one)return -tg(as<product>(x).right());		// tg(-x) ⇒ -tg(x)
	if(is<func, fn_arctg>(x))		return as<func, fn_arctg>(x).x();								// tg(arctg(x)) ⇒ x
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

template<> static expr fn_base<fn_user>::make(expr x) { return func{fn_user{x}}; };

template<> static expr fn_base<fn_int>::make(expr p) {
	if(!is<xset>(p) || as<xset>(p).items().size() < 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(p).items();
	switch(params.size()) {
	case 2:	return intf(params[0], params[1]);
	case 3:	return intf(params[0], params[1], params[2]);
	default:return intf(params[0], params[1], params[2], params[3]);
	}
}

template<> static expr fn_base<fn_dif>::make(expr p) {
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	expr f = as<xset>(p).items().front(), dx = as<xset>(p).items().back();
	return df(f, dx);
}

template<> static expr fn_base<fn_assign>::make(expr p) { 
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(p).items();
	return params[1];
};
template<> static expr fn_base<fn_subst>::make(expr p) { 
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(p).items();
	if(!is<xset>(params[1]) || as<xset>(params[1]).items().size() != 2)	throw error_t::syntax;
	return cas::subst(params[0], as<xset>(params[1]).items()[0], as<xset>(params[1]).items()[1]);
};


inline expr make_integral(expr f, expr dx)					{ return func{fn_int{xset{f, dx}}}; }
inline expr make_integral(expr f, expr dx, expr a, expr b)	{ return func{fn_int{xset{f, dx, a, b}}}; }

// User-defined functions
inline fn_user::fn_user(string name, expr body, list_t args) : fn_base(xset{symbol{name}, body, args}) {}
inline string fn_user::name() const { return as<symbol>((*this)[0]).name(); }
inline expr fn_user::body() const { return (*this)[1]; }
inline list_t fn_user::args() const { return as<xset>((*this)[2]).items(); }
inline expr fn_user::operator()(xset values) const { 
	return body() == empty ?
		make(x() | std::make_pair(as<xset>((*this)[2]), values)) : 
		body() | std::make_pair(as<xset>((*this)[2]), values); 
}
inline expr fn_user::operator ()() const { return func{*this}; }
template <typename ... Params> expr fn_user::operator ()(expr val, Params ... rest) const
{
	list_t rest_args = args();
	if(rest_args.empty())	return error{error_t::syntax};
	expr arg = rest_args.front();
	rest_args.erase(rest_args.begin());
	return rest_args.empty() ? 
		body() | std::make_pair(arg, val) : 
		fn_user{name(), body() | std::make_pair(arg, val), rest_args}(rest...);
}

static expr apply_fun(expr x, real_t rfun(real_t x), complex_t cfun(complex_t x))
{
	if(is<numeric, int_t>(x))		return make_num(rfun((real_t)as<numeric, int_t>(x)));
	if(is<numeric, real_t>(x))		return make_num(rfun(as<numeric, real_t>(x)));
	if(is<numeric, complex_t>(x))	return make_num(cfun(as<numeric, complex_t>(x)));
	return x;
}
template<class F> expr fn_base<F>::approx() const { return fn_base<F>::make(~_x); }
template<class F> expr fn_base<F>::subst(pair<expr, expr> s) const { return expr{func{F{_x}}} == s.first ? s.second : make(_x | s); }
template<class F> bool fn_base<F>::match(function_t f, match_result& res) const { return f.type() == typeid(F) ? cas::match(boost::get<F>(f).x(), _x, res) : res.found = false; }
template<class F> unsigned fn_base<F>::exponents(const list_t& vars) const { auto c = get_exps(_x, vars); return c ? 99 : 0; }
template<class F> expr operator *(fn_base<F> f) { return f.make(f.x()); }

template<> expr inline fn_base<fn_ln>::approx() const { return apply_fun(~_x, log, [](complex_t x) {return log(x); }); }
template<> expr inline fn_base<fn_sin>::approx() const { return apply_fun(~_x, std::sin, [](complex_t x) {return sin(x); }); }
template<> expr inline fn_base<fn_cos>::approx() const { return apply_fun(~_x, std::cos, [](complex_t x) {return cos(x); }); }
template<> expr inline fn_base<fn_tg>::approx() const { return apply_fun(~_x, tan, [](complex_t x) {return tan(x); }); }
template<> expr inline fn_base<fn_arcsin>::approx() const { return apply_fun(~_x, asin, [](complex_t x) {return asin(x); }); }
template<> expr inline fn_base<fn_arccos>::approx() const { return apply_fun(~_x, acos, [](complex_t x) {return acos(x); }); }
template<> expr inline fn_base<fn_arctg>::approx() const { return apply_fun(~_x, atan, [](complex_t x) {return atan(x); }); }
template<> expr inline fn_base<fn_int>::approx() const { return make_integral(~(*this)[0], (*this)[1]); }
template<> expr inline fn_base<fn_assign>::approx() const { return make(~_x); }
template<> expr inline fn_base<fn_subst>::approx() const { return make(~_x); }

inline expr operator * (func f) { return boost::apply_visitor([](auto f) { return *f; }, f.value()); }

inline expr func::d(expr dx) const { return boost::apply_visitor([dx](auto x) { return x.d(dx); }, _func); }
inline expr func::integrate(expr dx, expr c) const { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, _func); }
inline expr func::subst(pair<expr, expr> s) const { return boost::apply_visitor([s](auto x) { return x.subst(s); }, _func);}
inline expr func::approx() const { return boost::apply_visitor([](auto x) { return x.approx(); }, _func); }
inline unsigned func::exponents(const list_t& vars) const { return boost::apply_visitor([&vars](auto x) { return x.exponents(vars); }, _func); }
inline bool func::match(expr e, match_result& res) const {
	return is<func>(e) ? 
		boost::apply_visitor([fun = as<func>(e).value(), &res](auto f) { return f.match(fun, res); }, _func) :
		res.found = false;
}

inline expr ln(expr x) { return fn_ln::make(x); }
inline expr sin(expr x) { return fn_sin::make(x); }
inline expr cos(expr x) { return fn_cos::make(x); }
inline expr tg(expr x) { return fn_tg::make(x); }
inline expr arcsin(expr x) { return fn_arcsin::make(x); }
inline expr arccos(expr x) { return fn_arccos::make(x); }
inline expr arctg(expr x) { return fn_arctg::make(x); }
inline expr fn(string name, expr body, list_t args) { return func{fn_user{name, body, args}}; }

}