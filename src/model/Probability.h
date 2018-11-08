#ifndef __PROB_LOGPROB__
#define __PROB_LOGPROB__

#include <nlp_vector.h>
#include <iostream>
using namespace std;


class LogProbability;
typedef Vector<double>	double_Vec_t;


//												
class Probability
{
	private:
		double_Vec_t	vec_Prob;

	public:
		Probability (const LogProbability& _rLogProb);
		Probability (size_t _iSize);
		~Probability (void);

		size_t Size (void)
		{ return vec_Prob.Size (); };
		double& operator[] (size_t _iIndex)
		{
			assert (_iIndex < vec_Prob.Size ());
			return vec_Prob [_iIndex];
		};
		void PrintToStream (ostream& _rStream) const
		{ _rStream << vec_Prob; };

		double* GetData (void)
		{ return vec_Prob; };
};

inline ostream& operator<< (ostream& _rStream, const Probability& _rProb)
{
	_rProb.PrintToStream (_rStream);
	return _rStream;
}



//												
class LogProbability
{
	friend class Probability;
	private:
		double_Vec_t	vec_LogProb;

	public:
		LogProbability (size_t _iSize);
		~LogProbability (void);

		size_t Size (void)
		{ return vec_LogProb.Size (); };
		double& operator[] (size_t _iIndex)
		{
			assert (_iIndex < vec_LogProb.Size ());
			return vec_LogProb [_iIndex];
		};
		// operator Probability (void) const;
		void PrintToStream (ostream& _rStream) const
		{ _rStream << vec_LogProb; };

		double* GetData (void)
		{ return vec_LogProb; };
};

inline ostream& operator<< (ostream& _rStream, const LogProbability& _rLogProb)
{
	_rLogProb.PrintToStream (_rStream);
	return _rStream;
}


#endif

