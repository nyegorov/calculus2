#pragma once
#include <boost/variant.hpp>
#include <boost/math/constants/constants.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include <unordered_set>
#include "expr_list.h"

using std::string;
using std::ostream;
using std::pair;

namespace cas {
class integer;
class rational;
class real;
class complex;
class symbol;
class func;
class power;
class product;
class sum;
class xset;
class error;

struct fn_id;
struct fn_ln;
struct fn_sin;
struct fn_cos;
struct fn_tg;
struct fn_arcsin;
struct fn_arccos;
struct fn_arctg;
struct fn_int;
struct fn_dif;
struct fn_user;

typedef int int_t;
typedef double real_t;
typedef std::complex<real_t> complex_t;
typedef boost::variant<
	integer, 
	rational, 
	real, 
	complex, 
	boost::recursive_wrapper<symbol>, 
	boost::recursive_wrapper<func>, 
	boost::recursive_wrapper<power>, 
	boost::recursive_wrapper<product>, 
	boost::recursive_wrapper<sum>, 
	boost::recursive_wrapper<xset>,
	error>	expr;

typedef boost::variant<fn_id, fn_ln, fn_sin, fn_cos, fn_tg, fn_arcsin, fn_arccos, fn_arctg, fn_int, fn_dif, fn_user> function_t;
typedef std::vector<expr> vec_expr;
typedef std::vector<expr> list_t;
const unsigned numbers = 3;

enum class error_t { cast, invalid_args, not_implemented, syntax, empty };
const char * error_msgs[] = {"Invalid cast", "Invalid arguments", "Not implemented", "Syntax error", "Empty"};

struct match_result
{
	bool found = true;
	std::vector<symbol> matches;
	expr operator[] (symbol s);
	operator bool() { return found; }
};

bool has_sign(expr e);
bool is_numeric(expr e);
template<class T> bool is(const expr& e) { return e.type() == typeid(T); }
template<class T> T& as(expr& e) { return boost::get<T>(e); }
template<class T> const T& as(const expr& e) { return boost::get<T>(e); }

template<class T> class base
{
public:
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr approx() const;
	expr subst(pair<expr, expr> s) const;
	bool match(expr e, match_result& res) const;
};

class error : public base<error>
{
	error_t	_error;
public:
	error(error_t err) : _error(err) {}
	error_t get() const { return _error; }
	expr d(expr dx) const;
};

bool operator == (error lh, error rh) { return lh.get() == rh.get(); }
bool operator < (error lh, error rh) { return lh.get() < rh.get(); }
ostream& operator << (ostream& os, error e) { return os << error_msgs[(int)e.get()]; }

class integer : public base<integer>
{
	int_t	_value;
public:
	integer() : _value(0) {}
	integer(int_t value) : _value(value) {}
	int_t value() const { return _value; }
	bool has_sign() const { return _value < 0; }
	expr approx() const;
};
bool operator == (integer lh, integer rh) { return lh.value() == rh.value(); }
bool operator < (integer lh, integer rh) { return lh.value() < rh.value(); }
ostream& operator << (ostream& os, integer n) { return os << n.value(); }

class rational : public base<rational>
{
	int_t	_numer = {0};
	int_t	_denom = {1};
public:
	rational(int_t numer, int_t denom) : _numer(numer), _denom(denom) {}
	int_t numer() const { return _numer; }
	int_t denom() const { return _denom; }
	bool has_sign() const { return _numer * _denom < 0; }
	expr approx() const;
};

bool operator == (rational lh, rational rh) { return lh.numer() == rh.numer() && lh.denom() == rh.denom(); }
bool operator < (rational lh, rational rh) { return lh.numer() * rh.denom() < rh.numer() * lh.denom(); }
ostream& operator << (ostream& os, const rational& r) { 
	if(r.denom() == 0)	return os << (r.numer() > 0 ? "inf" : "-inf");
	return os << r.numer() << "/" << r.denom(); 
}

class real : public base<real>
{
	real_t	_value;
public:
	real(real_t value, real_t unused) : _value(value) {}
	real_t value() const { return _value; }
	bool has_sign() const { return _value < 0; }
	expr approx() const;
};
bool operator == (real lh, real rh) { return fabs(lh.value() - rh.value()) < std::numeric_limits<real_t>::epsilon() * std::max({1.0, fabs(lh.value()), fabs(rh.value())}); }
bool operator < (real lh, real rh) { return lh.value() < rh.value(); }
ostream& operator << (ostream& os, real n) { return os << n.value(); }

class complex : public base<complex>
{
	complex_t _value;
public:
	complex(complex_t value) : _value(value) {}
	std::complex<real_t> value() const { return _value; }
	bool has_sign() const { return _value.real() < 0; }
	expr approx() const;
};
bool operator == (complex lh, complex rh) { return lh.value() == rh.value(); }
bool operator < (complex lh, complex rh) { return abs(lh.value()) < abs(rh.value()); }
ostream& operator << (ostream& os, complex c) {
	if(abs(c.value().real()) >= std::numeric_limits<real_t>::epsilon())	os << c.value().real() << (c.value().imag() > 0 ? '+' : '-');
	else if(c.value().imag() < 0)										os << '-';
	if(abs(c.value().imag()) != 1.)	os << abs(c.value().imag());
	return os << 'i';
}

class symbol
{
	string	_name;
	expr	_value;
public:
	symbol(string name) : _name(name), _value(error_t::empty) {}
	symbol(string name, expr value) : _name(name), _value(value) {}
	string name() const { return _name; }
	expr value() const { return _value; }
	symbol operator = (expr value) { _value = value; return *this; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};

bool operator == (symbol lh, symbol rh) { return lh.name() == rh.name(); }
bool operator < (symbol lh, symbol rh) { return lh.name() < rh.name(); }
ostream& operator << (ostream& os, symbol s) { return os << s.name(); }

class power
{
	expr _x;
	expr _y;
public:
	power(expr x, expr y) : _x(x), _y(y) {}
	expr x() const { return _x; }
	expr y() const { return _y; }
	bool has_sign() const { return cas::has_sign(_x); }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};

bool operator == (power lh, power rh) { return lh.x() == rh.x() && lh.y() == rh.y(); }
bool operator < (power lh, power rh) { return lh.x() == rh.x() ? lh.y() < rh.y() : lh.x() < rh.x(); }
ostream& operator << (ostream& os, power s) {
	if(is<sum>(s.x()) || is<product>(s.x())) os << '(' << s.x() << ')'; else os << s.x();
	os << '^';
	if(is<sum>(s.y()) || is<product>(s.y())) os << '(' << s.y() << ')'; else os << s.y();
	return os;
}

class product : public detail::expr_list<product, expr>
{
public:
	static expr unit() { return{1}; }
	static expr op(const expr& lh, const expr& rh);

	product(expr left, expr right) : expr_list(left, right) {}
	bool has_sign() const { return cas::has_sign(_left); }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
};

bool operator == (product lh, product rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
bool operator < (product lh, product rh) { return lh.right() < rh.right(); }
ostream& operator << (ostream& os, product p) {
	if(p.left() == expr{-1})	os << '-'; else os << p.left();
	return os << p.right();
}

class sum : public detail::expr_list<sum, expr>
{
public:
	static expr unit() { return{0}; }
	static expr op(const expr& lh, const expr& rh);

	sum(expr left, expr right) : expr_list(left, right) {}
	bool has_sign() const { return cas::has_sign(_left); }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
};

bool operator == (sum lh, sum rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
bool operator < (sum lh, sum rh) { return lh.right() < rh.right(); }
ostream& operator << (ostream& os, sum s) {
	os << s.left();
	if(!cas::has_sign(s.right()))	os << '+';
	return os << s.right();
}

template<class F> class fn_base
{
	expr _x;
public:
	fn_base(expr x) : _x(x) {}
	expr x() const { return _x; }
	static expr make(expr x) { return x; };
	expr param(unsigned i) const {
		if(is<xset>(_x))	return i < as<xset>(_x).items().size() ? as<xset>(_x).items()[i] : make_err(error_t::invalid_args);
		else				return i == 0 ? _x : make_err(error_t::invalid_args);
	}
	size_t params_count() const { return is<xset>(_x) ? as<xset>(_x).items().size() : 1; }
	expr operator[] (unsigned i) const { return param(i); }
	size_t size() const { return is<xset>(_x) ? as<xset>(_x).items().size() : 1; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(function_t f, match_result& res) const;
};
template<class F> bool operator == (fn_base<F> lh, fn_base<F> rh) { return lh.x() == rh.x(); }
template<class F> bool operator < (fn_base<F> lh, fn_base<F> rh) { return lh.x() < rh.x(); }
template<class F> ostream& operator << (ostream& os, fn_base<F> f) { return os << f.x(); }

struct fn_id : public fn_base<fn_id> { using fn_base::fn_base; };
struct fn_ln : public fn_base<fn_ln> { using fn_base::fn_base; };
struct fn_sin : public fn_base<fn_sin> { using fn_base::fn_base; };
struct fn_cos : public fn_base<fn_cos> { using fn_base::fn_base; };
struct fn_tg : public fn_base<fn_tg> { using fn_base::fn_base; };
struct fn_arcsin : public fn_base<fn_arcsin> { using fn_base::fn_base; };
struct fn_arccos : public fn_base<fn_arccos> { using fn_base::fn_base; };
struct fn_arctg : public fn_base<fn_arctg> { using fn_base::fn_base; };
struct fn_int : public fn_base<fn_int> { using fn_base::fn_base; };
struct fn_dif : public fn_base<fn_dif> { using fn_base::fn_base; };
struct fn_user : public fn_base<fn_user> { using fn_base::fn_base; };

ostream& operator << (ostream& os, fn_base<fn_ln> f) { return os << "ln(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_sin> f) { return os << "sin(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_cos> f) { return os << "cos(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_tg> f) { return os << "tg(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arcsin> f) { return os << "arcsin(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arccos> f) { return os << "arccos(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_arctg> f) { return os << "arctg(" << f.x() << ')'; }
ostream& operator << (ostream& os, fn_base<fn_int> f) { return os << "int(" << f.param(0) << ',' << f.param(1) << ')'; }
ostream& operator << (ostream& os, fn_base<fn_dif> f) { return os << "d/d" << f[1] << " " << f[0]; }
ostream& operator << (ostream& os, fn_base<fn_user> f) {
	return os << f[0] << '(';
	for(size_t i = 2; i < f.size(); i++) {
		if(i != 2)	os << ',';
		os << f[i];
	}
	return os << ')';
}

class func
{
	function_t	_func;
public:
	func(function_t func) : _func(func) {}
	function_t f() const { return _func; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};

bool operator == (func lh, func rh) { return lh.f() == rh.f(); }
bool operator < (func lh, func rh) { return lh.f() < rh.f(); }
ostream& operator << (ostream& os, func f) { return os << f.f(); }

class xset
{
	list_t	_items;
public:
	xset(expr item) { _items.push_back(item); }
	xset(list_t items) : _items(items) {}
	xset(std::initializer_list<expr> items) : _items(items) {}
	const list_t& items() const { return _items; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};

bool operator == (xset lh, xset rh) { return lh.items() == rh.items(); }
bool operator < (xset lh, xset rh) { return lh.items() < rh.items(); }
ostream& operator << (ostream& os, xset l) {
	os << '['; 
	for(auto it = l.items().cbegin(); it != l.items().cend(); ++it) {
		if(it != l.items().cbegin())	os << ',';
		os << *it;
	}
	return os << ']';
}

const expr empty = error{error_t::empty};

expr operator + (expr op1, expr op2);
expr operator * (expr op1, expr op2);
expr operator ^ (expr op1, expr op2);
expr operator - (expr op1, expr op2);
expr operator - (expr op1);
expr operator ~ (expr op1);
expr operator / (expr op1, expr op2);
expr operator | (expr op1, symbol op2);
expr operator | (expr op1, pair<expr, expr> op2);

bool failed(expr e) { return e.type() == typeid(error); }
string to_string(expr e) { std::stringstream ss; ss << e; return ss.str(); }
bool has_sign(expr e) { return boost::apply_visitor([](auto x) { return x.has_sign(); }, e); }
bool is_numeric(expr e) { return e.which() <= numbers; }
expr subst(expr e, pair<expr, expr> s) { return boost::apply_visitor([s](auto x) { return x.subst(s); }, e); }
expr subst(expr e, expr from, expr to) { return subst(e, {from, to}); }
expr subst(expr e, symbol var) { return subst(e, var, var.value()); }
expr df(expr e, expr dx) { return boost::apply_visitor([dx](auto x) { return x.d(dx); }, e); }
expr intf(expr e, expr dx, expr c = expr{0}) { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, e); }
expr intf(expr e, expr dx, expr a, expr b) { auto F = intf(e, dx); return subst(F, dx, b) - subst(F, dx, a); }
expr approx(expr e) { return boost::apply_visitor([](auto x) { return x.approx(); }, e); }
bool match(expr e, expr pattern, match_result& res) { return boost::apply_visitor([e, &res](auto x) { return x.match(e, res); }, pattern); }
match_result match(expr e, expr pattern) { match_result res; match(e, pattern, res); return res; }

expr make_err(error_t err);
expr make_int(int_t value);
expr make_rat(int_t numer, int_t denom);
expr make_real(real_t value);
expr make_complex(complex_t value);
expr make_power(expr x, expr y);
expr make_sum(expr x, expr y);
expr make_prod(expr left, expr right);
expr make_integral(expr f, expr dx);

template<class T> expr base<T>::subst(pair<expr, expr> s) const { return {*static_cast<const T*>(this)}; };
template<class T> expr base<T>::d(expr dx) const { return 0; };
template<class T> expr base<T>::integrate(expr dx, expr c) const { return dx + c; };
template<class T> expr base<T>::approx() const { return {*static_cast<const T*>(this)}; };
template<class T> bool base<T>::match(expr e, match_result& res) const { if(e != expr{*static_cast<const T*>(this)}) res.found = false; return res; };
expr match_result::operator[] (symbol s) { auto it = std::find(matches.begin(), matches.end(), s); return it == matches.end() ? empty : it->value(); }

const expr e = symbol{"#e", make_real(boost::math::constants::e<double>())};
const expr pi = symbol{"#p", make_real(boost::math::constants::pi<double>())};

}

