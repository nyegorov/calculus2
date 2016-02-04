﻿#pragma once

#include "common.h"
#include "numeric.h"

namespace cas {

inline bool is_func(expr x, const char name[]) { return is<func>(x) && as<func>(x).name() == name; }

expr fn(string name, list_t args, expr body);
expr ln(expr x);
expr sin(expr x);
expr cos(expr x);
expr tg(expr x);
expr arcsin(expr x);
expr arccos(expr x);
expr arctg(expr x);

inline func::func(string name, list_t args, expr body) : _name(name), _args(args), _body(body), _impl{
	[=](expr x) { return fn(name, is<xset>(x) ? as<xset>(x).items() : list_t{x}, body); },
	[=](expr dx) {
		if(body != empty)	return df(body, dx);
		auto it = std::find_if(args.begin(), args.end(), [dx](auto x) {return df(x, dx) != zero; });
		return it == args.end() ? zero : df(*it, dx) * make_dif(fn(name, args, body), dx);
	},
	[=](expr dx) { auto f = fn(name, args, body);  return (df(f, dx) == zero ? f * dx : make_integral(f, dx)); },
//	[](expr x) { return fn(name, x, body); },
	print_fun
} {};

inline expr fn(string name, list_t args, expr body) { return func{name, args, body}; }

inline expr ln(expr x)
{
	if(x == zero)		return minf;																// ln(0) ⇒ -∞
	if(x == one)		return zero;																// ln(1) ⇒ 0
	if(x == e)			return one;																	// ln(e) ⇒ 1
	if(x == inf)		return inf;																	// ln(∞) ⇒ ∞
	if(is<product>(x))	return ln(as<product>(x).left()) + ln(as<product>(x).right());				// ln(x∙y) ⇒ ln(x)+ln(y)
	if(is<power>(x))	return as<power>(x).y() * ln(as<power>(x).x());								// ln(xʸ) ⇒ y∙ln(x)
	return func{S_LN, {x}, empty,{
		ln,
		[x](expr dx) { return df(x, dx) / x; },														// ln(f)' ⇒ f'/x
		[x](expr dx) { auto a = df(x, dx); return a != zero && (df(a, dx) == zero) ? x / a * ln(x) - dx : make_integral(ln(x), dx); }
	}};
}

inline expr sin(expr x)
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
	if(is<func>(x) && as<func>(x).name() == S_ASIN)	return as<func>(x).x();						// sin(arcsin(x)) ⇒ x
	return func{S_SIN, {x}, empty, {
		sin, 
		[x](expr dx) { return df(x, dx) * cos(x); },												// sin(f)' ⇒ f'∙cos(x)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? -cos(x) : make_integral(sin(x), dx); }
	}};
}

inline expr cos(expr x)
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
	if(is<product>(x) && as<product>(x).left() == minus_one) return cos(as<product>(x).right());	// cos(-x) ⇒ cos(x)
	if(is<func>(x) && as<func>(x).name() == S_ACOS)	return as<func>(x).x();						// cos(arccos(x)) ⇒ x
	return func{S_COS, {x}, empty,{
		cos,
		[x](expr dx) { return df(x, dx) * -sin(x); },												// cos(f)' ⇒ -f'∙sin(x)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? sin(x) : make_integral(cos(x), dx); }
	}};
}

inline expr tg(expr x) {
	if(is<product>(x) && as<product>(x).left() == minus_one)return -tg(as<product>(x).right());		// tg(-x) ⇒ -tg(x)
	if(is<func>(x) && as<func>(x).name() == S_ATG)		return as<func>(x).x();					// tg(arctg(x)) ⇒ x
	return func{S_TG, {x}, empty,{
		tg,
		[x](expr dx) { return df(x, dx) / (cos(x) ^ two); },										// tg(f)' ⇒ f'/cos²(x)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? -ln(cos(x)) : make_integral(tg(x), dx); }
	}};
}
inline expr arcsin(expr x)	{
	return func{S_ASIN,{x}, empty,{
		arcsin,
		[x](expr dx) { return df(x, dx) / ((1 - (x^2)) ^ half); },									// arcsin(f)' ⇒ f'/√(1-x²)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * arcsin(x) + ((1-(x^2))^half) : make_integral(arcsin(x), dx); }
	}};
}
inline expr arccos(expr x)	{
	return func{S_ACOS, {x}, empty,{
		arccos,
		[x](expr dx) { return -df(x, dx) / ((1 - (x^2)) ^ half); },									// arccos(f)' ⇒ -f'/√(1-x²)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * arccos(x) - ((1-(x^2))^half) : make_integral(arccos(x), dx); }
	}};
}
inline expr arctg(expr x)	{
	return func{S_ATG, {x}, empty,{
		arctg,
		[x](expr dx) { return df(x, dx) / ((1 + (x^2)) ^ half); },									// arctg(f)' ⇒ f'/√(1+x²)
		[x](expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * arctg(x) - half * ln(1+(x^2)) : make_integral(arctg(x), dx); }
	}};
}

/*template<> static expr fn_base<fn_user>::make(expr x) { return func{fn_user{x}}; };

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
	if(is<symbol>(params[0]))			return symbol{as<symbol>(params[0]).name(), params[1]};
	if(is<func, fn_user>(params[0])) {
		auto fn = as<func, fn_user>(params[0]);
		return symbol{fn.name(), func{fn_user{fn.name(), params[1], fn.args()}}};
	}
	return make_err(error_t::syntax);
};
template<> static expr fn_base<fn_subst>::make(expr p) { 
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(p).items();
	if(is<symbol>(params[1]))		return cas::subst(params[0], as<symbol>(params[1]));
	if(is<xset>(params[1]))			return cas::subst(params[0], as<xset>(params[1]).items());
	return make_err(error_t::syntax);
};

inline expr make_dif(expr f, expr dx)						{ return func{fn_dif{xset{f, dx}}}; }
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
inline expr fn_user::simplify() const
{ 
	expr res = body();
	list_t from, to;
	if(res == empty)	return make(*x());
	return *::subst(res, args());
}
inline expr fn_user::subst(pair<expr, expr> s) const
{
	if(is<symbol>(s.first) && is<func, fn_user>(s.second)) {
		auto& f = as<func, fn_user>(s.second);
		if(as<symbol>(s.first).name() == name() && f.name() == name() && f.args().size() == args().size())	return ::subst(f.body(), f[2], (*this)[2]);
	}
	return expr{func{*this}} == s.first ? s.second : fn(name(), args(), body() | s);
}

static expr apply_fun(expr x, real_t rfun(real_t x), complex_t cfun(complex_t x))
{
	if(is<numeric, int_t>(x))		return make_num(rfun((real_t)as<numeric, int_t>(x)));
	if(is<numeric, real_t>(x))		return make_num(rfun(as<numeric, real_t>(x)));
	if(is<numeric, complex_t>(x))	return make_num(cfun(as<numeric, complex_t>(x)));
	return x;
}
template<class F> expr fn_base<F>::approx() const { return fn_base<F>::make(~_x); }
template<class F> expr fn_base<F>::simplify() const {
	return fn_base<F>::make(*_x);
}
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
inline expr func::simplify() const { return boost::apply_visitor([](auto x) { return x.simplify(); }, _func); }
inline unsigned func::exponents(const list_t& vars) const { return boost::apply_visitor([&vars](auto x) { return x.exponents(vars); }, _func); }
inline bool func::match(expr e, match_result& res) const {
	return is<func>(e) ? 
		boost::apply_visitor([fun = as<func>(e).value(), &res](auto f) { return f.match(fun, res); }, _func) :
		res.found = false;
}
*/

inline expr fdif(expr x)
{
	if(!is<xset>(x) || as<xset>(x).items().size() != 2)	return make_err(error_t::syntax);
	expr f = as<xset>(x).items().front(), dx = as<xset>(x).items().back();
	return df(f, dx);
}

inline expr fint(expr x)
{
	if(!is<xset>(x) || as<xset>(x).items().size() < 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(x).items();
	switch(params.size()) {
	case 2:	return intf(params[0], params[1]);
	case 3:	return intf(params[0], params[1], params[2]);
	default:return intf(params[0], params[1], params[2], params[3]);
	}
}

static expr fass(expr x) {
	if(!is<xset>(x) || as<xset>(x).items().size() != 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(x).items();
	if(is<symbol>(params[0]))			return symbol{as<symbol>(params[0]).name(), params[1]};
	if(is<func>(params[0])) {
		auto f = as<func>(params[0]);
		return symbol{f.name(), fn(f.name(), f.args(), params[1])};
	}
	return make_err(error_t::syntax);
};
static expr fsub(expr x) {
	if(!is<xset>(x) || as<xset>(x).items().size() != 2)	return make_err(error_t::syntax);
	auto& params = as<xset>(x).items();
	if(is<symbol>(params[1]))		return cas::subst(params[0], as<symbol>(params[1]));
	if(is<xset>(params[1]))			return cas::subst(params[0], as<xset>(params[1]).items());
	return make_err(error_t::syntax);
};

inline ostream& print_dif(ostream& os, expr f, expr dx);
inline ostream& print_int(ostream& os, expr f, expr dx, expr a, expr b);

inline expr make_dif(expr f, expr dx) {
	return func{S_DIF, {f, dx}, empty, {
		fdif,
		[f, dx](expr dx2) { return df(df(f, dx), dx2); },
		[f, dx](expr dx2) { return dx2 == dx ? f : make_integral(df(f, dx), dx2); },
		[f, dx](ostream& os, const func&) -> ostream& { return print_dif(os, f, dx); }
	}};
}
inline expr make_integral(expr f, expr dx) { return make_integral(f, dx, zero); }
inline expr make_integral(expr f, expr dx, expr c) {
	return func{S_INT, {f, dx, c}, empty,{
		fint,
		[f, dx, c](expr dx2) { return dx2 == dx ? f : make_dif(intf(f, dx, c), dx2); },
		[f, dx, c](expr dx2) { return intf(intf(f, dx, c), dx2); },
		[f, dx](ostream& os, const func&) -> ostream& { return print_int(os, f, dx); }
	}};
}
inline expr make_integral(expr f, expr dx, expr a, expr b) { 
	return func{S_INT,{f, dx, a, b}, empty,{
		fint,
		[f, dx, a, b](expr dx2) { return make_dif(intf(f, dx, a, b), dx2); },
		[f, dx, a, b](expr dx2) { return intf(intf(f, dx, a, b), dx2); },
		[f, dx, a, b](ostream& os, const func&) -> ostream& { return print_int(os, f, dx, a, b); }
	}};
}

inline expr make_assign(expr x, expr y) {
	return func{S_ASSIGN,{x, y}, empty,{
		fass,
		[y](expr dx) { return df(y, dx); },
		[y](expr dx) { return intf(y, dx); },
		[x, y](ostream& os, const func&) -> ostream& { return print_assign(os, x, y); }
	}};
}

inline expr make_subst(expr x, expr y) {
	return func{S_SUBST,{x, y}, empty,{
		fsub,
		[x, y](expr dx) { return df(fsub(xset{x, y}), dx); },
		[x, y](expr dx) { return intf(fsub(xset{x, y}), dx); },
		[x, y](ostream& os, const func&) -> ostream& { return print_subst(os, x, y); }
	}};
}

inline expr func::d(expr dx) const { return _impl.fdif(dx); }
inline expr func::integrate(expr dx, expr c) const { return _impl.fint(dx) + c; }
inline expr func::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (*this)(_args | s); }
inline expr func::approx() const { return (*this)(~_args); }
inline expr func::simplify() const { return (*this)(*_args); }
inline ostream& func::print(ostream& os) const { return _impl.print(os, *this); }
inline unsigned func::exponents(const list_t& vars) const { return 0; }
inline bool func::match(expr e, match_result& res) const {	
	if(!is<func>(e)) return res.found = false;
	auto f = as<func>(e);
	return _name == f.name() ? cas::match(f.args(), _args, res) : res.found = false;
}
inline expr func::operator[](unsigned i) const { return i < _args.size() ? _args[i] : make_err(error_t::invalid_args); }
inline expr func::operator()(expr values) const {
	if(_body != empty)	return _body | pair<expr, expr>{_args, values};
	return _impl.make(is<xset>(values) && as<xset>(values).items().size() == 1 ? as<xset>(values).items()[0] : values);
}

}