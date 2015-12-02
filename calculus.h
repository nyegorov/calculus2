#pragma once

#include "numeric.h"
#include "symbolic.h"
#include "derive.h"

namespace cas {
	
expr symbol::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : *this; }
expr power::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_x | s) ^ (_y | s); }
expr product::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) * (_right | s); }
expr sum::subst(pair<expr, expr> s) const { return expr{*this} == s.first ? s.second : (_left | s) + (_right | s); }
expr xset::subst(pair<expr, expr> s) const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [s](auto e) {return e | s; });
	return{ret};
}

expr symbol::approx() const { return _value == empty ? expr{*this} : ~_value; }
expr power::approx() const { return ~_x ^ ~_y; }
expr product::approx() const { return ~_left * ~_right; }
expr sum::approx() const { return ~_left + ~_right; }
expr xset::approx() const {
	list_t ret;
	transform(_items.begin(), _items.end(), back_inserter(ret), [](auto e) {return ~e; });
	return{ret};
}

bool symbol::match(expr e, match_result& res) const {
	auto it = find(res.matches.begin(), res.matches.end(), *this);
	if(it == res.matches.end()) {
		res.matches.push_back({_name, e});
	} else {
		if(e != it->value())	res.found = false;
	}
	return res.found;
}
bool power::match(expr e, match_result& res) const { return is<power>(e) ? cas::match(as<power>(e).y(), _y, res) && cas::match(as<power>(e).x(), _x, res) : res.found = false; }
bool xset::match(expr e, match_result& res) const {
	if(!is<xset>(e) || as<xset>(e).items().size() != _items.size())	return res.found = false;
	auto pe = as<xset>(e).items().begin();
	for(auto item : _items)	if(!cas::match(*pe++, item, res)) break;
	return res.found;
}

expr operator ~ (expr op1) { return cas::approx(op1); }
expr operator | (expr op1, symbol op2) { return cas::subst(op1, op2); }
expr operator | (expr op1, pair<expr, expr> op2) { return cas::subst(op1, op2); }

}