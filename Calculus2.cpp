﻿// Calculus2.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "calculus.h"
#include "parser.h"

#pragma execution_character_set("utf-8")

#include <ostream>
#include <sstream>
#include <complex>

#include <fcntl.h>
#include <io.h>
#include <codecvt>
#include <locale>

using namespace std;
using namespace cas;
using namespace std::literals;

extern void initStreams();

void run()
{
	string s;
	NScript ns;
	while(true) {
		cout << "> ";
		getline(cin, s);
		if(cin.fail() || s.empty())	break;
		cout << "  " << ns.eval(s.c_str()) << endl;
	}
}

int main()
{
	//run();

	symbol x{"x"}, y{"y"}, a{"a"};


	{
		NScript ns;
		cout << ns.eval("cos(y)") << endl;
		cout << ns.eval("df(cos(y), y)") << endl;
	}

	//p = (x - y) ^ 3;
	fn_user f{"f", x/y, {x, y}};
	auto f4 = f(4);
	auto f45 = f(4, 5);
	cout << f << endl;
	cout << f(4) << endl;
	cout << f(4, 5) << endl;
	

	numeric i{complex_t{0., 1.}}, c1mi{complex_t{1, -1}}, cm5i{complex_t{0, -5.}}, c3p2i{complex_t{3., 2.}}, r(1,2), n(1);

	/*expr e(42);
	1 % e;
	e % 2;
	e % e;
	//foo(1, 2);
	1 % x;
	x % 2;
	x % i;*/

	//cout << has_sign(make_num(-1,2)) << endl;
	//cout << (1*((-y) ^ 2) + 1*(x^2) - 2*x*y) << endl;
	sum s(x, y);
	//expr e(2);
	//cout << s + e << endl;
	//cout << e + s << endl;
	//cout << s + s << endl;
	numeric half{rational_t{1, 2}};
	cout << half+1 << endl;

	cout << intf(2 * x*y*x, x) << endl;

	if(c1mi + i == make_num(1.0))

	cout << (df(intf(sin(x)*tg(x), x), x)) << endl;

	//a = y;
	auto mr = match(2 * ln(x) * cos(x), x*ln(y));
	cout << mr[x] << "," << mr[y] << endl;
	mr = match(cos(expr{3}), cos(expr{3}));
	mr = match(cos(expr{3}), cos(x));

	std::complex<double> c(1, 1), z(2,0);
	auto cz = pow(c, z);

	return 0;
}

