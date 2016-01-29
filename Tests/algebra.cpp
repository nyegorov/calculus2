#include "stdafx.h"
#include <codecvt>
#include <complex>
#include "CppUnitTest.h"

#pragma warning(disable:4503)

#include "../calculus.h"
#include "../parser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace cas;
using namespace std::literals::complex_literals;

namespace Microsoft {
namespace VisualStudio {
namespace CppUnitTestFramework {
template<> inline std::wstring ToString<expr>(const expr& e)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(to_string(e));
}
}
}
}

real_t to_real(expr e)
{
	if(is<numeric, int_t>(e))	return (real_t)as<numeric, int_t>(e);
	if(is<numeric, real_t>(e))	return as<numeric, real_t>(e);
	return std::numeric_limits<double>::quiet_NaN();
}

complex_t to_complex(expr e)
{
	return is<numeric, complex_t>(e) ? as<numeric, complex_t>(e) : std::numeric_limits<complex_t>::quiet_NaN();
}

namespace Tests
{		
	TEST_CLASS(Algebra)
	{
	public:
		
		TEST_METHOD(Other)
		{
			expr e;
			auto err = make_err(error_t::invalid_args);
			auto i = make_num(42), q = make_num(1, 2), r = make_num(-1. / 3.), c = make_num(1., -1.);
			Assert::IsFalse(has_sign(err));
			Assert::IsFalse(has_sign(one));
			Assert::IsTrue(has_sign(minus_one));
			Assert::IsFalse(has_sign(q));
			Assert::IsTrue(has_sign(q * -1));
			Assert::IsFalse(has_sign(r * -1));
			Assert::IsTrue(has_sign(r));
			Assert::IsFalse(has_sign(c));
			Assert::IsTrue(has_sign(c * -1));
			Assert::IsTrue(i == i);
			Assert::IsTrue(q == q);
			Assert::IsTrue(r == r);
			Assert::IsTrue(c == c);
			Assert::IsFalse(i == make_num(1));
			Assert::IsFalse(q == make_num(3, 2));
			Assert::IsFalse(r == make_num(0.5));
			Assert::IsFalse(c == make_num(2, 2));

			Assert::IsTrue(err + one == err);
			Assert::IsTrue(err + err == err);
			Assert::IsTrue(err * err == err);
			Assert::IsTrue((err ^ err) == err);
			Assert::AreEqual("Invalid arguments", to_string(err).c_str());
		}
		TEST_METHOD(Integers)
		{
			numeric three{ 3 }, five{ 5 };
			Assert::AreEqual("8",   to_string(three + five).c_str());
			Assert::AreEqual("2",   to_string(five - three).c_str());
			Assert::AreEqual("15",  to_string(five * three).c_str());
			Assert::AreEqual("4",   to_string((five + three) / two).c_str());
			Assert::AreEqual("125", to_string(five ^ three).c_str());
			int_t r = boost::get<int_t>(boost::get<numeric>(three + five).value());
			Assert::AreEqual(8, r);
			Assert::IsTrue(five - five == zero);
			Assert::IsTrue(boost::get<numeric>(minus_one).has_sign());
			Assert::IsTrue(one < two);
			Assert::IsFalse(two < one);
		}
		TEST_METHOD(Rationals)
		{
			numeric half{rational_t{ 1, 2 }}, minus_two_third{rational_t{-2, 3}};
			Assert::AreEqual("3/2",  to_string(half + 1).c_str());
			Assert::AreEqual("-1/6", to_string(half + minus_two_third).c_str());
			Assert::AreEqual("7/6",  to_string(half - minus_two_third).c_str());
			Assert::AreEqual("-1/3", to_string(half * minus_two_third).c_str());
			Assert::AreEqual("-3/4", to_string(half / minus_two_third).c_str());
			Assert::AreEqual("4/9",  to_string(minus_two_third ^ 2).c_str());
			Assert::AreEqual("-3/2", to_string(minus_two_third ^ -1).c_str());
			Assert::AreEqual("2",    to_string(make_num(4) ^ half).c_str());
			Assert::AreEqual("9",    to_string(make_num(1, 27) ^ minus_two_third).c_str());
			Assert::AreEqual("i",    to_string(make_num(-1) ^ half).c_str());
			Assert::AreEqual("1",	 to_string(-1 ^ two).c_str());
			Assert::AreEqual("-1",   to_string(-1 ^ make_num(1, 3)).c_str());
			Assert::AreEqual("2^1/2",to_string(two ^ half).c_str());
			Assert::AreEqual("4^-1/3", to_string(two ^ minus_two_third).c_str());
			Assert::AreEqual("2",	 to_string(1+half+1*half+(0^half)).c_str());

			Assert::IsTrue(make_num(4, 6) == make_num(2,3));
			Assert::IsTrue(make_num(2, -5) == make_num(-2, 5));
			Assert::IsTrue(make_num(2, 2) == one);
			rational_t r = as<numeric, rational_t>(half / minus_two_third);
			Assert::AreEqual(-3, r.numer());
			Assert::AreEqual(4,  r.denom());
			Assert::IsTrue(half + minus_two_third == make_num(-1, 6));
			Assert::IsTrue(numeric(minus_two_third).has_sign());
			Assert::IsFalse(numeric(half).has_sign());
			Assert::IsTrue(expr{half} < two);
			Assert::IsFalse(two < expr{half});
		}
		TEST_METHOD(Real)
		{
			auto q = make_num(3, 4);
			auto half = make_num(0.5), three_seconds = make_num(1.5);
			Assert::AreEqual("0.5", to_string(half).c_str());
			Assert::AreEqual("1.5", to_string(three_seconds).c_str());
			Assert::AreEqual("1", to_string(half + make_num(1, 2)).c_str());
			Assert::AreEqual("2", to_string(half + three_seconds).c_str());
			Assert::AreEqual("-1", to_string(half - three_seconds).c_str());
			Assert::AreEqual("1", to_string(two * half).c_str());
			Assert::AreEqual("3", to_string(three_seconds / half).c_str());
			Assert::AreEqual("2.25", to_string(three_seconds ^ two).c_str());
			Assert::AreEqual("2", to_string(q+half+q*half+half*q).c_str());
			Assert::AreEqual(1.46062, to_real(~((q^half) + (half^q))), 0.0001);
			Assert::IsTrue(half + three_seconds == two);
			Assert::IsTrue(half < one);
			Assert::IsFalse(three_seconds < one);
		}
		TEST_METHOD(Complex)
		{
			auto q = make_num(3, 4), r = make_num(0.5), n = make_num(1);
			numeric i{ complex_t{0., 1.} }, c1mi{complex_t{1, -1} }, cm5i{complex_t{0, -5.} }, c3p2i{complex_t{3., 2.} };
			Assert::AreEqual("i", to_string(i).c_str());
			Assert::AreEqual("1-i", to_string(c1mi).c_str());
			Assert::AreEqual("-5i", to_string(cm5i).c_str());
			Assert::AreEqual("3+2i", to_string(c3p2i).c_str());
			Assert::AreEqual("4+i", to_string(c1mi + c3p2i).c_str());
			Assert::AreEqual("-2-3i", to_string(c1mi - c3p2i).c_str());
			Assert::AreEqual("5-i", to_string(c1mi * c3p2i).c_str());
			Assert::AreEqual("-2i", to_string(c1mi ^ make_num(2)).c_str());
			Assert::AreEqual("-0.707107+0.707107i", to_string(-1 ^ q).c_str());
			Assert::AreEqual("3.5+6.25i", to_string((q+i)+(i+q)+(i+n)+(n+i)+r*i+q*i+n*i).c_str());
			Assert::AreEqual("0.958904-0.28373i", to_string(q^i).c_str());
			Assert::IsTrue(c1mi + i == make_num(1.0));
			Assert::IsTrue(c1mi - make_num(1.5) + i - make_num(-1, 2) == zero);
			Assert::IsTrue(i < c3p2i && 0.5 < c3p2i);
			Assert::IsFalse(c3p2i < c1mi);
		}
		TEST_METHOD(Symbolic)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"};
			Assert::AreEqual(x+y, y+x);
			Assert::AreEqual(x*y, y*x);
			Assert::AreEqual(x*(a+b), x*a+x*b);
			Assert::AreEqual("2x", to_string(x+x).c_str());
			Assert::AreEqual("7x", to_string(2*x+5*x).c_str());
			Assert::AreEqual("2xy", to_string(x*y+x*y).c_str());
			Assert::AreEqual("x", to_string(x+0).c_str());
			Assert::AreEqual("1", to_string((x/x)^y).c_str());
			Assert::AreEqual("x^15", to_string((x^5)^3).c_str());
			Assert::AreEqual("(x+y)^(a+b)", to_string((x+y)^(a+b)).c_str());
			Assert::AreEqual("x^2", to_string(x*x).c_str());
			Assert::AreEqual("x^6", to_string((x^2)^3).c_str());
			Assert::AreEqual("x^5", to_string((x^2)*(x^3)).c_str());
			Assert::AreEqual("6yx^2", to_string((2*y*x)*(3*x)).c_str());
			Assert::AreEqual("4y", to_string((2*x*y)*(2/x)).c_str());
			Assert::AreEqual("27x^3y^3", to_string((3*x*y)^3).c_str());
			Assert::AreEqual("x^(2y)", to_string((x^y)*(x^y)).c_str());
			Assert::AreEqual("x^2+2xy+y^2", to_string((x+y)^2).c_str());
			Assert::AreEqual("x^2-2xy+y^2", to_string((-x+y)^2).c_str());
			Assert::AreEqual("x^2-y^2", to_string((x+y)*(x-y)).c_str());
			Assert::AreEqual("x^3-3yx^2+3xy^2-y^3", to_string((x - y) ^ 3).c_str());
			Assert::AreEqual("x^3-y^3", to_string((x-y)*((x^2)+x*y+(y^2))).c_str());
			Assert::AreEqual("x^3+y^3", to_string((x+y)*((x^2)-x*y+(y^2))).c_str());
			Assert::IsTrue(x+y < x+y+a);
			Assert::IsFalse(x+y+a < x+y);
			Assert::AreEqual("y^2", to_string((x*y) | (x = expr{y})).c_str());
			Assert::AreEqual("11", to_string(((x ^ 2) + 2 * x + 3) | (x = 2)).c_str());
		}
		TEST_METHOD(Functions)
		{
			symbol x{"x"}, y{"y"};
			// Logarithm
			Assert::AreEqual("1", to_string(ln(e)).c_str());
			Assert::AreEqual("0", to_string(ln(1)).c_str());
			Assert::AreEqual("-inf", to_string(ln(0)).c_str());
			Assert::AreEqual(ln(x)+ln(y), ln(x*y));
			Assert::AreEqual(y*ln(x), ln(x^y));
			// Sine
			Assert::AreEqual("0", to_string(sin(pi)).c_str());
			Assert::AreEqual("3", to_string((sin(pi/3)^2)*4).c_str());
			Assert::AreEqual("x", to_string(sin(arcsin(x))).c_str());
			Assert::AreEqual("x", to_string(cos(arccos(x))).c_str());
			Assert::AreEqual("x", to_string(tg(arctg(x))).c_str());
			Assert::AreEqual("1", to_string((sin(3*x)^2)+(cos(3*x)^2)).c_str());
			Assert::AreEqual("-sin(x^2)", to_string(sin(-x*x)).c_str());
			Assert::AreEqual("cos(x^2)", to_string(cos(-x*x)).c_str());
			// User
			fun f{"f", x / y,{x,y}};
			Assert::AreEqual(one / 5, f(1, 5));

		}
		TEST_METHOD(Approximation)
		{
			numeric i{complex_t{0., 1.}};
			symbol x{"x"};
			auto pi_ = boost::math::constants::pi<double>();
			Assert::AreEqual(1., to_real(~(two-1)));
			Assert::AreEqual(1., to_real(~(sin(pi/6)+cos(pi/3))));
			Assert::AreEqual(3., to_real(~((sin(pi/3) + cos(pi/6))^2)), 1e-8);
			Assert::AreEqual(sin(pi_/7.), to_real(~sin(pi/7)));
			Assert::AreEqual(1., to_real(~(ln(x)|(x=e))), 1e-8);
			Assert::AreEqual(-1.,to_real(~(i*(e^(i*pi/2)))), 1e-8);
			Assert::IsTrue(i/2 == (~(ln(i)/pi)));
			Assert::AreEqual(0.5, to_real(~cos(pi/3)));
			Assert::AreEqual(1., to_real(~tg(pi/4)), 1e-8);
			Assert::AreEqual(90., to_real(~(180/pi*arcsin(1))));
			Assert::AreEqual(90., to_real(~(180/pi*arccos(0))));
			Assert::AreEqual(45., to_real(~(180/pi*arctg(1))));
			Assert::AreEqual(-1., to_real(~(e^(-i*pi))));
		}
		TEST_METHOD(Derivative)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"};
			fun f{"f", x / y, {x,y}};
			Assert::AreEqual(3*(x^2), df(x^3, x));
			Assert::AreEqual(ln(3)*(3^x), df(3^x, x));
			Assert::AreEqual((x^x)+ln(x)*(x^x), df(x^x, x));
			Assert::AreEqual(ln(x), ln(x));
			Assert::AreEqual(2/x, df(ln(x^2), x));
			Assert::AreEqual(-cos(1/x)/(x^2), df(sin(1/x), x));
			Assert::AreEqual(-sin(tg(x))/(cos(x)^2), df(cos(tg(x)), x));
			//Assert::AreEqual(1 / y, df(f, x));
			//Assert::AreEqual(-1 / (y^2), df(f, y));
			//Assert::AreEqual(1 / y, df(f, x));
		}
		TEST_METHOD(Integrals)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"}, c{"c"};
			Assert::AreEqual(one, intf(cos(x), x, 0, pi/2));
			Assert::AreEqual(e^x, intf(e^x, x));
			Assert::AreEqual(sin(x)-cos(x), intf(sin(x)+cos(x), x));
			Assert::AreEqual(2*y/3*(x^3), intf(2*x*y*x, x));
			Assert::AreEqual(x+10*(x^2)+50*(x^3)+125*(x^4)+125*(x^5), intf((5*x+1)^4, x));
			Assert::AreEqual(ln(1+5*x)/5, intf((5 * x + 1) ^ -1, x));
			Assert::AreEqual(-(x^2)/4+ln(x)*(x^2)/2, intf(x*ln(x), x));
			Assert::AreEqual(two, intf(sin(x), x, 0, pi));
			Assert::AreEqual("int(#e^x^2,x)+c", to_string(intf(e^(x^2), x, c)).c_str());
			Assert::AreEqual((ln(x)^2)/2, intf(ln(x)/x, x));
			Assert::AreEqual(sin(x)*tg(y), df(intf(sin(x)*tg(y), x), x));
			Assert::AreEqual(-cos(x)/(cos(y)^2), df(intf(sin(x)*tg(y), x), y));
			Assert::AreEqual(-ln(cos(y))*cos(x), df(intf(sin(x)*tg(y), y), x));
			Assert::AreEqual((e^2*pi*x)*(x/2/pi-1/(4*pi*pi)), intf(x*(e^2*pi*x), x));
			Assert::AreEqual("d/dy int(x^(xy),x)", to_string(df(intf(x^(x*y), x), y)).c_str());
			symbol rho("rho"), phi("phi"), psi("psi"), R("R");
			Assert::AreEqual(4*pi*(R^3)/3, intf(intf(intf((rho^2)*sin(phi), rho, 0, R), phi, 0, pi), psi, 0, 2*pi));
		}
		TEST_METHOD(Matches)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"};
			match_result mr;
			Assert::AreEqual("cos(a)", to_string(subst(cos(x+y), x+y, a)).c_str());
			Assert::IsTrue((mr = match(2 * ln(x) * cos(x), x*ln(y))) && mr[x] == 2 * cos(x) && mr[y] == expr{x});
			Assert::IsTrue((mr = match(2 * pi, x*y)) && mr[x] == two && mr[y] == pi);
			Assert::IsTrue(mr = match(cos(a+2*pi), cos(x+2*pi)));
			Assert::IsTrue(mr = match(sin(a+2*pi), sin(y)));
			Assert::IsTrue(mr = match((cos(expr{3 / 2}) ^ 2) + (sin(expr{3 / 2}) ^ 2), (cos(x) ^ 2) + (sin(x) ^ 2)));
			Assert::IsFalse(mr = match((cos(expr{3 / 2}) ^ 2) + (sin(expr{5 / 2}) ^ 2), (cos(x) ^ 2) + (sin(x) ^ 2)));
		}
		TEST_METHOD(Parser)
		{
			NScript ns;

			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"};
			Assert::AreEqual(2*x*x+3*x+1, *ns.eval("2*x^2+3*x+1"));
			Assert::AreEqual(cos(y^2), *ns.eval("cos(y^2)"));
			Assert::AreEqual(-2*y*sin(y^2), *ns.eval("df(cos(y^2),y)"));
			Assert::AreEqual(half, *ns.eval("int(1/x^2,x,2,inf)"));
			ns.set("f", fn("f", empty, {x})); ns.set("g", fn("g", empty, {x})); ns.set("h", fn("h", (x^2)/y, {x, y}));
			Assert::AreEqual((y ^ 2) / x, *ns.eval("h(y,x)"));
			Assert::AreEqual(expr{16}/5, *ns.eval("h(4,5)"));
			Assert::AreEqual("g(x)f'(x)+f(x)g'(x)", to_string(*ns.eval("df(f(x)*g(x),x)")).c_str());
			Assert::AreEqual("f'(g(x))g'(x)", to_string(*ns.eval("df(f(g(x)),x)")).c_str());
			ns.set("xx", 4); ns.set("yy", 5);
			Assert::AreEqual(expr{20}, *ns.eval("xx*yy"));
			Assert::AreEqual(sin(x), *ns.eval("int(f(x),x)|f(x)=cos(x)"));
			Assert::AreEqual(a/(b^2), *ns.eval("x/y^2|(x=a,y=b)"));
		}

	};
}