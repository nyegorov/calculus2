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
	Context(bool evaluate, const Context *base);
	void Push()		{_locals.push_back(vars_t());}
	void Pop()		{_locals.pop_back();}
	expr& Get(const string& name, bool local = false);
	bool Get(const string& name, expr& result) const;
	void Set(const string& name, const expr& value)		{_locals.front()[name] = value;}
private:
	struct vars_t : public std::map<string, expr> {};
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
	NScript(bool evaluate = true, const char* script = NULL, const Context *pcontext = NULL) : _context(evaluate, pcontext)	{
		_evaluate = evaluate;
		_operators = evaluate ? &_operators_exec : &_operators_nonexec;
		_parser.Init(script);
	}
	~NScript(void)						{};
	expr eval(const char* script);

protected:
	enum Precedence	{Statement, Approx, Assignment, Subst, Addition,Multiplication,Power,Unary,Functional,Primary,Term};
	void Parse(Precedence level, expr& result);
	void ParseVar(expr& result);

	Parser				_parser;
	Context				_context;
	bool				_evaluate;

	typedef void OpFunc(expr& op1, expr& op2, expr& result);
	struct OpInfo { Parser::Token token; OpFunc* op; };
	OpInfo (*_operators)[Term][10];
	static OpInfo _operators_exec[Term][10];
	static OpInfo _operators_nonexec[Term][10];
};

}
