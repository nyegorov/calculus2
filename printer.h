#pragma once

#include <iostream>
#include <boost/format.hpp>
#include "common.h"

namespace cas {

struct io_manip : public std::function<ostream&(ostream&)> { using std::function<ostream&(ostream&)>::function; };
inline ostream& operator << (ostream& os, io_manip man) { return man(os); }

inline int get_fmt_idx()   { static int i = std::ios_base::xalloc(); return i; }
inline int get_part_idx()  { static int i = std::ios_base::xalloc(); return i; }
inline int get_bevel_idx() { static int i = std::ios_base::xalloc(); return i; }

inline ostream& fmt_mml(ostream& os) { os.iword(get_fmt_idx()) = 1; return os; }
inline ostream& fmt_plain(ostream& os) { os.iword(get_fmt_idx()) = 0; return os; }

inline ostream& fract_bevel(ostream& os)   { os.iword(get_bevel_idx()) = 1; return os; }
inline ostream& fract_default(ostream& os) { os.iword(get_bevel_idx()) = 0; return os; }

inline ostream& print_num(ostream& os) { os.iword(get_part_idx()) = (long)part_t::num; return os; }
inline ostream& print_den(ostream& os) { os.iword(get_part_idx()) = (long)part_t::den; return os; }
inline ostream& print_all(ostream& os) { os.iword(get_part_idx()) = (long)part_t::all; return os; }

const auto mml = [](bool is_mml) -> decltype(&fmt_mml)		 {return is_mml ? fmt_mml : fmt_plain; };

inline bool is_mml(ostream& os) { return os.iword(get_fmt_idx()) != 0; }
inline bool is_bevel(ostream& os) { return os.iword(get_bevel_idx()) != 0; }
inline bool is_den(ostream& os) { return (part_t)os.iword(get_part_idx()) == part_t::den; }
inline part_t get_part(ostream& os) { return (part_t)os.iword(get_part_idx()); }

template <class T> io_manip mfence(const T& x, bool fence) {
	return io_manip{[=](ostream& os) -> ostream& { if(fence) os << "<mfenced>"; os << x; if(fence) os << "</mfenced>"; return os; }};
}

inline ostream& operator << (ostream& os, part_t part) {
	decltype(&print_all) tmp[] = {print_all, print_num, print_den};
	return os << tmp[(long)part];
}

inline ostream& operator << (ostream& os, error e) {
	if(e.get() == error_t::empty)	return os;
	if(is_mml(os)) return os << "<merror><mtext>" << error_msgs[(int)e.get()] << "</mtext></merror>";
	else			return os << error_msgs[(int)e.get()];
}

inline ostream& operator << (ostream& os, const rational_t& r) {
	if(is_mml(os)) {
		if(get_part(os) == part_t::num) {
			if(r.denom() == 0)		os << "<mn>&infin;</mn>";
			else if(abs(r.numer()) != 1)	os << "<mn>" << abs(r.numer()) << "</mn>";
		} else if(get_part(os) == part_t::den) {
			if(r.denom() != 0 && r.denom() != 1) os << "<mn>" << r.denom() << "</mn>";
		} else {
			if(r.denom())			os << (r.numer() < 0 ? "<mo>&minus;</mo>" : "") << "<mfrac" << (is_bevel(os) ? " bevelled='true'" : "") << "><mn>" << abs(r.numer()) << "</mn><mn>" << r.denom() << "</mn></mfrac>";
			else if(r.numer() < 0)	os << "<mrow><mo>&minus;</mo><mn>&infin;</mn></mrow>";
			else					os << "<mn>&infin;</mn>";
		}
		return os;
	} else {
		if(r.denom() == 0)	return os << (r.numer() > 0 ? "inf" : "-inf");
		return os << r.numer() << "/" << r.denom();
	}
}

inline ostream& operator << (ostream& os, numeric n) { 
	if(is_mml(os)) {
		if(is<numeric, int_t>(n) || is<numeric, real_t>(n)) {
			if(is_den(os))	return os;
			return os << "<mn>" << n.value() << "</mn>";
		} else
			return os << n.value();
	} else {
		return os << n.value();
	}
}

inline ostream& operator << (ostream& os, symbol s) { 
	if(is_mml(os)) {
		if(is_den(os))			return os;
		if(s.name() == "#p")	return os << "<mi>&pi;</mi>";
		if(s.name() == "#e")	return os << "<mi>e</mi>";
		return os << "<mi>" << s.name() << "</mi>";
	}
	else			return os << s.name();
}

inline io_manip print_pwr(const expr& x, int_t numer, int_t denom, bool print_exp_1 = false) {
	return io_manip{[=](ostream& os) -> ostream& {
		if(numer == 1 && denom == 1 && !print_exp_1)		return os << x;
		switch(denom) {
		case 0:	 return os << "<msup>"  << mfence(x, is<sum>(x) || is<product>(x)) << "<mn>&infin;</mn></msup>";
		case 1:  return os << "<msup>"  << mfence(x, is<sum>(x) || is<product>(x)) << "<mn>" << numer << "</mn></msup>";
		case 2:  return os << "<msqrt>" << print_pwr(x, numer, 1) << "</msqrt>";
		default: return os << "<mroot>" << print_pwr(x, numer, 1) << "<mn>" << denom << "</mn></mroot>";
		}
	}};
}

inline ostream& operator << (ostream& os, power p) {
	if(is_mml(os)) {
		if(is<numeric, int_t>(p.y()))		p = power{p.x(), numeric{as<numeric, int_t>(p.y()), 1}};
		if(is<numeric, rational_t>(p.y())) {
			int_t d = as<numeric, rational_t>(p.y()).denom(), n = as<numeric, rational_t>(p.y()).numer();
			switch(get_part(os)) {
			case part_t::num:	if(n >= 0)	os << print_pwr(p.x(), n, d, true); break;
			case part_t::den:	if(n < 0)	os << part_t::all << print_pwr(p.x(), -n, d) << part_t::den; break;
			case part_t::all:	if(n < 0)	os << "<mfrac" << (is_bevel(os) ? " bevelled = 'true'" : "") << "><mn>1</mn>" << print_pwr(p.x(), -n, d) << "</mfrac>";
									else	os << print_pwr(p.x(), n, d, true);
			}
			return os;
		} else {
			switch(get_part(os)) {
			case part_t::all:	os << "<msup>" << mfence(p.x(), is<sum>(p.x()) || is<product>(p.x())) << p.y() << "</msup>"; break;
			case part_t::num:	os << part_t::all << "<msup>" << mfence(p.x(), is<sum>(p.x()) || is<product>(p.x())) << p.y() << "</msup>" << part_t::num; break;
			}
		}
		return os;
	} else {
		if(is<sum>(p.x()) || is<product>(p.x())) os << '(' << p.x() << ')'; else os << p.x();
		os << '^';
		if(is<sum>(p.y()) || is<product>(p.y())) os << '(' << p.y() << ')'; else os << p.y();
		return os;
	}
}

inline ostream& join_streams(ostream& os, std::ostringstream& osl, std::ostringstream& osr, string op, bool lfence, bool rfence)
{
	bool left = !osl.str().empty(), right = !osr.str().empty();
	if(left)			os << mfence(osl.str(), lfence && right);
	if(left && right)	os << op;
	if(right)			os << mfence(osr.str(), rfence && left);
	return os;
}

static void get_multiplicands(list_t& multiplicands, const expr& x, part_t part)
{
	if(is<product>(x)) {
		get_multiplicands(multiplicands, as<product>(x).left(), part);
		get_multiplicands(multiplicands, as<product>(x).right(), part);
	} else if(is<power>(x)) {
		if(as<power>(x).y() == minus_one && part == part_t::den)	multiplicands.push_back(as<power>(x).x());
		else if(as<power>(x).y() < zero && part == part_t::den)		multiplicands.push_back(x);
		else if(as<power>(x).y() >= zero && part != part_t::den)	multiplicands.push_back(x);
	} else if(is<numeric, rational_t>(x)) {
		multiplicands.push_back(numeric{part == part_t::den ? as<numeric, rational_t>(x).denom() : as<numeric, rational_t>(x).denom()});
	} else
		if(part != part_t::den)	multiplicands.push_back(x);
}

inline bool need_fence(const expr& x, part_t part)
{
	list_t m;
	get_multiplicands(m, x, part);
	return m.size() == 1 && is<sum>(m.front());
}

inline io_manip print_mul(const expr& left, const expr& right) {
	return io_manip{[=](ostream& os) -> ostream& {
		std::ostringstream osl, osr;
		part_t part = get_part(os);
		if(left == minus_one && part != part_t::den)	osr << "<mo>&minus;</mo>";
		else	osl << mml(true) << part << left;
		osr << mml(true) << part << right;
		return join_streams(os, osl, osr, "<mo lspace='verythinmathspace' rspace='verythinmathspace'>&sdot;</mo>", need_fence(left, part), need_fence(right, part));
	}};
}

inline ostream& operator << (ostream& os, product p) {
	if(is_mml(os)) {
		if(get_part(os) == part_t::all) {
			std::ostringstream osn, osd;
			osn << part_t::num << print_mul(p.left() == minus_one ? empty : p.left() < zero ? -p.left() : p.left(), p.right());
			osd << part_t::den << print_mul(p.left(), p.right());
			if(osn.str().empty())	osn << "<mn>1</mn>";

			os << "<mrow>";
			if(has_sign(p.left()))	os <<"<mo>&minus;</mo>";
			if(osd.str().empty())	os << osn.str();
			else					os << "<mfrac" << (is_bevel(os) ? " bevelled='true'" : "") << "><mrow>" << osn.str() << "</mrow><mrow>" << osd.str() << "</mrow></mfrac>";
			os << "</mrow>";

			return os;
		} else {
			return os << print_mul(p.left(), p.right());
		}
	} else {
		if(p.left() == minus_one)	os << '-'; else os << p.left();
		return os << p.right();
	}
}

/*inline ostream& operator << (ostream& os, sum s) {
	if(use_mml(os)) {
		os << "<mrow><mrow>" << s.left() << "</mrow>";
		if(!has_sign(s.right()))	os << "<mo>&plus;</mo>";
		if(is<sum>(s.right()))		return os << s.right() << "</mrow>";
		else						return os << "<mrow>" << s.right() << "</mrow></mrow>";
	} else {
		os << s.left();
		if(!has_sign(s.right()))	os << '+';
		return os << s.right();
	}
}
*/
/*
inline io_manip print_fun(const char *name, const expr& args) {
	return io_manip{[=](ostream& os) -> ostream& {
		if(is_den(os))	return os;
		if(is_mml(os))	return os << "<mrow><mi>" << name << "</mi><mfenced>" << args << "</mfenced></mrow>";
		else 			return os << name << "(" << args << ')';
	}};
}

inline ostream& operator << (ostream& os, fn_base<fn_ln> f)		{ return os << print_fun("ln", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_sin> f)	{ return os << print_fun("sin", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_cos> f)	{ return os << print_fun("cos", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_tg> f)		{ return os << print_fun("tg", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arcsin> f)	{ return os << print_fun("arcsin", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arccos> f)	{ return os << print_fun("arccos", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arctg> f)	{ return os << print_fun("arctg", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_int> f) { 
	if(is_mml(os)) {
		if(is_den(os))					return os;
		if(f.size() == 4)
			return os << fract_bevel << "<mrow><munderover><mo>&int;</mo>" << f[2] << f[3] << "</munderover><mrow>" << fract_default << f[0] << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << f[1] << "</mrow></mrow>";
		else
			return os << "<mrow><mo>&int;</mo>" << f[0] << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << f[1] << "</mrow>";
	}	else
		return os << "int(" << f[0] << ',' << f[1] << ')'; 
}

inline ostream& operator << (ostream& os, fn_base<fn_dif> f) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << "<mrow><mi>" << as<func, fn_user>(f[0]).name() << "</mi><mo>&prime;</mo><mfenced>" << as<func, fn_user>(f[0]).args() << "</mfenced></mrow>";
		else																	return os << "<mrow><mfrac><mi>d</mi><mrow><mi>d</mi>" << f[1] << "</mrow></mfrac>" << f[0] << "</mrow>";
	} else {
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << as<func, fn_user>(f[0]).name() << '\'' << '(' << as<func, fn_user>(f[0]).args() << ')';
		else																	return os << "d/d" << f[1] << " " << f[0];
	}
}

inline ostream& operator << (ostream& os, fn_base<fn_assign> f) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		return os << "<mrow>" << f[0] << "<mo>=</mo><mspace width='thinmathspace'/>" << f[1] << "</mrow>";
	} else
		return os << f[0] << '=' << f[1];
}

inline ostream& operator << (ostream& os, fn_base<fn_subst> f) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		return os << "<mrow><msub>" << f[0] << "<mfenced open='|' close=''>" << f[1] << "</mfenced></msub></mrow>";
	} else
		return os << f[0] << '|' << f[1];
}
*/
inline io_manip print_arg(const list_t& args) {
	return io_manip{[&](ostream& os) -> ostream& {
		for(const auto& a : args) os << (is<symbol>(a) && as<symbol>(a).value() != empty ? as<symbol>(a).value() : a);
		return os;
	}};
}
/*
inline ostream& operator << (ostream& os, fn_user f) { 
	if(is_den(os))					return os;
	if(is_mml(os)) return os << "<mrow><mi>" << f.name() << "</mi><mfenced>" << print_arg(f.args()) << "</mfenced></mrow>";
	else			return os << f.name() << '(' << f.args() << ')';
}*/

inline ostream& print_fun(ostream& os, const func& f)
{
	if(is_den(os))	return os;
	if(is_mml(os))	return os << "<mrow><mi>" << f.name() << "</mi><mfenced>" << print_arg(f.args()) << "</mfenced></mrow>";
	else			return os << f.name() << '(' << f.args() << ')';
}

inline ostream& print_dif(ostream& os, expr f, expr dx) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		if(is<func>(f) && as<func>(f).x() == dx)	return os << "<mrow><mi>" << as<func>(f).name() << "</mi><mo>&prime;</mo><mfenced>" << as<func>(f).args() << "</mfenced></mrow>";
		else										return os << "<mrow><mfrac><mi>d</mi><mrow><mi>d</mi>" << dx << "</mrow></mfrac>" << f << "</mrow>";
	} else {
		if(is<func>(f) && as<func>(f).x() == dx)	return os << as<func>(f).name() << '\'' << '(' << as<func>(f).args() << ')';
		else										return os << "d/d" << dx << " " << f;
	}
}

inline ostream& print_int(ostream& os, expr f, expr dx, expr a = empty, expr b = empty) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		if(a != empty || b != empty)
			return os << fract_bevel << "<mrow><munderover><mo>&int;</mo>" << a << b << "</munderover><mrow>" << fract_default << f << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << dx << "</mrow></mrow>";
		else
			return os << "<mrow><mo>&int;</mo>" << f << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << dx << "</mrow>";
	} else
		return os << "int(" << f << ',' << dx << ')';
}

inline ostream& print_assign(ostream& os, expr x, expr y) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		return os << "<mrow>" << x << "<mo>=</mo><mspace width='thinmathspace'/>" << y << "</mrow>";
	} else
		return os << x << '=' << y;
}

inline ostream& print_subst(ostream& os, expr x, expr y) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		return os << "<mrow><msub>" << x << "<mfenced open='|' close=''>" << y << "</mfenced></msub></mrow>";
	} else
		return os << x << '|' << y;
}

inline ostream& operator << (std::ostream& os, const list_t& l) {
	if(is_mml(os)) {
		if(is_den(os))					return os;
		for(auto& e : l) os << e;
	} else {
		for(auto it = l.cbegin(); it != l.cend(); ++it) {
			if(it != l.cbegin())	os << ',';
			os << *it;
		}
	}
	return os;
}

inline ostream& operator << (ostream& os, xset l) { 
	if(is_den(os))					return os;
	if(is_mml(os)) return os << "<mfenced open=\"[\" close=\"]\" separators=\";\">" << l.items() << "</mfenced>";
	else			return os << '[' << l.items() << ']';
}

}


namespace std {

using namespace cas;
inline ostream& operator << (ostream& os, complex_t c) {
	if(is_mml(os)) {
		if(is_den(os))	return os;
		os << "<mrow>";
		if(abs(c.real()) >= std::numeric_limits<real_t>::epsilon())	os << "<mn>" << c.real() << "</mn><mo>" << (c.imag() > 0 ? "&plus;" : "&minus;") << "</mo>";
		else if(c.imag() < 0)										os << "<mo>&minus;</mo>";
		if(abs(c.imag()) != 1.)	os << "<mn>" << abs(c.imag()) << "</mn>";
		return os << "<mi>&ImaginaryI;</mi></mrow>";
	} else {
		if(abs(c.real()) >= std::numeric_limits<real_t>::epsilon())	os << c.real() << (c.imag() > 0 ? '+' : '-');
		else if(c.imag() < 0)										os << '-';
		if(abs(c.imag()) != 1.)	os << abs(c.imag());
		return os << 'i';
	}
}

}

