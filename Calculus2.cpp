// Calculus2.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "calculus.h"

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

typedef boost::variant<int, double, complex_t> myvar;
//bool operator < (complex_t lh, complex_t rh) { return lh.real() < rh.real(); }

int main()
{
	symbol x{"x"}, y{"y"}, a{"a"};


	//p = (x - y) ^ 3;
	//func f{fn_user{"f", x*y, x, y}};
	//cout << (f(4, 5)) << end;
	numeric i{complex_t{0., 1.}}, c1mi{complex_t{1, -1}}, cm5i{complex_t{0, -5.}}, c3p2i{complex_t{3., 2.}};
	cout << c1mi + i << endl;

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

