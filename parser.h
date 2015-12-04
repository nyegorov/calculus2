// NScript - lightweight, single-pass parser for executing c-style expressions. 
// Supports integer, double, string and date types. Has limited set of flow 
// control statements (if/then/else, loops). Fully extensible by user-defined 
// objects with IObject interface.

#pragma once

#include "calculus.h"
#include <string>
#include <vector>
#include <map>
#include <set>

namespace cas	{

// Container for storing named objects and variables
class Context
{
public:
	Context(const Context *base);
	void Push()		{_locals.push_back(vars_t());}
	void Pop()		{_locals.pop_back();}
	expr& Get(const string& name, bool local = false);
	bool Get(const string& name, expr& result) const;
	void Set(const string& name, const expr& value)		{_locals.front()[name] = value;}
private:
	typedef std::map<string, expr>	vars_t;
	static vars_t		_globals;
	std::vector<vars_t>	_locals;
};

// Parser of input stream to a list of tokens
class Parser	{
public:
	typedef std::vector<string>	ArgList;
	typedef size_t	State;
	enum Token	{end,mod,assign,ge,gt,le,lt,nequ,name,value,land,lor,lnot,stmt,err,dot,newobj,minus,lpar,rpar,lcurly,rcurly,equ,plus,lsquare,rsquare,multiply,divide,idiv,and,or,not,pwr,comma,unaryplus,unaryminus,forloop,ifop,iffunc,ifelse,func,object,plusset, minusset, mulset, divset, idivset, setvar, my};

	Parser();
	void Init(const char* expr){if(expr)	_content = expr;SetState(0);}
	Token GetToken()			{return _token;}
	const expr& GetValue()	{return _value;}
	const string& GetName()	{return _name;}
	const ArgList& GetArgs()	{return _args;}
	State GetState()			{return _lastpos;}
	void SetState(State state)	{_pos = state;_token=end;Next();}
	string GetContent(State begin, State end)	{return _content.substr(begin, end-begin);}
	void CheckPairedToken(Token token);
	Token Next();
private:
	static char	_decpt;

	Token			_token;
	string			_content;
	State			_pos;
	State			_lastpos;
	expr			_value;
	string			_name;
	ArgList			_args;
	string			_temp;

	char Peek()			{return _pos >= _content.length() ? 0 : _content[_pos];}
	char Read()			{char c = Peek();_pos++;return c;}
	void Back()				{_pos--;}
	void ReadNumber(char c);
	void ReadArgList();
	void ReadName(char c);
};

// Main class for executing scripts
class NScript
{
public:
	NScript(const char* script = NULL, const Context *pcontext = NULL) : _context(pcontext)	{_parser.Init(script);}
	~NScript(void)						{};
	expr eval(const char* script);

protected:
	enum Precedence	{Statement, Assignment,Addition,Multiplication,Power,Unary,Functional,Primary,Term};
	void Parse(Precedence level, expr& result);
	void ParseVar(expr& result, bool local);

	Parser				_parser;
	Context				_context;

	typedef void OpFunc(expr& op1, expr& op2, expr& result);
	static struct OpInfo {Parser::Token token; OpFunc* op;}	_operators[Term][10];
};
/*
// User-defined functions
class Function	: public Object {
public:
	typedef Parser::ArgList	ArgList;
	Function(const ArgList& args, const char* body, const Context *pcontext = NULL) : _args(args), _body(body), _context(pcontext)	{}
	STDMETHODIMP Call(const expr& params, expr& result);
protected:
	~Function()			{}
	const ArgList		_args;
	const string		_body;
	const Context		_context;
};

// Built-in functions
class BuiltinFunction : public Object	{
public:
	typedef void FN(int n, const expr v[], expr& result);
	BuiltinFunction(int count, FN *pfunc) : _count(count), _pfunc(pfunc)	{}
	STDMETHODIMP Call(const expr& params, expr& result)	{
		SafeArray a(const_cast<expr&>(params));
		if(_count >= 0 && _count != a.Count())	return DISP_E_BADPARAMCOUNT;
		(*_pfunc)(a.Count(), a.GetData(), result); 
		return S_OK;
	}
protected:
	~BuiltinFunction()	{}
	const int			_count;
	FN*					_pfunc;
};
*/
}