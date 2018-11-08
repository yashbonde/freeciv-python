#ifndef __HIDDEN_STATE_VAR__
#define __HIDDEN_STATE_VAR__

#include <vector>
#include "Text.h"
#include "LogLinearModel.h"
#include "Interface.h"
#include "HiddenState.h"
using namespace std;


typedef vector <HiddenState>	HiddenState_vec_t;


//												
class HiddenStateVariable
{
	private:
		LogLinearModel		o_Model;
		double				d_Epsilon;
		int					i_HiddenStates;
		HiddenState_vec_t	vec_HiddenStates;
		pthread_rwlock_t	rwl_Weights;


		inline int ComputeFeatureVectors (Action& _rAction,
										  Feature_vec_t& _rvecFeatureVectors);
	public:
		HiddenStateVariable (void);
		~HiddenStateVariable (void);

		void Init (void);

		HiddenState* SelectRelevantState (Sample& _rSample, Action& _rAction);
		void UpdateParameters (double _dReward, Action& _rAction);
		void SaveWeights (const char* _zName);
		void ResetWeights (void)
		{ o_Model.ResetWeights (); };
};



#endif
