#pragma once
#include "expr_list.h"

using std::string;
using std::ostream;
using std::pair;

const char S_LN[] = "ln";
const char S_SIN[] = "sin";
const char S_COS[] = "cos";
const char S_TG[] = "tg";
const char S_ASIN[] = "arcsin";
const char S_ACOS[] = "arccos";
const char S_ATG[] = "arctg";
const char S_DIF[] = "dif";
const char S_INT[] = "int";
const char S_ASSIGN[] = "assign";
const char S_SUBST[] = "subst";

namespace cas {
class rational_t;

class numeric;
class symbol;
class func;
class func;
class power;
class product;
class sum;
class xset;
class error;

typedef int int_t;
typedef double real_t;
typedef std::complex<real_t> complex_t;

typedef boost::variant<int_t, rational_t, real_t, complex_t> numeric_t;
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

typedef std::vector<expr> list_t;
typedef std::function<expr(expr)> fmake_t;
typedef std::function<expr(expr, expr)> fcall_t;
typedef std::function<ostream&(ostream& os, const func&)> fprint_t;

enum class part_t { all = 0, num = 1, den = 2 };
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
unsigned get_exps(expr e, const list_t& vars);
bool is_mml(ostream& os);
expr make_err(error_t err);
expr make_num(int_t value);
expr make_num(int_t numer, int_t denom);
expr make_num(real_t value);
expr make_num(real_t real, real_t imag);
expr make_num(complex_t value);
expr make_power(expr x, expr y);
expr make_sum(expr x, expr y);
expr make_prod(expr left, expr right);
expr make_dif(expr f, expr dx);
expr make_int(expr f, expr dx);
expr make_intd(expr f, expr dx, expr a, expr b);

template<class T> bool is(const expr& e) { return e.type() == typeid(T); }
template<class T, class F> bool is(const expr& f) { return f.type() == typeid(T) && boost::get<T>(f).value().type() == typeid(F); }
template<class T> T& as(expr& e) { return boost::get<T>(e); }
template<class T> const T& as(const expr& e) { return boost::get<T>(e); }
template<class T, class F> const F as(const expr& f) { return boost::get<F>(boost::get<T>(f).value()); }
bool is_func(const expr& x, const char name[]);

std::ostream& operator << (std::ostream& os, const list_t& l);
std::ostream& operator << (std::ostream& os, part_t part);

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
	expr simplify() const;
	expr subst(pair<expr, expr> s) const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (error lh, error rh) { return lh.get() == rh.get(); }
inline bool operator < (error lh, error rh) { return lh.get() < rh.get(); }

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
	expr simplify() const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};
inline bool operator == (numeric lh, numeric rh) { return lh.value() == rh.value(); }
inline bool operator < (numeric lh, numeric rh) { return less(lh.value(), rh.value()); }

class symbol
{
	string	_name;
	expr	_value;
public:
	explicit symbol(string name) : _name(name), _value(error_t::empty) {}
	symbol(string name, expr value) : _name(name), _value(value) {}
	string name() const { return _name; }
	expr value() const { return _value; }
	symbol operator = (expr value) { _value = value; return *this; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	expr simplify() const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (symbol lh, symbol rh) { return lh.name() == rh.name(); }
inline bool operator < (symbol lh, symbol rh) { return lh.name() < rh.name(); }

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
	expr simplify() const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (power lh, power rh) { return lh.x() == rh.x() && lh.y() == rh.y(); }
inline bool operator < (power lh, power rh) { return lh.y() == rh.y() ? lh.x() < rh.x() : lh.y() < rh.y(); }
inline ostream& operator << (ostream& os, power p);

struct prod_comp { bool operator ()(const expr& left, const expr& right) const; };
class product : public detail::expr_list<product, expr, prod_comp>
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
	expr simplify() const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (product lh, product rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
inline bool operator < (product lh, product rh) { return lh.right() == rh.right() ? lh.left() < rh.left() : lh.right() < rh.right(); }
inline ostream& operator << (ostream& os, product p);

struct sum_comp { bool operator ()(const expr& left, const expr& right) const; };
class sum : public detail::expr_list<sum, expr, sum_comp>
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
	expr simplify() const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (sum lh, sum rh) { return lh.left() == rh.left() && lh.right() == rh.right(); }
inline bool operator < (sum lh, sum rh) { return lh.right() == rh.right() ? lh.left() < rh.left() : lh.right() < rh.right(); }
part_t get_part(ostream& os);
inline ostream& operator << (ostream& os, sum s) {
	if(is_mml(os)) {
		auto part = get_part(os);
		if(part == part_t::den)	return os;
		os << part_t::all << "<mrow>" << s.left();
		if(!has_sign(s.right()))	os << "<mo>&plus;</mo>";
		return os << s.right() << "</mrow>" << part;
	} else {
		os << s.left();
		if(!has_sign(s.right()))	os << '+';
		return os << s.right();
	}
}

class xset
{
	list_t	_items;
public:
	xset(const xset& x) { _items = x.items(); }
	xset(expr item) { _items.push_back(item); }
	xset(list_t items) : _items(items) {}
	xset(std::initializer_list<expr> items) : _items(items) {}
	const list_t& items() const { return _items; }
	bool has_sign() const { return false; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	expr simplify() const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (xset lh, xset rh) { return lh.items() == rh.items(); }
inline bool operator < (xset lh, xset rh) { return lh.items() < rh.items(); }

ostream& print_fun(ostream& os, const func& f);
expr approx_fun(expr f, expr x);

class func
{
public:
	struct callbacks {
		callbacks(fmake_t m, fcall_t d = make_dif, fcall_t i = make_int, fcall_t a = approx_fun, fprint_t p = print_fun) : make(m), d(d), integrate(i), approx(a), print(p) {}
		fmake_t make;
		fcall_t d;
		fcall_t integrate;
		fcall_t approx;
		fprint_t print;
	};

	string	  _name;
	expr	  _args;
	callbacks _impl;
public:
	func(string name, expr args);
	func(string name, expr args, expr body);
	func(string name, expr args, callbacks impl) : _name(name), _args(args), _impl(impl) {}
	string name() const { return _name; }
	callbacks impl() const { return _impl; }
	list_t args() const;
	bool has_sign() const { return false; }
	template <typename ... Params> expr operator ()(expr val, Params ... rest) const;
	expr operator()(expr params) const;
	expr x() const { return _args; }
	expr d(expr dx) const;
	expr integrate(expr dx, expr c) const;
	expr subst(pair<expr, expr> s) const;
	expr approx() const;
	expr simplify() const;
	ostream& print(ostream& os) const;
	bool match(expr e, match_result& res) const;
	unsigned exponents(const list_t& vars) const;
};

inline bool operator == (func lh, func rh) { return lh.name() == rh.name() && lh.args() == rh.args(); }
inline bool operator < (func lh, func rh) { return lh.name() < rh.name(); }
inline ostream& operator << (ostream& os, func f) { return f.print(os); }

expr operator + (expr op1, expr op2);
expr operator * (expr op1, expr op2);
expr operator ^ (expr op1, expr op2);
expr operator - (expr op1, expr op2);
expr operator - (expr op1);
expr operator ~ (expr op1);
expr operator * (expr op1);
expr operator / (expr op1, expr op2);
expr operator | (expr op1, symbol op2);
expr operator | (expr op1, pair<expr, expr> op2);

const expr empty = error{error_t::empty};

inline bool failed(expr e) { return e.type() == typeid(error); }
inline string to_string(expr e) { std::stringstream ss; ss << e; return ss.str(); }
inline bool has_sign(expr e) { return boost::apply_visitor([](auto x) { return x.has_sign(); }, e); }
inline expr subst(expr e, pair<expr, expr> s) { return boost::apply_visitor([s](auto x) { return x.subst(s); }, e); }
inline expr subst(expr e, expr from, expr to) { return subst(e, std::make_pair(from, to)); }
inline expr subst(expr e, symbol var)  { return subst(e, var, var.value()); }
inline expr subst(expr e, list_t vars) { 
	list_t from, to;
	for(auto& e : vars)	if(is<symbol>(e))	from.push_back(e), to.push_back(as<symbol>(e).value() == empty ? e : as<symbol>(e).value());
	return subst(e, std::make_pair(expr{from}, expr{to}));
}
inline expr df(expr e, expr dx) { return boost::apply_visitor([dx](auto x) { return x.d(dx); }, e); }
inline expr intf(expr e, expr dx, expr c) { return boost::apply_visitor([dx, c](auto x) { return x.integrate(dx, c); }, e); }
inline expr intf(expr e, expr dx) { return intf(e, dx, expr{0}); }
inline expr intf(expr e, expr dx, expr a, expr b) { auto F = intf(e, dx); return is<func>(F) && as<func>(F).name() == S_INT ? make_intd(e, dx, a, b) : subst(F, dx, b) - subst(F, dx, a); }
inline expr approx(expr e) { return boost::apply_visitor([](auto x) { return x.approx(); }, e); }
inline expr simplify(expr e) { return boost::apply_visitor([](auto x) { return x.simplify(); }, e); }
inline bool match(expr e, expr pattern, match_result& res) { return boost::apply_visitor([e, &res](auto x) { return x.match(e, res); }, pattern); }
inline match_result match(expr e, expr pattern) { match_result res; match(e, pattern, res); return res; }
inline unsigned get_exps(expr e, const list_t& vars) { return boost::apply_visitor([&vars](auto x) { return x.exponents(vars); }, e); }

inline expr error::subst(pair<expr, expr> s) const { return *this; };
inline expr error::d(expr dx) const { return *this; };
inline expr error::integrate(expr dx, expr c) const { return *this; };
inline expr error::approx() const { return *this; };
inline expr error::simplify() const { return *this; };
inline bool error::match(expr e, match_result& res) const { if(e != expr{*this}) res.found = false; return res; };
inline unsigned error::exponents(const list_t& vars) const { return 0; }
inline expr match_result::operator[] (symbol s) { auto it = std::find(matches.begin(), matches.end(), s); return it == matches.end() ? empty : it->value(); }

const expr e = symbol{"#e", numeric{boost::math::constants::e<double>()}};
const expr pi = symbol{"#p", numeric{boost::math::constants::pi<double>()}};
const list_t variables = {symbol{"x"}, symbol{"y"}, symbol{"z"}};

}
