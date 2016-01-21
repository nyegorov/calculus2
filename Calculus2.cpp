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
	run();
	return 0;
}

