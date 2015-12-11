#pragma once

#include "numeric.h"
#include "symbolic.h"
#include "derive.h"

namespace cas {
	
inline expr symbol::subst(pair<expr, expr> s) const { 
	if(is<xset>(s.first) && is<xset>(s.second)) {
		const auto& from = as<xset>(s.first).items(), to = as<xset>(s.second).items();
		for(auto pi = from.cbegin(), pf = to.cbegin(); pi != from.cend(); pi++, pf++)
			if(*pi == expr{*this})	return *pf;
	}
	return expr{*this} == s.first ? s.second : *this; 
}
inline expr power::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_x | s) ^ (_y | s); }
inline expr product::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) * (_right | s); }
inline expr sum::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) + (_right | s); }
inline expr xset::subst(pair<expr, expr> s) const {
	if(expr{*this} == s.first)	return s.second;
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [s](auto e) {return e | s; });
	return{ret};
}

inline expr symbol::approx() const { return _value == empty ? expr{*this} : ~_value; }
inline expr power::approx() const { return ~_x ^ ~_y; }
inline expr product::approx() const { return ~_left * ~_right; }
inline expr sum::approx() const { return ~_left + ~_right; }
inline expr xset::approx() const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [](auto e) {return ~e; });
	return{ret};
}

inline bool symbol::match(expr e, match_result& res) const {
	auto it = find(res.matches.begin(), res.matches.end(), *this);
	if(it == res.matches.end()) {
		if(_value == empty || _value == e)	res.matches.push_back({_name, e});
		else if(expr{*this} != e)	res.found = false;
	} else {
		if(e != it->value())	res.found = false;
	}
	return res.found;
}
inline bool power::match(expr e, match_result& res) const { return is<power>(e) ? cas::match(as<power>(e).y(), _y, res) && cas::match(as<power>(e).x(), _x, res) : res.found = false; }
inline bool xset::match(expr e, match_result& res) const {
	if(!is<xset>(e) || as<xset>(e).items().size() != _items.size())	return res.found = false;
	auto pe = as<xset>(e).items().begin();
	for(auto item : _items)	if(!cas::match(*pe++, item, res)) break;
	return res.found;
}

inline unsigned symbol::exponents(const list_t& vars) const { 
	auto it = find(vars.begin(), vars.end(), expr{*this});
	return it == vars.end() ? 0 : 70 * pwr(100u, vars.size() - std::distance(vars.begin(), it) - 1);
}
inline unsigned power::exponents(const list_t& vars) const { 
	unsigned res = 0;
	double e =	is<numeric, int_t>(_y) ? (real_t)as<numeric, int_t>(_y) :
				is<numeric, rational_t>(_y) ? (real_t)as<numeric, rational_t>(_y) :
				is<numeric, real_t>(_y) ? as<numeric, real_t>(_y) : 9.;

	int n = (int)(e > 1 ? std::min(99., e+70) : e > -1 ? e*20+50 : std::max(0., 30 + e));

	for(auto c = get_exps(_x, vars), i = 0u; c; c /= 100, i++) {
		res += pwr(100u, i) * std::min(99u, (c % 100) * n / 70);
	}

	return res; 
}
inline unsigned product::exponents(const list_t& vars) const { return get_exps(_left, vars) + get_exps(_right, vars); }
inline unsigned sum::exponents(const list_t& vars) const { return std::max(get_exps(_left, vars), get_exps(_right, vars)); }
inline unsigned xset::exponents(const list_t& vars) const { 
	return std::accumulate(_items.begin(), _items.end(), 0u, [&vars](unsigned s, expr e) {return std::max(get_exps(e, vars), s); });
}

inline bool prod_comp::operator ()(const expr& left, const expr& right) const
{
	//auto l = get_exps(left, variables), r = get_exps(right, variables);
	//return l != r ? l < r : left < right;
	return left < right;
}

inline bool sum_comp::operator ()(const expr& left, const expr& right) const
{
	auto l = get_exps(left, variables), r = get_exps(right, variables);
	return l != r ? l > r : left < right;
}

inline expr operator ~ (expr op1) { return cas::approx(op1); }
inline expr operator | (expr op1, symbol op2) { return cas::subst(op1, op2); }
inline expr operator | (expr op1, pair<expr, expr> op2) { return cas::subst(op1, op2); }

}