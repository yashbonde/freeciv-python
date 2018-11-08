#ifndef __NLP_TIME__
#define __NLP_TIME__
#include <sys/time.h>
#include <time.h>
#include <nlp_string.h>


class Time
{
	private:
		timeval tv_Start;
		timeval tv_Elapsed;

		timeval DiffToNow (void);

	public:
		static String CurrentTime (void);

		Time (void);
		void Reset (void);

		void StartTimer (void);
		void StopTimer (void);

		time_t SecondsToNow (void);
		time_t MinutesToNow (void);

		time_t SecondsToCompletion (long _iCurrent, long _iEnd);
		String sTimeToCompletion (long _iCurrent, long _iEnd);

		String sTimeToNow (void);
		String sStartStopTime (void);
		static String sDateTime (void);
		static String sDateTimeMicroSec (void);
};


#endif
