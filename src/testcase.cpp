// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#include <iostream>
#include <string>
using namespace std;
class c { public: string m; c():m("m"){}  operator string() const { return m; } };
void f(string s) { cout<<s; }
void testcase() {	c C;	f(C);	/*cout << C; XXX */ }

