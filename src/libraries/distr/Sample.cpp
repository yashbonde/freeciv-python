#include "nlp_distr.h"
#include <algorithm>
#include <time.h>
using namespace std;

gsl_rng*	Random::p_GslRng;


//													
void Random::Init (void)
{
	gsl_rng_env_setup ();
	gsl_rng_default_seed = (unsigned long) time (NULL);
	p_GslRng = gsl_rng_alloc (gsl_rng_default);
}



//													
void Random::Destroy (void)
{
	assert ((gsl_rng*)0x1 != p_GslRng);
	gsl_rng_free (p_GslRng);
	p_GslRng = (gsl_rng*)0x1;
}


//													
void Sample::LogPdfToPdf (double* _pdLogPDF, double* _pdPDF, size_t _iSize, double _dTemperature)
{
	double dMaxLog = _pdLogPDF [0];
	for (size_t i = 1; i < _iSize; ++ i)
	{
		if (dMaxLog < _pdLogPDF [i])
			dMaxLog = _pdLogPDF [i];
	}

	double dSum = 0;
	for (size_t i = 0; i < _iSize; ++ i)
	{
		double dProb = exp ((_pdLogPDF [i] - dMaxLog) * _dTemperature);
		_pdPDF [i] = dProb;
		dSum += dProb;
	}

	for (size_t i = 0; i < _iSize; ++ i)
		_pdPDF [i] /= dSum;
}


//													
size_t Sample::Argmax (double* _pdLogPDF, size_t _iSize)
{
	double dMax = _pdLogPDF [0];
	for (size_t i = 1; i < _iSize; ++ i)
	{
		if (dMax >= _pdLogPDF [i])
			continue;
		dMax = _pdLogPDF [i];
	}

	double dPrev = 0;
	double dMaxCDF [_iSize];
	for (size_t i = 0; i < _iSize; ++ i)
	{
		dMaxCDF [i] = dPrev;
		if (dMax == _pdLogPDF [i])
			dMaxCDF [i] = 1 + dPrev;
		dPrev = dMaxCDF [i];
	}

	return SampleFromCDF (dMaxCDF, _iSize);
}


//													
size_t Sample::SampleFromLogPDF (double* _pdLogPDF, size_t _iSize, double _dTemperature)
{
	double pdCDF [_iSize];
	double dMaxLog = _pdLogPDF [0];
	for (size_t i = 1; i < _iSize; ++ i)
	{
		if (dMaxLog < _pdLogPDF [i])
			dMaxLog = _pdLogPDF [i];
	}

	for (size_t i = 0; i < _iSize; ++ i)
		pdCDF [i] = exp ((_pdLogPDF [i] - dMaxLog) * _dTemperature);

	for (size_t i = 1; i < _iSize; ++ i)
		pdCDF [i] += pdCDF [i - 1];

	return SampleFromCDF (pdCDF, _iSize);
}


//													
size_t Sample::SampleFromPDF (double* _pdPDF, size_t _iSize)
{
	double pdCDF [_iSize];
	for (size_t i = 0; i < _iSize; ++ i)
		pdCDF [i] = _pdPDF [i];
	for (size_t i = 1; i < _iSize; ++ i)
		pdCDF [i] += pdCDF [i - 1];

	return SampleFromCDF (pdCDF, _iSize);
}


//													
size_t Sample::SampleFromCDF (double* _pdCDF, size_t _iSize)
{
	double dMaxCDF = _pdCDF [_iSize - 1];
	double dUniform = dMaxCDF * gsl_rng_uniform (Random::p_GslRng);
	if (dUniform == dMaxCDF)
		return _iSize - 1;

	double* pBegin = _pdCDF;
	double* pEnd = pBegin + _iSize;
	double* pSample =  std::upper_bound (pBegin, pEnd, dUniform);
	while (0 == *pSample)
	{
		++ pSample;
		if (pSample >= pEnd)
			abort ();
	}
	return (pSample - pBegin);
}


//													
int Sample::SampleUniformCategorical (int _iCategories)
{
	double dUniform = gsl_rng_uniform (Random::p_GslRng) * _iCategories;
	return (int) floor (dUniform);
}


//													
double Sample::SampleUniform (void)
{
	return gsl_rng_uniform (Random::p_GslRng);
}


//													
double Sample::SampleGaussian (double _dMean, double _dSigma)
{
	return _dMean + gsl_ran_gaussian (Random::p_GslRng, _dSigma);
}





