#pragma once

#include "common.h"
#include "numeric.h"

namespace cas {

ostream& operator << (ostream& os, fn_base<fn_ln> f) { return os << "ln(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_sin> f) { return os << "sin(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_cos> f) { return os << "cos(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_tg> f) { return os << "tg(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arcsin> f) { return os << "arcsin(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arccos> f) { return os << "arccos(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arctg> f) { return os << "arctg(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_int> f) { return os << "int(" << f[0] << ',' << f[1] << ')'; }
ostream& operator << (ostream& os, fn_base<fn_dif> f) { return os << "d/d" << f[1] << " " << f[0]; }
ostream& operator << (ostream& os, fn_user f) {
	return os << f.name() << '(' << f.args() << ')';
}

expr ln(expr x);

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
	if(is<func, fn_arcsin>(x))		return as<func, fn_arcsin>(x).x();									// sin(arcsin(x)) => x
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
	if(is<func, fn_arccos>(x))		return as<func, fn_arccos>(x).x();									// cos(arccos(x)) => x
	return func{fn_cos{x}};
}

template<> static expr fn_base<fn_tg>::make(expr x) {
	if(is<product>(x) && as<product>(x).left() == minus_one)return -1 * make(as<product>(x).right());	// tg(-x) => -tg(x)
	if(is<func, fn_arctg>(x))		return as<func, fn_arctg>(x).x();									// tg(arctg(x)) => x
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

template<> static expr fn_base<fn_int>::make(expr p) {
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	expr f = as<xset>(p).items().front(), dx = as<xset>(p).items().back();
	if(df(f, dx) == zero)	return f * dx;												// ∫ y dx => yx
	match_result mr;
	symbol x{"x", dx}, y{"y"};
	//if((mr = cas::match(f, x*ln(x))))	return (dx ^ 2)*ln(x) / 2 - (dx ^ 2) / 4;		// ∫ x∙ln(x) dx => (x²∙lnx)/2 - x²/4
	if((mr = cas::match(f, ln(x) / x)))	return half * (ln(dx) ^ 2);						// ∫ ln(x)/x dx => 1/2∙ln²(x)
	if(mr = cas::match(f, x*ln(y))) {													// ∫ x∙ln(ax+b) dx => (a²x²-b²)∙ln(ax+b)/2a²-x∙(ax-2b)/4a
		auto a = df(mr[y], dx), b = mr[y] - a*x;
		if(df(a, dx) == zero)	return (((a*x) ^ 2) - (b ^ 2))*ln(a*x + b) / (2 * (a ^ 2)) - x*(a*x - 2 * b) / (4 * a);
	}
	return func{fn_int{xset{f, dx}}};
}

template<> static expr fn_base<fn_dif>::make(expr p) {
	if(!is<xset>(p) || as<xset>(p).items().size() != 2)	return make_err(error_t::syntax);
	return func{fn_dif{p}};
}

template<> static expr fn_base<fn_user>::make(expr x) {	return func{fn_user{x}}; };

// User-defined functions
fn_user::fn_user(string name, expr body, list_t args) : fn_base(xset{name, body, args}) {}
string fn_user::name() const { return as<symbol>((*this)[0]).name(); }
expr fn_user::body() const { return (*this)[1]; }
list_t fn_user::args() const { return as<xset>((*this)[2]).items(); }
expr fn_user::operator ()() const { return func{*this}; }
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

expr apply_fun(expr x, real_t rfun(real_t x), complex_t cfun(complex_t x))
{
	if(is<numeric, int_t>(x))		return make_num(rfun((real_t)as<numeric, int_t>(x)));
	if(is<numeric, real_t>(x))		return make_num(rfun(as<numeric, real_t>(x)));
	if(is<numeric, complex_t>(x))	return make_num(cfun(as<numeric, complex_t>(x)));
	return x;
}
template<class F> expr fn_base<F>::approx() const { return fn_base<F>::make(~_x); }
template<class F> expr fn_base<F>::subst(pair<expr, expr> s) const { return expr{func{F{_x}}} == s.first ? s.second : make(_x | s); }
template<class F> bool fn_base<F>::match(function_t f, match_result& res) const { return f.type() == typeid(F) ? cas::match(boost::get<F>(f).x(), _x, res) : res.found = false; }
template<class F> expr operator *(fn_base<F> f) { return f.make(f.x()); }

template<> expr fn_base<fn_ln>::approx() const { return apply_fun(~_x, log, [](complex_t x) {return log(x); }); }
template<> expr fn_base<fn_sin>::approx() const { return apply_fun(~_x, std::sin, [](complex_t x) {return sin(x); }); }
template<> expr fn_base<fn_cos>::approx() const { return apply_fun(~_x, std::cos, [](complex_t x) {return cos(x); }); }
template<> expr fn_base<fn_tg>::approx() const { return apply_fun(~_x, tan, [](complex_t x) {return tan(x); }); }
template<> expr fn_base<fn_arcsin>::approx() const { return apply_fun(~_x, asin, [](complex_t x) {return asin(x); }); }
template<> expr fn_base<fn_arccos>::approx() const { return apply_fun(~_x, acos, [](complex_t x) {return acos(x); }); }
template<> expr fn_base<fn_arctg>::approx() const { return apply_fun(~_x, atan, [](complex_t x) {return atan(x); }); }
template<> expr fn_base<fn_int>::approx() const { return make_integral(~(*this)[0], (*this)[1]); }

expr operator * (func f) { return boost::apply_visitor([](auto f) { return *f; }, f.value()); }						 

expr func::d(expr dx) const { return boost::apply_visitor([dx](auto x) { return x.d(dx); }, _func); }
expr func::integrate(expr dx, expr c) const { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, _func); }
expr func::subst(pair<expr, expr> s) const { return boost::apply_visitor([s](auto x) { return x.subst(s); }, _func);}
expr func::approx() const { return boost::apply_visitor([](auto x) { return x.approx(); }, _func); }
bool func::match(expr e, match_result& res) const { 
	return is<func>(e) ? 
		boost::apply_visitor([fun = as<func>(e).value(), &res](auto f) { return f.match(fun, res); }, _func) :
		res.found = false;
}

expr ln(expr x) { return fn_ln::make(x); }
expr sin(expr x) { return fn_sin::make(x); }
expr cos(expr x) { return fn_cos::make(x); }
expr tg(expr x) { return fn_tg::make(x); }
expr arcsin(expr x) { return fn_arcsin::make(x); }
expr arccos(expr x) { return fn_arccos::make(x); }
expr arctg(expr x) { return fn_arctg::make(x); }
expr fn(string name, expr body, list_t args) { return func{fn_user{name, body, args}}; }

}