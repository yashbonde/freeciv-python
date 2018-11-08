#include <nlp_time.h>
#include <iostream>
using namespace std;
Time	o_Timers [10];

extern "C" {
void StartTimer (int x);
void StopTimer (int x);
void PrintTimers (void);


void StartTimer (int _iTimer)
{
	o_Timers [_iTimer].StartTimer ();
}

void StopTimer (int _iTimer)
{
	o_Timers [_iTimer].StopTimer ();
}

void PrintTimers (void)
{
	for (int i = 0; i < 3; ++ i)
		cout << i << " : " << o_Timers [i].sStartStopTime () << endl;
}
}
