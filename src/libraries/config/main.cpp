#include <iostream>
#include "nlp_config.h"
using namespace std;


int main (int argc, const char* argv [])
{
	Config::SetCommandLine (argc, argv);

	cout << Config::Get ("wow") << endl;
	cout << (config)"what" << endl;

	return 0;
}
