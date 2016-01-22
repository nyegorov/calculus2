#pragma once

#include <iostream>
#include <boost/format.hpp>
#include "common.h"

namespace cas {

inline int get_fmt_idx()   { static int i = std::ios_base::xalloc(); return i; }
inline int get_part_idx()  { static int i = std::ios_base::xalloc(); return i; }
inline int get_bevel_idx() { static int i = std::ios_base::xalloc(); return i; }

inline ostream& fmt_mml(ostream& os) { os.iword(get_fmt_idx()) = 1; return os; }
inline ostream& fmt_plain(ostream& os) { os.iword(get_fmt_idx()) = 0; return os; }

inline ostream& fract_bevel(ostream& os)   { os.iword(get_bevel_idx()) = 1; return os; }
inline ostream& fract_default(ostream& os) { os.iword(get_bevel_idx()) = 0; return os; }

inline ostream& print_num(ostream& os) { os.iword(get_part_idx()) = (long)print_type::num; return os; }
inline ostream& print_den(ostream& os) { os.iword(get_part_idx()) = (long)print_type::den; return os; }
inline ostream& print_all(ostream& os) { os.iword(get_part_idx()) = (long)print_type::all; return os; }

const auto mml = [](bool use_mml) -> decltype(&fmt_mml)		 {return use_mml ? fmt_mml : fmt_plain; };
const auto set_part = [](print_type type) -> decltype(&print_all) { decltype(&print_all) tmp[] = {print_all, print_num, print_den}; return tmp[(long)type]; };

inline bool use_mml(ostream& os) { return os.iword(get_fmt_idx()) != 0; }
inline bool is_bevel(ostream& os) { return os.iword(get_bevel_idx()) != 0; }
inline print_type get_part(ostream& os) { return (print_type)os.iword(get_part_idx()); }
inline bool print_part(ostream& os, print_type type) { return (print_type)os.iword(get_part_idx()) == type; }

inline ostream& mml_fence(ostream& os, expr x, bool fence) { if(fence) os << "<mfenced>"; os << x; if(fence) os << "</mfenced>"; return os; }

inline ostream& operator << (ostream& os, error e) {
	if(e.get() == error_t::empty)	return os;
	if(use_mml(os)) return os << "<merror><mtext>" << error_msgs[(int)e.get()] << "</mtext></merror>";
	else			return os << error_msgs[(int)e.get()];
}

inline ostream& operator << (ostream& os, const rational_t& r) {
	if(use_mml(os)) {
		if(get_part(os) == print_type::num) {
			if(r.denom() == 0)		os << "<mn>&infin;</mn>";
			else if(abs(r.numer()) != 1)	os << "<mn>" << abs(r.numer()) << "</mn>";
		} else if(get_part(os) == print_type::den) {
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
	if(use_mml(os)) {
		if(is<numeric, int_t>(n) || is<numeric, real_t>(n)) {
			if(print_part(os, print_type::den))	return os;
			return os << "<mn>" << n.value() << "</mn>";
		} else
			return os << n.value();
	} else {
		return os << n.value();
	}
}

inline ostream& operator << (ostream& os, symbol s) { 
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
		if(s.name() == "#p")	return os << "<mi>&pi;</mi>";
		if(s.name() == "#e")	return os << "<mi>e</mi>";
		return os << "<mi>" << s.name() << "</mi>";
	}
	else			return os << s.name();
}

inline ostream& print_pwr(ostream& os, expr x, int_t n, int_t d, bool print_exp_1 = false) {
	if(n == 1 && d == 1 && !print_exp_1)		return os << x;
	switch(d) {
	case 0:	 return os << "<msup>", mml_fence(os, x, is<sum>(x) || is<product>(x)), os << "<mn>&infin;</mn></msup>";
	case 1:  return os << "<msup>", mml_fence(os, x, is<sum>(x) || is<product>(x)), os << "<mn>" << n << "</mn></msup>";
	case 2:  return os << "<msqrt>", print_pwr(os, x, n, 1), os << "</msqrt>";
	default: return os << "<mroot>", print_pwr(os, x, n, 1), os << "<mn>" << d << "</mn></mroot>";
	}
}

inline ostream& operator << (ostream& os, power p) {
	if(use_mml(os)) {
		if(is<numeric, int_t>(p.y()))	p = power{p.x(), numeric{as<numeric, int_t>(p.y()), 1}};
		if(is<numeric, rational_t>(p.y())) {
			int_t d = as<numeric, rational_t>(p.y()).denom(), n = as<numeric, rational_t>(p.y()).numer();
			switch(get_part(os)) {
			case print_type::num:	if(n >= 0)		print_pwr(os, p.x(), n, d, true); break;
			case print_type::den:	if(n < 0)		os << print_all, print_pwr(os, p.x(), -n, d), os << print_den; break;
			case print_type::all:	if(n < 0)		os << "<mfrac" << (is_bevel(os) ? " bevelled = 'true'" : "") << "><mn>1</mn>", print_pwr(os, p.x(), -n, d), os << "</mfrac>";
									else			print_pwr(os, p.x(), n, d, true);
			}
			return os;
		}	
		else if(get_part(os) == print_type::all)	os << "<msup>" << p.x() << p.y() << "</msup>";
		else if(get_part(os) == print_type::num)	os << print_all << "<msup>" << p.x() << p.y() << "</msup>" << print_num;
		return os;
	} else {
		if(is<sum>(p.x()) || is<product>(p.x())) os << '(' << p.x() << ')'; else os << p.x();
		os << '^';
		if(is<sum>(p.y()) || is<product>(p.y())) os << '(' << p.y() << ')'; else os << p.y();
		return os;
	}
}

inline ostream& print_num(ostream& os, const expr& e) { return os << fmt_mml << print_num << e;}
inline ostream& print_den(ostream& os, const expr& e) { return os << fmt_mml << print_den << e; }
inline ostream& print_stream(ostream& os, std::ostringstream& os1, const expr& e) { 
	print_part(os1, get_part(os));
	return os1 << fmt_mml << e; 
}
inline ostream& join_streams(ostream& os, std::ostringstream& osl, std::ostringstream& osr, string op, bool lfence, bool rfence)
{
	bool left = !osl.str().empty(), right = !osr.str().empty();
	if(left)			lfence ? os << "<mfenced>" << osl.str() << "</mfenced>" : os << osl.str();
	if(left && right)	os << op;
	if(right)			rfence && left ? os << "<mfenced>" << osr.str() << "</mfenced>" : os << osr.str();
	return os;
}
inline ostream& print_mul(ostream& os, expr left, expr right, print_type part)
{
	std::ostringstream osl, osr;
	if(left == minus_one && part != print_type::den)	osr << "<mo>&minus;</mo>";
	else					osl << fmt_mml << set_part(part) << left;
	osr << fmt_mml << set_part(part) << right;
	join_streams(os, osl, osr, "<mo>&sdot;</mo>", is<sum>(left), is<sum>(right));
	return os;
}

inline ostream& operator << (ostream& os, product p) {
	if(use_mml(os)) {
		if(get_part(os) == print_type::all) {
			std::ostringstream osn, osd;
			print_mul(osn, p.left() == minus_one ? empty : p.left() < zero ? -p.left() : p.left(), p.right(), print_type::num);
			print_mul(osd, p.left(), p.right(), print_type::den);
			if(osn.str().empty())	osn << "<mn>1</mn>";

			os << "<mrow>";
			if(has_sign(p.left()))	os <<"<mo>&minus;</mo>";
			if(osd.str().empty())	os << osn.str();
			else					os << "<mfrac" << (is_bevel(os) ? " bevelled='true'" : "") << "><mrow>" << osn.str() << "</mrow><mrow>" << osd.str() << "</mrow></mfrac>";
			os << "</mrow>";

			return os;
		} else {
			return print_mul(os, p.left(), p.right(), get_part(os));
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

inline ostream& print_fun(ostream& os, const char *name, expr args) { 
	if(print_part(os, print_type::den))	return os;
	if(use_mml(os)) return os << "<mrow><mi>" << name << "</mi><mfenced>" << args << "</mfenced></mrow>";
	else 			return os << name << "(" << args << ')'; 
}

inline ostream& operator << (ostream& os, fn_base<fn_ln> f) { return print_fun(os, "ln", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_sin> f) { return print_fun(os, "sin", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_cos> f) { return print_fun(os, "cos", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_tg> f) { return print_fun(os, "tg", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arcsin> f) { return print_fun(os, "arcsin", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arccos> f) { return print_fun(os, "arccos", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_arctg> f) { return print_fun(os, "arctg", f.x()); }
inline ostream& operator << (ostream& os, fn_base<fn_int> f) { 
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
		if(f.size() == 4)
			return os << fract_bevel << "<mrow><munderover><mo>&int;</mo>" << f[2] << f[3] << "</munderover><mrow>" << fract_default << f[0] << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << f[1] << "</mrow></mrow>";
		else
			return os << "<mrow><mo>&int;</mo>" << f[0] << "<mspace width='thinmathspace'/><mo>&dd;</mo>" << f[1] << "</mrow>";
	}	else
		return os << "int(" << f[0] << ',' << f[1] << ')'; 
}

inline ostream& operator << (ostream& os, fn_base<fn_dif> f) {
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << "<mrow><mi>" << as<func, fn_user>(f[0]).name() << "</mi><mo>&prime;</mo><mfenced>" << as<func, fn_user>(f[0]).args() << "</mfenced></mrow>";
		else																	return os << "<mrow><mfrac><mi>d</mi><mrow><mi>d</mi>" << f[1] << "</mrow></mfrac>" << f[0] << "</mrow>";
	} else {
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << as<func, fn_user>(f[0]).name() << '\'' << '(' << as<func, fn_user>(f[0]).args() << ')';
		else																	return os << "d/d" << f[1] << " " << f[0];
	}
}

inline ostream& operator << (ostream& os, fn_base<fn_assign> f) {
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
		return os << "<mrow>" << f[0] << "<mo>:</mo><mo>=</mo><mspace width='thinmathspace'/>" << f[1] << "</mrow>";
	} else
		return os << f[0] << '=' << f[1];
}

inline ostream& operator << (ostream& os, fn_base<fn_subst> f) {
	auto subs = f[1];
	if(!is<xset>(subs) || as<xset>(subs).items().size() != 2)	return os;
	auto& s = as<xset>(subs).items();
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
		return os << "<mrow><msub><mfenced open='' close='|'>" << f[0] << "</mfenced><mrow>" << s[0] << "<mo>=</mo>" << s[1] << "</mrow></msub></mrow>";
	} else
		return os << f[0] << '|' << s[0] << '=' << s[1];
}

inline ostream& operator << (ostream& os, fn_user f) { 
	if(print_part(os, print_type::den))					return os;
	if(use_mml(os)) return os << "<mrow><mi>" << f.name() << "</mi><mfenced>" << f.args() << "</mfenced></mrow>";
	else			return os << f.name() << '(' << f.args() << ')';
}

inline ostream& operator << (std::ostream& os, const list_t& l) {
	if(use_mml(os)) {
		if(print_part(os, print_type::den))					return os;
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
	if(print_part(os, print_type::den))					return os;
	if(use_mml(os)) return os << "<mfenced open=\"[\" close=\"]\" separators=\";\">" << l.items() << "</mfenced>";
	else			return os << '[' << l.items() << ']';
}

}


namespace std {

using namespace cas;
inline ostream& operator << (ostream& os, complex_t c) {
	if(use_mml(os)) {
		if(print_part(os, print_type::den))	return os;
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

