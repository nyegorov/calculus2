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

	struct op_cast : public boost::static_visitor<expr>
	{
		template <typename T, typename U> expr operator()(T, U) const { return error(error_t::cast); }
/*		expr operator()(integer from, rational) const { return rational{ from.value(), 1 }; }
		expr operator()(integer from, real) const { return real{ (real_t)from.value(), 1. }; }
		expr operator()(rational from, real) const { return real{ (real_t)from.numer() / from.denom(), 1. }; }
		expr operator()(integer from, complex) const { return complex{ complex_t{ (real_t)from.value(), 0.0 } }; }
		expr operator()(rational from, complex) const { return complex{ { (real_t)from.numer() / from.denom(), 0 } }; }
		expr operator()(real from, complex) const { return complex{ { from.value(), 0.0 } }; }*/
		template <typename T> expr operator()(T from, symbol) const { return symbol{"", from}; }
		template <typename T> expr operator()(T from, func) const { return func{fn_id{from}}; }
		template <typename T> expr operator()(T from, power) const { return power{from, one}; }
		template <typename T> expr operator()(T from, product) const { return product{ one, from }; }
		template <typename T> expr operator()(T from, sum) const { return sum{ 0, from }; }
	};
	/*
	struct op_add : public boost::static_visitor<expr>
	{
	public:
		template <typename T, typename U> expr operator()(T, U) const {return error(error_t::cast);}
		template <typename T> expr operator()(T lh, T rh) const {return lh + rh;}
	};

	struct op_mul : public boost::static_visitor<expr>
	{
	public:
		template <typename T, typename U> expr operator()(T, U) const { return error(error_t::cast); }
		template <typename T> expr operator()(T lh, T rh) const { return lh * rh; }
	};

	struct op_pwr : public boost::static_visitor<expr>
	{
	public:
		template <typename T, typename U> expr operator()(T lh, U rh) const { return error(error_t::cast); }
		template <typename T> expr operator()(T lh, T rh) const { return lh ^ rh; }
		expr operator()(integer lh, rational rh) const { return lh ^ rh; }
		expr operator()(rational lh, integer rh) const { return lh ^ rh; }
		expr operator()(sum lh, integer rh) const { return lh ^ rh; }
	};

	template <class OP> expr apply_op(expr op1, expr op2) {
		auto res = boost::apply_visitor(OP(), op1, op2);
		if(res != expr{ error_t::cast })	return res;

		if(op1.which() < op2.which()) {
			auto op1c = boost::apply_visitor(op_cast(), op1, op2);
			if(!failed(op1c))	return boost::apply_visitor(OP(), op1c, op2);
		} else {
			auto op2c = boost::apply_visitor(op_cast(), op2, op1);
			if(!failed(op2c))	return boost::apply_visitor(OP(), op1, op2c);
		}

		return error{ error_t::invalid_args };
	}
	*/
	//expr operator + (expr op1, expr op2) { return make_sum(op1, op2); /*apply_op<op_add>(op1, op2); */ }
	//expr operator * (expr op1, expr op2) { return make_prod(op1, op2);/*apply_op<op_mul>(op1, op2);*/ }
	//expr operator ^ (expr op1, expr op2) { return make_power(op1, op2);/*apply_op<op_pwr>(op1, op2);*/ }
	expr operator - (expr op1, expr op2) { return op1 + op2 * minus_one; }
	expr operator / (expr op1, expr op2) { return op1 * (op2 ^ minus_one); }
	expr operator - (expr op1) { return op1 * minus_one; }
	expr operator ~ (expr op1) { return cas::approx(op1); }
	expr operator | (expr op1, symbol op2) { return cas::subst(op1, op2); }
	expr operator | (expr op1, pair<expr, expr> op2) { return cas::subst(op1, op2); }
}