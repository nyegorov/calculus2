#include "stdafx.h"
#include <locale>
#include <limits>
#include <algorithm>
#include <functional>
#include <assert.h>
#include "parser.h"

namespace cas	{

// process list of argument
/*HRESULT ProcessArgsList(const Parser::ArgList& args, const expr& params, NScript& script)	{
	SafeArray a(const_cast<expr&>(params));
	if(args.size() == 0)	{
		script.AddObject(TEXT("@"),params);
	}	else	{
		if(args.size() != a.Count())	return DISP_E_BADPARAMCOUNT;
		const expr *pargs = a.GetData();
		for(int i = a.Count()-1; i>=0; i--)	script.AddObject(args[i].c_str(),pargs[i]);
	}
	return S_OK;
}

// Function

STDMETHODIMP Function::Call(const expr& params, expr& result)	{
	NScript script(_body.c_str(), &_context);
	HRESULT hr = ProcessArgsList(_args, params, script);
	return FAILED(hr) ? hr : script.Exec(NULL, result);
}*/

// Operators

void OpAssign(expr& op1, expr& op2, expr& result){result = op1;}
void OpAdd(expr& op1, expr& op2, expr& result)	{result = op1 + op2;}
void OpSub(expr& op1, expr& op2, expr& result)	{result = op1 - op2;}
void OpMul(expr& op1, expr& op2, expr& result)	{result = op1 * op2;}
void OpDiv(expr& op1, expr& op2, expr& result)	{result = op1 / op2;}
void OpPow(expr& op1, expr& op2, expr& result)	{result = op1 ^ op2;}
void OpApp(expr& op1, expr& op2, expr& result)	{result = ~op2;}
void OpNull(expr& op1, expr& op2, expr& result)	{}
void OpCall(expr& op1, expr& op2, expr& result)	{
	if(!is<func, fn_user>(op1))	throw error_t::syntax;
	//result = as<func, fn_user>(op1).call(op2);
	if(!is<xset>(op2))	result = as<func, fn_user>(op1)(op2);
	else 				result = as<func, fn_user>(op1).call(op2);
}

Context::vars_t	Context::_globals;
Context::Context(const Context *base) : _locals(1)
{
	if(base)	
		_locals.assign(base->_locals.begin(), base->_locals.end());
		//_locals[0] = base->_locals[0];
	if(_globals.empty())	{
		// Constants
		typedef vars_t::value_type pair;
		_globals.insert(pair("empty",	expr()));

		// Math
		symbol x{"x"}, f{"f"};
		_globals.insert(pair("pi",		pi));
		_globals.insert(pair("e",		e));
		_globals.insert(pair("ln",		fn("ln", ln(x), {x})));
		_globals.insert(pair("sin",		fn("sin", sin(x), {x})));
		_globals.insert(pair("cos",		fn("cos", cos(x), {x})));
		_globals.insert(pair("tg",		fn("tg", tg(x), {x})));
		_globals.insert(pair("arcsin",	fn("arcsin", arcsin(x), {x})));
		_globals.insert(pair("arccos",	fn("arccos", arccos(x), {x})));
		_globals.insert(pair("arctg",	fn("arctg", arctg(x), {x})));
		_globals.insert(pair("df", fn("df", func{fn_dif{xset{f, x}}}, {f, x})));
		//_globals.insert(pair("intf",	fn("arctg", arctg(x), {x})));
	}
}

expr& Context::Get(const string& name, bool local)
{
	if(!local)	{
		for(int i = (int)_locals.size() - 1; i >= -1; i--)	{
			vars_t& plane = i<0?_globals:_locals[i];
			vars_t::iterator p = plane.find(name);
			if(p != plane.end()) return	p->second;
		}
	}
	return _locals.back()[name] = symbol{name};
}

bool Context::Get(const string& name, expr& result) const
{
	for(std::vector<vars_t>::const_reverse_iterator ri = _locals.rbegin(); ri != _locals.rend(); ri++)	{
		vars_t::const_iterator p = ri->find(name);
		if(p != ri->end())	return result = p->second, true;
	}
	return false;
}

// Operator precedence
NScript::OpInfo NScript::_operators[Term][10] = {
	{{Parser::comma,	&OpNull},	{Parser::end, NULL}},
	{{Parser::assign,	&OpAssign},	{Parser::end, NULL}},
	{{Parser::plus,		&OpAdd},	{Parser::minus,	&OpSub},	{Parser::end, NULL}},
	{{Parser::multiply,	&OpMul},	{Parser::divide,&OpDiv},	{Parser::end, NULL}},
	{{Parser::pwr,		&OpPow},	{Parser::end, NULL}},
	{{Parser::not,		&OpApp},	{Parser::end, NULL}},
	{{Parser::lpar,		&OpCall},	{Parser::end, NULL}},
};

expr NScript::eval(const char* script)
{
	expr result = empty;
	try	{
		_parser.Init(script);
		Parse(Statement, result);
		if(_parser.GetToken() != Parser::end)	throw error_t::syntax;
	} catch(error_t e) {
		result = error{e};
	}
	return result;
}

// Parse "var[:=]" statement
void NScript::ParseVar(expr& result, bool local)
{
	string name = _parser.GetName();
	_parser.Next();
	if(_parser.GetToken() == Parser::setvar)	{
		_parser.Next(); 
		Parse(Assignment, result); 
		_context.Set(name, result);
	}	else	{
		result = _context.Get(name, local);
	}
}

void NScript::Parse(Precedence level, expr& result)
{
	// main parse loop
	Parser::Token token = _parser.GetToken();
	if(level == Primary)	{
		// primary expressions
		bool local = false;
		switch(token)	{
			case Parser::value:		result = _parser.GetValue();_parser.Next();break;
			case Parser::my:	
				if(_parser.Next() != Parser::name)	throw error_t::syntax;
				local = true;
			case Parser::name:		ParseVar(result, local);	break;
			case Parser::lpar:
				_parser.Next();
				result = empty;
				Parse(Statement, result);
				_parser.CheckPairedToken(token);
				break;
			case Parser::end:		throw error_t::syntax;
			default:				break;
		}
	} else if(level == Statement) {
		// comma operator, non-associative
		Parse((Precedence)((int)level + 1), result);
		if(_parser.GetToken() == Parser::comma) {
			expr e;// = result;
			list_t a = {result};
			//a.Put(0, *v, true);
			do {
				_parser.Next();
				Parse((Precedence)((int)level + 1), e);
				a.push_back(e);
			} while(_parser.GetToken() == Parser::comma);
			result = a;
		}
	}	else	{
		// all other
		bool noop = true;

		// parse left-hand operand (for binary operators)
		if(level != Unary)	Parse((Precedence)((int)level+1), result);
again:
		if(_parser.GetToken() == Parser::end)	return;
		for(OpInfo *pinfo = _operators[level];pinfo->op;pinfo++)	{
			token = pinfo->token;
			if(token == _parser.GetToken())	{
				if(token != Parser::lpar)	_parser.Next();

				expr left(result), right(empty);
				result = empty;

				// parse right-hand operand
				if(level == Assignment || level == Unary)	Parse(level, right);		// right-associative operators
				else Parse((Precedence)((int)level+1), level == Assignment ? result : right);		// left-associative operators

				// perform operator's action
				(*pinfo->op)(left, right, result);
				noop = false;

				if(level == Unary)	break;
				goto again;
			}
		}
		// for unary operators, return right-hand expression if no operators were found
		if(level == Unary && noop)	Parse((Precedence)((int)level+1), result);
	}
}

// Parser

char Parser::_decpt;

Parser::Parser() {
	_decpt = std::use_facet<std::numpunct<char> >(std::locale()).decimal_point();
	_temp.reserve(32);
	_name.reserve(32);
	_pos = _lastpos = 0;
}

Parser::Token Parser::Next()
{
	_lastpos = _pos;
	char c;
	while(isspace(c = Read()));
	switch(c)	{
		case '\0':	_token = end;break;
		case '+':	_token = plus; break;
		case '-':	_token = minus; break;
		case '*':	_token = multiply;break;
		case '/':	_token = divide;break;
		case '^':	_token = pwr;break;
		case '~':	_token = not;break;
		case '=':	_token = assign;break;
		case '|':	_token = or;break;
		case ',':	_token = comma ; break;
		case '(':	_token = lpar;break;
		case ')':	_token = rpar;break;
		default:
			if(isdigit(c))	{
				ReadNumber(c);
			}	else	{
				ReadName(c);
				_token = name;
			}
			break;
	}
	return _token;
}

// Parse integer/double/hexadecimal value from input stream
void Parser::ReadNumber(char c)
{
	enum NumberStage {nsint,nsdot,nsexp,nspwr} stage = nsint;
	int base = 10, m = c - '0';
	int e1 = 0, e2 = 0, esign = 1;
	bool overflow = false;

	while(c = Read())	{	
		if(isdigit(c))	{
			char v = c - '0';
			if(stage == nsint) {
				if(m > (LONG_MAX - v) / base)	throw error_t::syntax;
				m = m * base + v;
			}
			else if(stage == nsexp)		stage = nspwr;
			else if(stage == nsdot && !overflow)	{
				if(m > (LONG_MAX - v) / base)	overflow = true;
				else							m = m * base + v, e1--;
			}
			if(stage == nspwr)		e2 = e2 * 10  + v;
		}	else if(isxdigit(c))		{
			char v = 10 + (toupper(c) - 'A');
			if(m > (LONG_MAX - v) / base)	throw error_t::syntax;
			m = m * base + v;
		}	else if(c == '.')		{
			if(stage > nsint)	break;
			stage = nsdot;
		}	else if(c == 'e' || c == 'E' || c == 'd' || c == 'D')		{
			if(stage > nsexp)	break;
			stage = nsexp;
		}	else if(c == '+' || c == '-')		{
			if(stage == nsexp)	esign = c == '-' ? -1 : 1;	else break;
			stage = nspwr;
		}	else	break;
	};
	Back();
	if(stage == nsexp)	throw error_t::syntax;
	if(stage == nsint)	{
		_value = m;												// integers
	}	else				_value = m * pow(10., e1+esign*e2);	// floating-point
	_token = Parser::value;
}

// Parse comma-separated arguments list from input stream
void Parser::ReadArgList()	{
	_args.clear();
	_temp.clear();
	char c;
	while(isspace(c = Read()));
	if(c != '(')	{Back(); return;}		// function without parameters

	while(c = Read()) {
		if(isspace(c))	{
		}	else if(c == ',')	{
			_args.push_back(_temp);
			_temp.clear();
		}	else if(c == ')')	{
			if(!_temp.empty()) _args.push_back(_temp);
			break;
		}	else if(!isalnum(c) && c!= '_'){
			throw error_t::syntax;
		}	else	_temp += c;
	}
}

// Parse object name from input stream
void Parser::ReadName(char c)	
{
	if(!isalpha(c) && c != '@' && c != '_')	throw error_t::syntax;
	_name = c;
	while(isalnum(c = Read()) || c == '_')	_name += c;
	Back();
}

void Parser::CheckPairedToken(Parser::Token token)
{
	if(token == lpar && _token != rpar)			throw error_t::syntax;
	if(token == lsquare && _token != rsquare)	throw error_t::syntax;
	if(token == lcurly && _token != rcurly)		throw error_t::syntax;
	if(token == lpar || token == lsquare || token == lcurly)	Next();
}

}