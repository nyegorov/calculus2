// Calculus2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ostream>
#include <sstream>
#include <complex>
#include "calculus.h"

using namespace std;
using namespace cas;
using namespace std::literals;

int main()
{
	symbol x{"x"}, y{"y"}, a{"a"};

	/*sum ms{2, pi};
	//ms.insert(2);
	//ms.insert(pi);
	ms.insert(1);
	ms.insert(a);
	ms.insert(x);

	for(auto e : ms)	cout << e << endl;
	cout << endl;
	ms.erase(ms.begin());
	for(auto e : ms)	cout << e << endl;
	cout << endl;
	auto it = find(ms.begin(), ms.end(), expr{x});
	ms.erase(it);
	for(auto e : ms)	cout << e << endl;
	cout << endl;
	it = find(ms.begin(), ms.end(), expr{pi});
	ms.erase(it);
	for(auto e : ms)	cout << e << endl;
	cout << endl;
	ms.erase(ms.begin());
	for(auto e : ms)	cout << e << endl;
	cout << endl;
	ms.erase(++ms.begin());
	for(auto e : ms)	cout << e << endl;
	cout << endl;*/

	cas::complex i{{0., 1.}};
	//expr v = make_symb(x);
	auto s = make_sum(x, y);
	auto p = make_prod(x, y);

/*	if(!(expr{0.5} < one))
		cout << "wtf!" << endl;*/

/*	cout << (x*x) << endl;
	cout << ((x ^ 2) * (x ^ 3)) << endl;
	cout << ((x ^ 2)^ 3) << endl;
	cout << p << endl;
	cout << (2 * x)*(3 * x) << endl;
	cout << (2 * x * y)*(3 / x) << endl;
	cout << ((x^y)*(x^y)) << endl;
	cout << ((3 * x*y) ^ 3) << endl;*/


	/*cout << (x+x) << endl;
	cout << ((x * 2) * (x * 3)) << endl;
	cout << ((x * 2) * 3) << endl;
	cout << p << endl;
	cout << (2 + x)*(3 + x) << endl;
	cout << (2 + x + y)+(3 - x) << endl;
	cout << ((x*y)+(x*y)) << endl;
	cout << ((3 + x + y) * 3) << endl;*/
	//p = (x - y) ^ 3;
	cout << ((x ^ 3) || x) << endl;
	cout << ((x - y) ^ 3) << endl;
	//cout << ((x - y) ^ 3) << endl;
//	cout << ((x - y)*((x ^ 2) + x*y + (y ^ 2))) << endl;
	cout << p << endl;

	auto mr = match(2*pi, x*y);
	cout << mr[x] << "," << mr[y] << endl;
	mr = match(cos(expr{3}), cos(expr{3}));
	mr = match(cos(expr{3}), cos(x));

	std::complex<double> c(1, 1), z(2,0);
	auto cz = pow(c, z);

	return 0;
}

