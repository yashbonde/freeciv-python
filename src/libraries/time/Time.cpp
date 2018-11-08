#include "nlp_time.h"
#include <iostream>
#include <assert.h>
using namespace std;


//													
String Time::CurrentTime (void)
{
	String sTime;

	timeval tv;
	timerclear (&tv);
	if (0 != gettimeofday (&tv, NULL))
		return sTime;	

	sTime << tv.tv_sec << "s, " << tv.tv_usec << "us";
	return sTime;
}



//													
Time::Time (void)
{
	timerclear (&tv_Start);
	timerclear (&tv_Elapsed);
}


//													
void Time::Reset (void)
{
	timerclear (&tv_Start);
	timerclear (&tv_Elapsed);
}


//													
void Time::StartTimer (void)
{
	timerclear (&tv_Start);
	// timerclear (&tv_Elapsed);
	if (0 != gettimeofday (&tv_Start, NULL))
		cerr << "[ERROR]  Call to gettimeofday failed in Time::StartTimer" << endl;
}


//													
#define MILLION		1000000
void Time::StopTimer (void)
{
	timeval tvNow;
	timerclear (&tvNow);
	if (0 != gettimeofday (&tvNow, NULL))
	{
		cerr << "[ERROR]  Call to gettimeofday failed in Time::StartTimer" << endl;
		return;
	}

	timeval tvDiff;
	timersub (&tvNow, &tv_Start, &tvDiff);
	timeradd (&tv_Elapsed, &tvDiff, &tv_Elapsed);
}


//													
timeval Time::DiffToNow (void)
{
	timeval tvDiff;
	timerclear (&tvDiff);

	timeval tvNow;
	timerclear (&tvNow);
	if (0 != gettimeofday (&tvNow, NULL))
	{
		cerr << "[ERROR]  Call to gettimeofday failed in Time::StartTimer" << endl;
		return tvDiff;
	}

	timersub (&tvNow, &tv_Start, &tvDiff);
	return tvDiff;
}


//													
time_t Time::SecondsToNow (void)
{
	timeval tvDiff = DiffToNow ();
	return tvDiff.tv_sec;
}


//													
time_t Time::MinutesToNow (void)
{
	timeval tvDiff = DiffToNow ();
	return tvDiff.tv_sec / 60;
}


//													
String Time::sTimeToNow (void)
{
	timeval tvDiff = DiffToNow ();
	String sDiff;
	sDiff << tvDiff.tv_sec << "+s, " << tvDiff.tv_usec << "+us";
	return sDiff;
}


//													
String Time::sStartStopTime (void)
{
	String sElapsed;
	sElapsed << tv_Elapsed.tv_sec << ">s, " << tv_Elapsed.tv_usec << ">us";
	return sElapsed;
}


//													
String Time::sDateTime (void)
{
	time_t t = time (NULL);
	char zTime [201];
	#ifdef NDEBUG
	strftime (zTime, 200, "%Y-%m-%d_%H-%M-%S", localtime (&t));
	#else
	size_t iTimeLen = strftime (zTime, 200, "%Y-%m-%d_%H-%M-%S", localtime (&t));
	assert (iTimeLen < 200);
	#endif

	String sTime (zTime);
	return sTime;
}


//													
String Time::sDateTimeMicroSec (void)
{
	String sTime;

	timeval tv;
	timerclear (&tv);
	if (0 != gettimeofday (&tv, NULL))
	{
		cerr << "[ERROR]  Call to gettimeofday failed in Time::sDateTimeMicroSec" << endl;
		return sTime;
	}

	char zTime [201];
	#ifdef NDEBUG
	strftime (zTime, 200, "%Y-%m-%d_%H-%M-%S", localtime (&tv.tv_sec));
	#else
	size_t iTimeLen = strftime (zTime, 200, "%Y-%m-%d_%H-%M-%S", localtime (&tv.tv_sec));
	assert (iTimeLen < 200);
	#endif

	sTime << zTime << '-' << tv.tv_usec;
	return sTime;
}


//													
time_t Time::SecondsToCompletion (long _iCurrent, long _iEnd)
{
	time_t iSeconds = SecondsToNow ();
	time_t iRemaining = (time_t)((_iEnd - _iCurrent)
								* iSeconds / (double)_iCurrent);
	return iRemaining;
}


//													
String Time::sTimeToCompletion (long _iCurrent, long _iEnd)
{
	time_t iRemaining = SecondsToCompletion (_iCurrent, _iEnd);
	time_t iSeconds = iRemaining % 60;
	time_t iMinutes = (iRemaining / 60) % 60;
	time_t iHours = iRemaining / 3600;

	String sTime;
	sTime << iHours << "h:" << iMinutes << "m:" << iSeconds << 's';
	return sTime;
}



