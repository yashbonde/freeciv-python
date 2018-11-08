#ifndef __OPERATOR_WORDS__
#define __OPERATOR_WORDS__

#include "Text.h"
#include "LogLinearModel.h"
// #include "Interface.h"
#include <nlp_string.h>


//												
class OperatorWords
{
	private:
		LogLinearModel		o_Model;
		TextInterface*		p_TextInterface;
		double				d_Epsilon;
		double				d_SoftmaxBeta;
		bool				b_EpsilonGreedy;
		String_set_t		set_StateLabels;
		String_set_t		set_ActionLabels;
		String_set_t		set_UnitLabels;
		pthread_rwlock_t	rwl_Weights;
		pthread_mutex_t		mtx_Labels;

		inline int ComputeFeatureVectors (int _iWordLoc, 
										  OperatorLabeledSentence& _rSentence,
										  Feature_vec_t& _rvecFeatureVectors);

	public:
		OperatorWords (void);
		~OperatorWords (void);

		void Init (TextInterface& _rInterface);

		void AddLabels (String_dq_t& _rdqState, 
						String_dq_t& _rdqUnit, 
						String_dq_t& _rdqAction);

		void PredictOperatorWords (Sample& _rSample, OperatorLabeledSentence& _rSentence);
		void UpdateParameters (double _dReward, OperatorLabeledSentence& _rLabeledSentence);
		void SaveWeights (const char* _zName);
		void ResetWeights (void)
		{ o_Model.ResetWeights (); };
};



#endif
