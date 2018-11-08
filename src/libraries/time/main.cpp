#include <iostream>
#include <nlp_time.h>
using namespace std;



int main (int argc, char* argv[])
{
	Time oTimer;
	// oTimer.StartTimer ();
	sleep (1);
	cout << oTimer.SecondsToCompletion (1, 200) << endl;
	cout << oTimer.sTimeToCompletion (1, 200) << endl;
}
