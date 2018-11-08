#ifndef __LOG_LINEAR_MODEL__
#define __LOG_LINEAR_MODEL__

#include <vector>
#include <pthread.h>
#include <nlp_string.h>
#include <nlp_vector.h>
#include "Feature.h"
#include "Probability.h"
using namespace std;

typedef Vector<double>		double_Vec_t;
typedef vector<Features*>	Features_vec_t;


//												
class LogLinearModel
{
	private:
		double_Vec_t		vec_Weights;
		int					i_Features;
		double				d_LearningRate;
		double				d_RegularizationFactor;
		// pthread_rwlock_t	rwl_Weights;

		void CheckForNan (const char* _zId);

	public:
		LogLinearModel (void);
		~LogLinearModel (void);

		void Init (String _sName);

		double ComputeLogProb (Features& _rFeatures);
		void InitializeFeatureExpectationVector (double_Vec_t& _rvecExpectedFeatures,
												 int _iFeatureCount);
		void ComputeNegativeFeatureExpectation (LogProbability& _rLogProb,
												Features_vec_t& _rvecFeatures, 
												double_Vec_t& _rvecExpectedFeatures);
		void UpdateWeights (double _dReward, 
							double_Vec_t& _rvecTraces);

		void ResetWeights (void);
		bool SaveWeights (const char* _zName);
		bool LoadWeights (const char* _zName);
};


#endif
