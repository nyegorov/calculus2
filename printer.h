#pragma once

#include <iostream>
#include "common.h"

namespace cas {

inline int get_idx() {
	static int i = std::ios_base::xalloc();
	return i;
}

inline ostream& fmt_mml(ostream& os) { os.iword(get_idx()) = 1; return os; }
inline ostream& fmt_plain(ostream& os) { os.iword(get_idx()) = 0; return os; }

const auto mml = [](bool use_mml) -> decltype(&fmt_mml) {return use_mml ? fmt_mml : fmt_plain; };
inline bool use_mml(ostream& os) { return os.iword(get_idx()) != 0; }

inline ostream& operator << (ostream& os, error e) {
	if(use_mml(os)) return os << "<merror><mtext>" << error_msgs[(int)e.get()] << "</mtext></merror>";
	else			return os << error_msgs[(int)e.get()];
}

inline ostream& operator << (ostream& os, const rational_t& r) {
	if(use_mml(os)) {
		if(r.denom())	return os << (r.numer() < 0 ? "<mo>&minus;</mo>" : "") << "<mfrac><mn>" << abs(r.numer()) << "</mn><mn>" << r.denom() << "</mn></mfrac>";
		if(r.numer() < 0) os << "<mo>&minus;</mo>"; 
		return os << "<mn>&infin;</mn>";
	} else {
		if(r.denom() == 0)	return os << (r.numer() > 0 ? "inf" : "-inf");
		return os << r.numer() << "/" << r.denom();
	}
}

inline ostream& operator << (ostream& os, numeric n) { 
	if(use_mml(os)) {
		if(is<numeric, int_t>(n) || is<numeric, real_t>(n))	return os << "<mn>" << n.value() << "</mn>";
		else												return os << n.value();
	} else {
		return os << n.value();
	}
}

inline ostream& operator << (ostream& os, symbol s) { 
	if(use_mml(os)) {
		if(s.name() == "#p")	return os << "<mi>&pi;</mi>";
		if(s.name() == "#e")	return os << "<mi>e</mi>";
		return os << "<mi>" << s.name() << "</mi>";
	}
	else			return os << s.name();
}

inline void print_xn(ostream& os, expr x, int_t n) {
	if(n == 1)	os << x;
	else {
		os << "<msup>" << x << "<mn>" << n << "</mn></msup>";
	}
}

inline ostream& operator << (ostream& os, power p) {
	if(use_mml(os)) {
		if(is<numeric, int_t>(p.y())) {
			int n = as<numeric, int_t>(p.y());
			if(n < 0)	{ os << "<mfrac><mn>1</mn>"; print_xn(os, p.x(), abs(n)); return os << "</mfrac>"; }
			else		return print_xn(os, p.x(), n), os;
		} else if(is<numeric, rational_t>(p.y())) {
			int_t d = as<numeric, rational_t>(p.y()).denom(), n = as<numeric, rational_t>(p.y()).numer();
			if(n < 0)	os << "<mfrac><mn>1</mn>";
			if(d == 2) {
				os << "<msqrt>"; print_xn(os, p.x(), abs(n)); os << "</msqrt>";
			} else {
				os << "<mroot>"; print_xn(os, p.x(), abs(n)); os << "<mn>" << d << "</mn></mroot>";
			}
			if(n < 0)	os << "</mfrac>";
			return os;
		} else {
			return os << "<msup>" << p.x() << p.y() << "</msup>";
		}
	} else {
		if(is<sum>(p.x()) || is<product>(p.x())) os << '(' << p.x() << ')'; else os << p.x();
		os << '^';
		if(is<sum>(p.y()) || is<product>(p.y())) os << '(' << p.y() << ')'; else os << p.y();
		return os;
	}
}


inline ostream& operator << (ostream& os, product p) {
	if(use_mml(os)) {
		os << "<mrow>";
		if(p.left() == expr{-1})	os << "<mo>&minus;</mo>";
		else						os << p.left();
		//os << "<mo>&it;</mo>";
		return os << p.right() << "</mrow>";
	} else {
		if(p.left() == expr{-1})	os << '-'; else os << p.left();
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
		if(f.size() == 4)
			return os << "<mrow><munderover><mo>&int;</mo>" << f[2] << f[3] << "</munderover>" << f[0] << "<mspace width=\"thinmathspace\"/><mi>d</mi>" << f[1] << "</mrow>";
		else
			return os << "<mrow><mo>&int;</mo>" << f[0] << "<mspace width=\"thinmathspace\"/><mi>d</mi>" << f[1] << "</mrow>";
	}	else
		return os << "int(" << f[0] << ',' << f[1] << ')'; 
}

inline ostream& operator << (ostream& os, fn_base<fn_dif> f) {
	if(use_mml(os)) {
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << "<mrow><mi>" << as<func, fn_user>(f[0]).name() << "</mi><mo>&prime;</mo><mfenced>" << as<func, fn_user>(f[0]).args() << "</mfenced></mrow>";
		else																	return os << "<mrow><mfrac><mi>d</mi><mrow><mi>d</mi>" << f[1] << "</mrow></mfrac>" << f[0] << "</mrow>";
	} else {
		if(is<func, fn_user>(f[0]) && as<func, fn_user>(f[0]).body() == empty)	return os << as<func, fn_user>(f[0]).name() << '\'' << '(' << as<func, fn_user>(f[0]).args() << ')';
		else																	return os << "d/d" << f[1] << " " << f[0];
	}
}

inline ostream& operator << (ostream& os, fn_user f) { 
	if(use_mml(os)) return os << "<mrow><mi>" << f.name() << "</mi><mfenced>" << f.args() << "</mfenced></mrow>";
	else			return os << f.name() << '(' << f.args() << ')';
}

inline ostream& operator << (std::ostream& os, const list_t& l) {
	if(use_mml(os)) {
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
	if(use_mml(os)) return os << "<mfenced open=\"[\" close=\"]\" separators=\";\">" << l.items() << "</mfenced>";
	else			return os << '[' << l.items() << ']';
}

}


namespace std {

using namespace cas;
inline ostream& operator << (ostream& os, complex_t c) {
	if(use_mml(os)) {
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

