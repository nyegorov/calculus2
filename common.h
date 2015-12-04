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
class rational_t;

class numeric;
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

typedef boost::variant<int_t, rational_t, real_t, complex_t> numeric_t;
typedef boost::variant<fn_id, fn_ln, fn_sin, fn_cos, fn_tg, fn_arcsin, fn_arccos, fn_arctg, fn_int, fn_dif, fn_user> function_t;
typedef boost::variant<
	error,
	numeric,
	boost::recursive_wrapper<symbol>, 
	boost::recursive_wrapper<func>, 
	boost::recursive_wrapper<power>, 
	boost::recursive_wrapper<product>, 
	boost::recursive_wrapper<sum>, 
	boost::recursive_wrapper<xset>
	>	expr;

typedef std::vector<expr> vec_expr;
typedef std::vector<expr> list_t;

enum class error_t { cast, invalid_args, not_implemented, syntax, empty };
static const char * error_msgs[] = {"Invalid cast", "Invalid arguments", "Not implemented", "Syntax error", "Empty"};

struct match_result
{
	bool found = true;
	std::vector<symbol> matches;
	expr operator[] (symbol s);
	operator bool() { return found; }
};

bool has_sign(expr e);
bool less(numeric_t op1, numeric_t op2);

template<class T> bool is(const expr& e) { return e.type() == typeid(T); }
template<class T, class F> bool is(const expr& f) { return f.type() == typeid(T) && boost::get<T>(f).value().type() == typeid(F); }
template<class T> T& as(expr& e) { return boost::get<T>(e); }
template<class T> const T& as(const expr& e) { return boost::get<T>(e); }
template<class T, class F> const F as(const expr& f) { return boost::get<F>(boost::get<T>(f).value()); }

std::ostream& operator << (std::ostream& os, const list_t& l);

class rational_t
{
	int_t	_numer = {0};
	int_t	_denom = {1};
public:
	rational_t(int_t numer, int_t denom);
	int_t numer() const { return _numer; }
	int_t denom() const { return _denom; }
	operator real_t() const { return value(); }
	real_t value() const { 
		if(_denom == 0) {
			if(_numer > 0)	return std::numeric_limits<double>::infinity();
			else			return -std::numeric_limits<double>::infinity();
		}
		return (real_t)_numer / _denom; 
	}
};

inline bool operator == (rational_t lh, rational_t rh) { return lh.numer() == rh.numer() && lh.denom() == rh.denom(); }
inline bool operator < (rational_t lh, rational_t rh) { return lh.numer() * rh.denom() < rh.numer() * lh.denom(); }
inline ostream& operator << (ostream& os, const rational_t& r) {
	if(r.denom() == 0)	return os << (r.numer() > 0 ? "inf" : "-inf");
	return os << r.numer() << "/" << r.denom();
}

class error
{
	error_t	_error;
public:
	error() : _error(error_t::empty) {}
	error(error_t err) : _error(err) {}
	error_t get() const { return _error; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr approx() const;
	expr subst(pair<expr, expr> s) const;
	bool match(expr e, match_result& res) const;
};

inline bool operator == (error lh, error rh) { return lh.get() == rh.get(); }
inline bool operator < (error lh, error rh) { return lh.get() < rh.get(); }
inline ostream& operator << (ostream& os, error e) { return os << error_msgs[(int)e.get()]; }

class numeric
{
	numeric_t _value;
public:
	numeric(numeric_t value) : _value(value) {}
	numeric(int_t value) : _value(value) {}
	numeric(real_t value) : _value(value) {}
	numeric(rational_t value) : _value(value) {}
	numeric(int_t numer, int_t denom) : _value(rational_t{numer, denom}) {}
	numeric(complex_t value) : _value(value) {}

	numeric_t value() const { return _value; }
	bool has_sign() const { return less(_value, numeric_t{0}); }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};
inline bool operator == (numeric lh, numeric rh) { return lh.value() == rh.value(); }
inline bool operator < (numeric lh, numeric rh) { return less(lh.value(), rh.value()); }
inline ostream& operator << (ostream& os, numeric n) {	return os << n.value(); }

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

inline bool operator == (symbol lh, symbol rh) { return lh.name() == rh.name(); }
inline bool operator < (symbol lh, symbol rh) { return lh.name() < rh.name(); }
inline ostream& operator << (ostream& os, symbol s) { return os << s.name(); }

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

inline bool operator == (power lh, power rh) { return lh.x() == rh.x() && lh.y() == rh.y(); }
inline bool operator < (power lh, power rh) { return lh.x() == rh.x() ? lh.y() < rh.y() : lh.x() < rh.x(); }
inline ostream& operator << (ostream& os, power s) {
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

inline bool operator == (product lh, product rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
inline bool operator < (product lh, product rh) { return lh.right() < rh.right(); }
inline ostream& operator << (ostream& os, product p) {
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

inline bool operator == (sum lh, sum rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
inline bool operator < (sum lh, sum rh) { return lh.right() < rh.right(); }
inline ostream& operator << (ostream& os, sum s) {
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
	expr operator[] (unsigned i) const { 
		if(is<xset>(_x))	return i < as<xset>(_x).items().size() ? as<xset>(_x).items()[i] : make_err(error_t::invalid_args);
		else				return i == 0 ? _x : make_err(error_t::invalid_args);
	}
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
struct fn_user : public fn_base<fn_user> { 
	fn_user(expr x) : fn_base(x) {}
	fn_user(string name, expr body, list_t args);
	string name() const;
	expr body() const;
	list_t args() const;

	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr call(expr params) const;
	expr operator ()() const;
	template <typename ... Params> expr operator ()(expr val, Params ... rest) const;
};
typedef fn_user	fun;

class func
{
	function_t	_func;
public:
	func(function_t func) : _func(func) {}
	function_t value() const { return _func; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	bool match(expr e, match_result& res) const;
};

inline bool operator == (func lh, func rh) { return lh.value() == rh.value(); }
inline bool operator < (func lh, func rh) { return lh.value() < rh.value(); }
inline ostream& operator << (ostream& os, func f) { return os << f.value(); }

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

inline bool operator == (xset lh, xset rh) { return lh.items() == rh.items(); }
inline bool operator < (xset lh, xset rh) { return lh.items() < rh.items(); }
inline ostream& operator << (ostream& os, xset l) { return os << '[' << l.items() << ']'; }

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

inline bool failed(expr e) { return e.type() == typeid(error); }
inline string to_string(expr e) { std::stringstream ss; ss << e; return ss.str(); }
inline bool has_sign(expr e) { return boost::apply_visitor([](auto x) { return x.has_sign(); }, e); }
inline expr subst(expr e, pair<expr, expr> s) { return boost::apply_visitor([s](auto x) { return x.subst(s); }, e); }
inline expr subst(expr e, expr from, expr to) { return subst(e, {from, to}); }
inline expr subst(expr e, symbol var) { return subst(e, var, var.value()); }
inline expr df(expr e, expr dx) { return boost::apply_visitor([dx](auto x) { return x.d(dx); }, e); }
inline expr intf(expr e, expr dx, expr c = expr{0}) { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, e); }
inline expr intf(expr e, expr dx, expr a, expr b) { auto F = intf(e, dx); return subst(F, dx, b) - subst(F, dx, a); }
inline expr approx(expr e) { return boost::apply_visitor([](auto x) { return x.approx(); }, e); }
inline bool match(expr e, expr pattern, match_result& res) { return boost::apply_visitor([e, &res](auto x) { return x.match(e, res); }, pattern); }
inline match_result match(expr e, expr pattern) { match_result res; match(e, pattern, res); return res; }

expr make_err(error_t err);
expr make_num(int_t value);
expr make_num(int_t numer, int_t denom);
expr make_num(real_t value);
expr make_num(real_t real, real_t imag);
expr make_num(complex_t value);
expr make_power(expr x, expr y);
expr make_sum(expr x, expr y);
expr make_prod(expr left, expr right);
expr make_integral(expr f, expr dx);

inline expr error::subst(pair<expr, expr> s) const { return *this; };
inline expr error::d(expr dx) const { return *this; };
inline expr error::integrate(expr dx, expr c) const { return *this; };
inline expr error::approx() const { return *this; };
inline bool error::match(expr e, match_result& res) const { if(e != expr{*this}) res.found = false; return res; };
inline expr match_result::operator[] (symbol s) { auto it = std::find(matches.begin(), matches.end(), s); return it == matches.end() ? empty : it->value(); }

const expr e = symbol{"#e", numeric{boost::math::constants::e<double>()}};
const expr pi = symbol{"#p", numeric{boost::math::constants::pi<double>()}};

inline std::ostream& operator << (std::ostream& os, const list_t& l) {
	for(auto it = l.cbegin(); it != l.cend(); ++it) {
		if(it != l.cbegin())	os << ',';
		os << *it;
	}
	return os;
}

}

namespace std {
	using namespace cas;
	inline ostream& operator << (ostream& os, complex_t c) {
		if(abs(c.real()) >= std::numeric_limits<real_t>::epsilon())	os << c.real() << (c.imag() > 0 ? '+' : '-');
		else if(c.imag() < 0)										os << '-';
		if(abs(c.imag()) != 1.)	os << abs(c.imag());
		return os << 'i';
	}
}

