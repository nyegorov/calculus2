#pragma once

#include "common.h"
#include "numeric.h"

namespace cas {

inline bool is_func(const expr& x, const char name[]) { return is<func>(x) && as<func>(x).name() == name; }

expr fn(string name, list_t args);
expr fn(string name, list_t args, expr body);
expr ln(expr x);
expr sin(expr x);
expr cos(expr x);
expr tg(expr x);
expr arcsin(expr x);
expr arccos(expr x);
expr arctg(expr x);

inline func::func(string name, list_t args) : _name(name), _args(args), _impl{
	[name](expr x) { return fn(name, is<xset>(x) ? as<xset>(x).items() : list_t{x}); },
	[args](expr f, expr dx) {
		auto it = std::find_if(args.begin(), args.end(), [dx](auto x) {return df(x, dx) != zero; });
		return it == args.end() ? zero : df(*it, dx) * make_dif(f, dx);
	},
	[](expr f, expr dx) { return df(f, dx) == zero ? f * dx : make_int(f, dx); }
} {}

inline func::func(string name, list_t args, expr body) : _name(name), _args(args), _impl{
	[body, a = args.size() == 1 ? args.front() : args](expr x) { return ::subst(body, a, x); },
	[body](expr f, expr dx) { return df(body, dx);   },
	[body](expr f, expr dx) { return intf(body, dx); }
} {}

inline expr fn(string name, list_t args)			{ return func{name, args}; }
inline expr fn(string name, list_t args, expr body) { return func{name, args, body}; }

inline expr approx_fun(expr f, expr x) { return as<func>(f)(x); }
inline auto apply_fun(real_t rfun(real_t x), complex_t cfun(const complex_t& x)) {
	return [=](expr f, expr x) {
		if(is<xset>(x))	x = as<xset>(x).items().size() == 1 ? as<xset>(x).items()[0] : make_err(error_t::syntax);
		if(is<numeric, int_t>(x))		return make_num(rfun((real_t)as<numeric, int_t>(x)));
		if(is<numeric, real_t>(x))		return make_num(rfun(as<numeric, real_t>(x)));
		if(is<numeric, complex_t>(x))	return make_num(cfun(as<numeric, complex_t>(x)));
		return as<func>(f)(x);
	};
}

inline expr ln(expr x)
{
	if(x == zero)		return minf;																// ln(0) ⇒ -∞
	if(x == one)		return zero;																// ln(1) ⇒ 0
	if(x == e)			return one;																	// ln(e) ⇒ 1
	if(x == inf)		return inf;																	// ln(∞) ⇒ ∞
	if(is<product>(x))	return ln(as<product>(x).left()) + ln(as<product>(x).right());				// ln(x∙y) ⇒ ln(x)+ln(y)
	if(is<power>(x))	return as<power>(x).y() * ln(as<power>(x).x());								// ln(xʸ) ⇒ y∙ln(x)
	return func{S_LN, {x}, {
		ln,
		[x](expr f, expr dx) { return df(x, dx) / x; },												// ln(f)' ⇒ f'/x
		[x](expr f, expr dx) { auto a = df(x, dx); return a != zero && (df(a, dx) == zero) ? x / a * ln(x) - dx : make_int(f, dx); },
		apply_fun(std::log, std::log)
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
	if(is<product>(x) && has_sign(as<product>(x).left())) return -sin(-x);							// sin(-x) ⇒ -sin(x)
	if(is<func>(x) && as<func>(x).name() == S_ASIN)	return as<func>(x).x();							// sin(arcsin(x)) ⇒ x
	return func{S_SIN, {x}, {
		sin, 
		[x](expr f, expr dx) { return df(x, dx) * cos(x); },										// sin(f)' ⇒ f'∙cos(x)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? -cos(x) : make_int(f, dx); },
		apply_fun(std::sin, std::sin)
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
	if(is<func>(x) && as<func>(x).name() == S_ACOS)	return as<func>(x).x();							// cos(arccos(x)) ⇒ x
	return func{S_COS, {x}, {
		cos,
		[x](expr f, expr dx) { return df(x, dx) * -sin(x); },										// cos(f)' ⇒ -f'∙sin(x)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? sin(x) : make_int(f, dx); },
		apply_fun(std::cos, std::cos)
	}};
}

inline expr tg(expr x) {
	if(is<product>(x) && as<product>(x).left() == minus_one)return -tg(as<product>(x).right());		// tg(-x) ⇒ -tg(x)
	if(is<func>(x) && as<func>(x).name() == S_ATG)		return as<func>(x).x();						// tg(arctg(x)) ⇒ x
	return func{S_TG, {x}, {
		tg,
		[x](expr f, expr dx) { return df(x, dx) / (cos(x) ^ two); },								// tg(f)' ⇒ f'/cos²(x)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? -ln(cos(x)) : make_int(f, dx); },
		apply_fun(std::tan, std::tan)
	}};
}
inline expr arcsin(expr x)	{
	return func{S_ASIN,{x}, {
		arcsin,
		[x](expr f, expr dx) { return df(x, dx) / ((1 - (x^2)) ^ half); },							// arcsin(f)' ⇒ f'/√(1-x²)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * f + ((1-(x^2))^half) : make_int(f, dx); },
		apply_fun(std::asin, std::asin)
	}};
}
inline expr arccos(expr x)	{
	return func{S_ACOS, {x}, {
		arccos,
		[x](expr f, expr dx) { return -df(x, dx) / ((1 - (x^2)) ^ half); },							// arccos(f)' ⇒ -f'/√(1-x²)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * f - ((1-(x^2))^half) : make_int(f, dx); },
		apply_fun(std::acos, std::acos)
	}};
}
inline expr arctg(expr x)	{
	return func{S_ATG, {x}, {
		arctg,
		[x](expr f, expr dx) { return df(x, dx) / ((1 + (x^2)) ^ half); },							// arctg(f)' ⇒ f'/√(1+x²)
		[x](expr f, expr dx) { return df(x, dx) == zero ? x * dx : x == dx ? x * f - half * ln(1+(x^2)) : make_int(f, dx); },
		apply_fun(std::atan, std::atan)
	}};
}

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

inline expr make_dif(expr f, expr dx) {
	return func{S_DIF, {f, dx}, {
		fdif,
		make_dif,
		[f, dx](expr f2, expr dx2) { return dx2 == dx ? f : make_int(f2, dx2); },
		approx_fun,
		print_dif
	}};
}
inline expr make_int(expr f, expr dx) { 
	return func{S_INT,{f, dx},{
		fint,
		[f, dx](expr f2, expr dx2) { return dx2 == dx ? f : make_dif(f2, dx2); },
		make_int,
		approx_fun,
		print_int
	}};
}
inline expr make_integral(expr f, expr dx, expr a, expr b) { return func{S_INT,{f, dx, a, b}, { fint, make_dif, make_int, approx_fun, print_int }}; }
inline expr make_assign(expr x, expr y) { return func{S_ASSIGN, {x, y}, { fass, make_dif, make_int, approx_fun, print_assign }}; }
inline expr make_subst(expr x, expr y)  { return func{S_SUBST,  {x, y}, { fsub, make_dif, make_int, approx_fun, print_subst }}; }

inline expr func::x() const { return _args.size() == 1 ? _args.front() : xset(_args); }
inline expr func::d(expr dx) const { return _impl.d(*this, dx); }
inline expr func::integrate(expr dx, expr c) const { return _impl.integrate(*this, dx) + c; }
inline expr func::subst(pair<expr, expr> s) const { 
	if(is<symbol>(s.first) && is<func>(s.second)) {
		auto& f = as<func>(s.second);
		if(as<symbol>(s.first).name() == name() && f.name() == name() && f.args().size() == args().size())	return f(_args);
	}
	return expr{*this} == s.first ? s.second : _impl.make(x() | s);
}
inline expr func::approx() const { return _impl.approx(*this, ~_args); }
inline expr func::simplify() const { return _impl.make(*x()); }
inline ostream& func::print(ostream& os) const { return _impl.print(os, *this); }
inline unsigned func::exponents(const list_t& vars) const { return 0; }
inline bool func::match(expr e, match_result& res) const {	
	if(!is<func>(e)) return res.found = false;
	auto f = as<func>(e);
	return _name == f.name() ? cas::match(f.args(), _args, res) : res.found = false;
}
inline expr func::operator()(expr values) const {
	return func{_name, is<xset>(values) ? as<xset>(values).items() : list_t{values}, _impl};
}

}