#include "nlp_distr.h"
#include <nlp_macros.h>
#include <algorithm>
#include <time.h>
using namespace std;


//										
Gaussian::Gaussian (void)
{
	d_Mean = NAN;
	d_Sigma = NAN;
}


//										
Gaussian::Gaussian (double _dMean, double _dSigma)
{
	d_Mean = _dMean;
	d_Sigma = _dSigma;
}


//										
Gaussian::~Gaussian (void)
{
}


//										
void Gaussian::Init (double _dMean, double _dSigma)
{
	d_Mean = _dMean;
	d_Sigma = _dSigma;
}


//										
double Gaussian::Sample (void)
{
	return d_Mean + gsl_ran_gaussian (Random::p_GslRng, d_Sigma);
}


//										
void Gaussian::EstimateParams (double_dq_t _dqSamples)
{
	if (true == _dqSamples.empty ())
	{
		cout << "[ERROR] Gaussian::EstimateParams called with an empty sample list"
			 << endl;
		return;
	}

	double dCount = _dqSamples.size ();

	// mean 	
	{
		double dSum = 0;
		ITERATE (double_dq_t, _dqSamples, ite)
			dSum += *ite;
		d_Mean = dSum / dCount;
	}

	// sigma	
	{
		double dSum = 0;
		ITERATE (double_dq_t, _dqSamples, ite)
			dSum += (*ite - d_Mean) * (*ite - d_Mean);
		d_Sigma = sqrt (dSum / dCount);
	}
}





