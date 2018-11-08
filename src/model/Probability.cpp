#include "Probability.h"


//											
LogProbability::LogProbability (size_t _iSize)
{
	vec_LogProb.Reserve (_iSize);
}


LogProbability::~LogProbability (void)
{
}


//											
Probability::Probability (size_t _iSize)
{
	vec_Prob.Reserve (_iSize);
}

//											
Probability::Probability (const LogProbability& _rLogProb)
{
	size_t iSize = _rLogProb.vec_LogProb.Size ();
	if (0 == iSize)
		return;

	vec_Prob.Reserve (iSize);
	double dMaxLogProb = _rLogProb.vec_LogProb.Max ();
	double dSum = 0;
	for (size_t i = 0; i < iSize; ++ i)
	{
		double dProb = exp (_rLogProb.vec_LogProb [i] - dMaxLogProb);
		vec_Prob [i] = dProb;
		dSum += dProb;
	}

	for (size_t i = 0; i < iSize; ++ i)
		vec_Prob [i] /= dSum;
}



Probability::~Probability (void)
{
}



