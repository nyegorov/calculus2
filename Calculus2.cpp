// Calculus2.cpp : Defines the entry point for the console application.
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
	NScript ns;
	symbol x{"x"}, y{"y"}, a{"a"}, b{"b"}, c{"c"};
	numeric i{complex_t{0.0, 1.0}};
	//cout << ns.eval("int(1/x^2,x,2,inf)") << endl;
	run();
	return 0;
}

