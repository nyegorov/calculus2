#pragma once

#include "common.h"
#include "numeric.h"
#include "functions.h"

namespace cas {

expr error::derive(string dx) const { return _error == error_t::empty ? zero : *this; }

expr symbol::derive(string dx) const
{
	if(_name == dx) return one;
	else			return _value || dx;
}

expr func::derive(string dx) const { return boost::apply_visitor([dx](auto x) { return x.derive(dx); }, _func); }

template <> expr fn_base<fn_id>::derive(std::string dx) const { return _x || dx; }
template <> expr fn_base<fn_ln>::derive(std::string dx) const { return (_x || dx) / _x; }
template <> expr fn_base<fn_sin>::derive(std::string dx) const { return (_x || dx) * cos(_x); }
template <> expr fn_base<fn_cos>::derive(std::string dx) const { return -(_x || dx) * sin(_x); }
template <> expr fn_base<fn_tg>::derive(std::string dx) const { return (_x || dx) / (cos(_x) ^ two); }
template <> expr fn_base<fn_arcsin>::derive(std::string dx) const { return (_x || dx) / ((1-(_x^2)) ^ half); }
template <> expr fn_base<fn_arccos>::derive(std::string dx) const { return -(_x || dx) / ((1 - (_x ^ 2)) ^ half); }
template <> expr fn_base<fn_arctg>::derive(std::string dx) const { return (_x || dx) / (1 + (_x ^ 2)); }

expr power::derive(string dx) const
{
	// (f^g)' = f^g * (g'*ln(f) + g*f'/f)
	return (_x^_y) * ((_y || dx)*ln(_x) + _y / _x*(_x || dx));
}

expr product::derive(string dx) const { return (_left||dx) * _right + _left * (_right||dx); }

expr sum::derive(string dx) const { return (_left||dx) + (_right||dx); }

}
