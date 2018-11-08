#ifndef __NLP_SAMPLE_DISTRIBUTION__
#define __NLP_SAMPLE_DISTRIBUTION__

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <nlp_vector.h>
#include <deque>
using namespace std;

typedef deque <double>	double_dq_t;


//---------------------------------------------------
class Sample;
class Gaussian;
class Random
{
	friend class Sample;
	friend class Gaussian;

	private:
		static gsl_rng*	p_GslRng;

	public:
		static void Init (void);
		static void Destroy (void);
};


//---------------------------------------------------
class Sample
{
	public:
		static void LogPdfToPdf (double* _pdLogPDF, double* _pdPDF, size_t _iSize, double _dTemperature);

		static size_t Argmax (double* _pdLogPDF, size_t _iSize);
		static size_t SampleFromLogPDF (double* _pdLogPDF, size_t _iSize, double _dTemperature);
		static size_t SampleFromPDF (double* _pdPDF, size_t _iSize);
		static size_t SampleFromCDF (double* _pdCDF, size_t _iSize);

		static size_t Argmax (Vector<double>& _rLogPDF)
		{ return Argmax ((double*)_rLogPDF, _rLogPDF.Size ()); };
		static size_t SampleFromLogPDF (Vector<double>& _rLogPDF, double _dTemperature)
		{ return SampleFromLogPDF ((double*)_rLogPDF, _rLogPDF.Size (), _dTemperature); };
		static size_t SampleFromPDF (Vector<double> _rPDF, size_t _iSize)
		{ return SampleFromPDF ((double*)_rPDF, _rPDF.Size ()); };
		static size_t SampleFromCDF (Vector<double> _rCDF, size_t _iSize)
		{ return SampleFromCDF ((double*)_rCDF, _rCDF.Size ()); };


		static int SampleUniformCategorical (int _iCategories);
		static double SampleUniform (void);

		static double SampleGaussian (double _dMean, double _dSigma);
};


//---------------------------------------------------
class Gaussian
{
	public:
		double	d_Mean;
		double	d_Sigma;

		Gaussian (void);
		Gaussian (double _dMean, double _dSigma);
		~Gaussian (void);

		void Init (double _dMean, double _dSigma);
		double Sample (void);
		void EstimateParams (double_dq_t _dqSamples);
};



#endif
