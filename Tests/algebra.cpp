#include "stdafx.h"
#include <codecvt>
#include <complex>
#include "CppUnitTest.h"

#include "../calculus.h"

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
	if(is<integer>(e))	return as<integer>(e).value();
	if(is<real>(e))		return  as<real>(e).value();
	return std::numeric_limits<double>::quiet_NaN();
}

complex_t to_complex(expr e)
{
	return is<complex>(e) ? as<complex>(e).value() : std::numeric_limits<complex_t>::quiet_NaN();
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
			auto i = make_int(42), q = make_rat(1, 2), r = make_real(-1. / 3.), c = make_complex({ 1, -1 });
			Assert::IsFalse(has_sign(err));
			Assert::IsFalse(has_sign(one));
			Assert::IsTrue(has_sign(minus_one));
			Assert::IsFalse(has_sign(q));
			Assert::IsTrue(has_sign(q * minus_one));
			Assert::IsFalse(has_sign(r * minus_one));
			Assert::IsTrue(has_sign(r));
			Assert::IsFalse(has_sign(c));
			Assert::IsTrue(has_sign(c * minus_one));
			Assert::IsTrue(i == i);
			Assert::IsTrue(q == q);
			Assert::IsTrue(r == r);
			Assert::IsTrue(c == c);
			Assert::IsFalse(i == one);
			Assert::IsFalse(q == make_rat(3, 2));
			Assert::IsFalse(r == make_real(0.5));
			Assert::IsFalse(c == make_complex({ 2, 2 }));

			Assert::IsTrue(err + one == err);
			Assert::IsTrue(err + err == err);
			Assert::IsTrue(err * err == err);
			Assert::IsTrue((err ^ err) == err);
			Assert::AreEqual("Invalid arguments", to_string(err).c_str());
		}
		TEST_METHOD(Integers)
		{
			integer three{ 3 }, five{ 5 };
			Assert::AreEqual("8",   to_string(three + five).c_str());
			Assert::AreEqual("2",   to_string(five - three).c_str());
			Assert::AreEqual("15",  to_string(five * three).c_str());
			Assert::AreEqual("4",   to_string((five + three) / two).c_str());
			Assert::AreEqual("125", to_string(five ^ three).c_str());
			integer r = boost::get<integer>(three + five);
			Assert::AreEqual(8, r.value());
			Assert::IsTrue(five - five == zero);
			Assert::IsTrue(boost::get<integer>(minus_one).has_sign());
			Assert::IsFalse(three.has_sign());
			Assert::IsTrue(one < two);
			Assert::IsFalse(two < one);
		}
		TEST_METHOD(Rationals)
		{
			rational half{ 1, 2 }, minus_two_third{ -2, 3 };
			Assert::AreEqual("3/2",  to_string(half + one).c_str());
			Assert::AreEqual("-1/6", to_string(half + minus_two_third).c_str());
			Assert::AreEqual("7/6",  to_string(half - minus_two_third).c_str());
			Assert::AreEqual("-1/3", to_string(half * minus_two_third).c_str());
			Assert::AreEqual("-3/4", to_string(half / minus_two_third).c_str());
			Assert::AreEqual("4/9",  to_string(minus_two_third ^ two).c_str());
			Assert::AreEqual("-3/2", to_string(minus_two_third ^ minus_one).c_str());
			Assert::AreEqual("2",    to_string(make_int(4) ^ half).c_str());
			Assert::AreEqual("9",    to_string(make_rat(1, 27) ^ minus_two_third).c_str());
			Assert::AreEqual("i",    to_string(make_int(-1) ^ half).c_str());
			Assert::AreEqual("1",	 to_string(minus_one ^ two).c_str());
			Assert::AreEqual("-1",   to_string(minus_one ^ make_rat(1, 3)).c_str());
			Assert::AreEqual("2^1/2",to_string(two ^ half).c_str());
			Assert::AreEqual("4^-1/3", to_string(two ^ minus_two_third).c_str());

			Assert::IsTrue(make_rat(4, 6) == make_rat(2,3));
			Assert::IsTrue(make_rat(2, -5) == make_rat(-2, 5));
			Assert::IsTrue(make_rat(2, 2) == one);
			rational r = boost::get<rational>(half / minus_two_third);
			Assert::AreEqual(-3, r.numer());
			Assert::AreEqual(4,  r.denom());
			Assert::IsTrue(half + minus_two_third == make_rat(-1, 6));
			Assert::IsTrue(minus_two_third.has_sign());
			Assert::IsFalse(half.has_sign());
//			Assert::IsTrue(expr{half} < two);
//			Assert::IsFalse(two < expr{half});
		}
		TEST_METHOD(Real)
		{
			auto half = make_real(0.5), three_seconds = make_real(1.5);
			Assert::AreEqual("0.5", to_string(half).c_str());
			Assert::AreEqual("1.5", to_string(three_seconds).c_str());
			Assert::AreEqual("1", to_string(half + make_rat(1, 2)).c_str());
			Assert::AreEqual("2", to_string(half + three_seconds).c_str());
			Assert::AreEqual("-1", to_string(half - three_seconds).c_str());
			Assert::AreEqual("1", to_string(two * half).c_str());
			Assert::AreEqual("3", to_string(three_seconds / half).c_str());
			Assert::AreEqual("2.25", to_string(three_seconds ^ two).c_str());
			Assert::IsTrue(half + three_seconds == two);
//			Assert::IsTrue(half < one);
//			Assert::IsFalse(three_seconds < one);
		}
		TEST_METHOD(Complex)
		{
			complex i{ {0., 1.} }, c1mi{ {1, -1} }, cm5i{ {0, -5.} }, c3p2i{ {3., 2.} };
			Assert::AreEqual("i", to_string(i).c_str());
			Assert::AreEqual("1-i", to_string(c1mi).c_str());
			Assert::AreEqual("-5i", to_string(cm5i).c_str());
			Assert::AreEqual("3+2i", to_string(c3p2i).c_str());
			Assert::AreEqual("4+i", to_string(c1mi + c3p2i).c_str());
			Assert::AreEqual("-2-3i", to_string(c1mi - c3p2i).c_str());
			Assert::AreEqual("5-i", to_string(c1mi * c3p2i).c_str());
			Assert::AreEqual("-2i", to_string(c1mi ^ make_int(2)).c_str());
			Assert::IsTrue(c1mi + i == make_real(1.0));
			Assert::IsTrue(c1mi - make_real(1.5) + i - make_rat(-1, 2) == zero);
			Assert::IsTrue(i < c3p2i);
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
			Assert::AreEqual("x^2+y^2+2xy", to_string((x+y)^2).c_str());
			Assert::AreEqual("x^2+y^2-2xy", to_string((-x+y)^2).c_str());
			Assert::AreEqual("x^2-y^2", to_string((x+y)*(x-y)).c_str());
			Assert::AreEqual("x^3-y^3-3yx^2+3xy^2", to_string((x - y) ^ 3).c_str());
			Assert::AreEqual("x^3-y^3", to_string((x-y)*((x^2)+x*y+(y^2))).c_str());
			Assert::AreEqual("x^3+y^3", to_string((x+y)*((x^2)-x*y+(y^2))).c_str());
			Assert::IsTrue(x+y < x+y+a);
			Assert::IsFalse(x+y+a < x+y);
			Assert::AreEqual("y^2", to_string((x*y) | (x = expr{y})).c_str());
			Assert::AreEqual("11", to_string(((x ^ 2) + 2 * x + 3) | (x = 2)).c_str());
			//auto s = make_sum({ x,y,minus_one });
			//auto p = make_prod({ x,y,one });
			//Assert::AreEqual("x+y-1", to_string(s).c_str());
			//Assert::AreEqual("x·y", to_string(p).c_str());
		}
		TEST_METHOD(Functions)
		{
			symbol x{"x"}, y{"y"};
			// Logarithm
			Assert::AreEqual("1", to_string(ln(e)).c_str());
			Assert::AreEqual("0", to_string(ln(1)).c_str());
			Assert::AreEqual("-inf", to_string(ln(0)).c_str());
			Assert::AreEqual("ln(x)+ln(y)", to_string(ln(x*y)).c_str());
			Assert::AreEqual("yln(x)", to_string(ln(x^y)).c_str());
			// Sine
			Assert::AreEqual("0", to_string(sin(pi)).c_str());
			Assert::AreEqual("3", to_string((sin(pi/3)^2)*4).c_str());
			Assert::AreEqual("x", to_string(sin(arcsin(x))).c_str());
			Assert::AreEqual("x", to_string(cos(arccos(x))).c_str());
			Assert::AreEqual("x", to_string(tg(arctg(x))).c_str());
			Assert::AreEqual("1", to_string((sin(3*x)^2)+(cos(3*x)^2)).c_str());
			Assert::AreEqual("-sin(x^2)", to_string(sin(-x*x)).c_str());
			Assert::AreEqual("cos(x^2)", to_string(cos(-x*x)).c_str());
		}
		TEST_METHOD(Approximation)
		{
			complex i{{0., 1.}};
			symbol x{"x"};
			auto pi_ = boost::math::constants::pi<double>();
			Assert::AreEqual(1., to_real(~(two-1)));
			Assert::AreEqual(1., to_real(~(sin(pi/6)+cos(pi/3))));
			Assert::AreEqual(3., to_real(~((sin(pi/3) + cos(pi/6))^2)));
			Assert::AreEqual(sin(pi_/7.), to_real(~sin(pi/7)));
			Assert::AreEqual(1., to_real(~(ln(x)|(x=e))));
			Assert::AreEqual(-1.,to_real(~(i*(e^(i*pi/2)))));
			Assert::IsTrue(i/2 == (~(ln(i)/pi)));
			Assert::AreEqual(0.5, to_real(~cos(pi/3)));
			Assert::AreEqual(1., to_real(~tg(pi/4)), 1e-8);
			Assert::AreEqual(90., to_real(~(180/pi*arcsin(1))));
			Assert::AreEqual(90., to_real(~(180/pi*arccos(0))));
			Assert::AreEqual(45., to_real(~(180/pi*arctg(1))));
		}
		TEST_METHOD(Derivative)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"};
			Assert::AreEqual("3x^2", to_string(df(x^3, x)).c_str());
			Assert::AreEqual("ln(3)3^x", to_string(df(3^x, x)).c_str());
			Assert::AreEqual("x^x+ln(x)x^x", to_string(df(x^x, x)).c_str());
			Assert::AreEqual("ln(x)", to_string(ln(x)).c_str());
			Assert::AreEqual("2x^-1", to_string(df(ln(x^2), x)).c_str());
			Assert::AreEqual("sin(x^-1)", to_string(sin(1/x)).c_str());
			Assert::AreEqual("-cos(x^-1)x^-2", to_string(df(sin(1/x), x)).c_str());
			Assert::AreEqual("cos(tg(x))", to_string(cos(tg(x))).c_str());
			Assert::AreEqual("-sin(tg(x))cos(x)^-2", to_string(df(cos(tg(x)), x)).c_str());
		}
		TEST_METHOD(Integrals)
		{
			symbol x{"x"}, y{"y"}, a{"a"}, b{"b"}, c{"c"};
			Assert::AreEqual("1", to_string(intf(cos(x), x, 0, pi/2)).c_str());
			Assert::AreEqual("#e^x", to_string(intf(e^x, x)).c_str());
			Assert::AreEqual("sin(x)-cos(x)", to_string(intf(sin(x)+cos(x), x)).c_str());
			Assert::AreEqual("2/3yx^3", to_string(intf(2*x*y*x, x)).c_str());
			Assert::AreEqual("x+10x^2+50x^3+125x^4+125x^5", to_string(intf((5*x+1)^4, x)).c_str());
			Assert::AreEqual("1/5ln(1+5x)", to_string(intf((5 * x + 1) ^ -1, x)).c_str());
			Assert::AreEqual("-1/4x^2+1/2ln(x)x^2", to_string(intf(x*ln(x), x)).c_str());
			Assert::AreEqual("2", to_string(intf(sin(x), x, 0, pi)).c_str());
			Assert::AreEqual("c+int(#e^x^2,x)", to_string(intf(e^(x^2), x, c)).c_str());
			Assert::AreEqual("1/2ln(x)^2", to_string(intf(ln(x)/x, x)).c_str());
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

	};
}