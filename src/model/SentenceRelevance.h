#ifndef __SENTENCE_RELEVANCE__
#define __SENTENCE_RELEVANCE__

#include "Text.h"
#include "LogLinearModel.h"
#include "Interface.h"


//												
class SentenceRelevance
{
	private:
		LogLinearModel		o_Model;
		TextInterface*		p_TextInterface;
		double				d_Epsilon;
		double				d_SoftmaxBeta;
		bool				b_EpsilonGreedy;
		double				d_ForcedNullProbability;
		long long			ll_TotalCalls;
		long long			ll_NullSentenceReturns;
		pthread_rwlock_t	rwl_Weights;


		inline int ComputeFeatureVectors (Action& _rAction,
										  FilteredSentences& _rFilteredSentences,
										  Feature_vec_t& _rvecFeatureVectors);
	public:
		SentenceRelevance (void);
		~SentenceRelevance (void);

		void Init (TextInterface& _rInterface);

		Sentence* SelectRelevantSentence (Sample& _rSample,
										  Action& _rAction,
										  FilteredSentences& _rFilteredSentences,
										  bool& _bIsArgmax);
		void UpdateParameters (double _dReward, 
							   Action& _rAction,
							   FilteredSentences& _rFilteredSentences);

		void SaveWeights (const char* _zName);
		void ResetWeights (void)
		{ o_Model.ResetWeights (); };

		double NullSentenceRatio (void)
		{
			if (0 == ll_TotalCalls)
				return 0;
			return ll_NullSentenceReturns / (double) ll_TotalCalls;
		};
};



#endif
