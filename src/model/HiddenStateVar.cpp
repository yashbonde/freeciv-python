#include "HiddenStateVar.h"
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_distr.h>


//													
HiddenStateVariable::HiddenStateVariable (void)
{
	pthread_rwlock_init (&rwl_Weights, NULL);
}


//													
HiddenStateVariable::~HiddenStateVariable (void)
{
	pthread_rwlock_destroy (&rwl_Weights);
}


//													
void HiddenStateVariable::Init (void)
{
	i_HiddenStates = (config)"hsv:hidden_state_count";
	if (1 == (int)(config)"hsv:load_weights")
		RelevanceModelFeatures::LoadIndices ("hsv");
	o_Model.Init ("hsv");
	d_Epsilon = (config)"hsv:epsilon";

	vec_HiddenStates.resize (i_HiddenStates);
	for (int h = 0; h < i_HiddenStates; ++ h)
	{
		String sHidden;
		sHidden << "__HIDDEN__" << h;
		vec_HiddenStates [h].i_FeatureId
				= RelevanceModelFeatures::GetWordIndex (sHidden);
	}
}


//													
inline int HiddenStateVariable::ComputeFeatureVectors (Action& _rAction,
													   Feature_vec_t& _rvecFeatureVectors)
{
	_rvecFeatureVectors.reserve (i_HiddenStates);

	int iUnitType;
	iUnitType = _rAction.p_Agent->i_UnitType;
	int iActionType;
	iActionType = _rAction.i_CommandType;

	// for each sentence, compute features	
	int iSelectedHiddenState = -1;
	for (int i = 0; i < i_HiddenStates; ++i)
	{
		HiddenState* pHiddenState = &vec_HiddenStates [i];
		if (NULL != _rAction.p_SelectedHiddenState)
		{
			if (pHiddenState == _rAction.p_SelectedHiddenState)
				iSelectedHiddenState = i;
		}

		RelevanceModelFeatures* pFeatures = new RelevanceModelFeatures;
		pFeatures->SetSize (3);

		// words in sentence	
		pFeatures->SetPrefixFeature (0, pHiddenState->i_FeatureId);

		// words * unit type	
		pFeatures->SetPrefixFeature (iUnitType, pHiddenState->i_FeatureId);

		// words * action type	
		pFeatures->SetPrefixFeature (iActionType, pHiddenState->i_FeatureId);

		_rvecFeatureVectors.push_back (pFeatures);
	}

	return iSelectedHiddenState;
}


//													
HiddenState* HiddenStateVariable::SelectRelevantState (Sample& _rSample, Action& _rAction)
{
	pthread_rwlock_rdlock (&rwl_Weights);

	Feature_vec_t vecFeatureVectors;
	ComputeFeatureVectors (_rAction, vecFeatureVectors);

	// compute log prob ...					
	LogProbability oLogProb (i_HiddenStates);
	for (int i = 0; i < i_HiddenStates; ++ i)
		oLogProb [i] = o_Model.ComputeLogProb (*vecFeatureVectors [i]);


	// select sentence ...					
	size_t iSelectedHiddenState = 0;
	if (_rSample.SampleUniform () < d_Epsilon)
		iSelectedHiddenState = _rSample.SampleUniformCategorical (i_HiddenStates);
	else
		iSelectedHiddenState = _rSample.Argmax (oLogProb.GetData (), oLogProb.Size ());
	assert (iSelectedHiddenState < (size_t)i_HiddenStates);

	// cleanup...
	ITERATE (Feature_vec_t, vecFeatureVectors, ite)
		delete *ite;

	//										
	pthread_rwlock_unlock (&rwl_Weights);

	return &vec_HiddenStates [iSelectedHiddenState];
}


//													
void HiddenStateVariable::UpdateParameters (double _dReward, Action& _rAction)
{
	// Feature_vec_t& vecFeatureVectors = _rAction.vec_HiddenStateVariableFeatures;
	Feature_vec_t vecFeatureVectors;
	int iSelectedHiddenState = ComputeFeatureVectors (_rAction, vecFeatureVectors);

	#ifndef NDEBUG
	int iVectors = (int)vecFeatureVectors.size ();
	if (iVectors != i_HiddenStates)
		cout << "vector != hidden states : " << iVectors << " != " << i_HiddenStates << endl;
	#endif

	// compute log prob ...					
	LogProbability oLogProb (i_HiddenStates);
	for (int i = 0; i < i_HiddenStates; ++ i)
		oLogProb [i] = o_Model.ComputeLogProb (*vecFeatureVectors [i]);

	// compute expected feature values 		
	// for later parameter updates ...		
	double_Vec_t vecTrace;
	int iFeatureCount = 1 + RelevanceModelFeatures::MaxIndex ();
	o_Model.InitializeFeatureExpectationVector (vecTrace, iFeatureCount);
	o_Model.ComputeNegativeFeatureExpectation (oLogProb, 
											   vecFeatureVectors,
											   vecTrace);

	// update trace with feature of selected
	// action ...							
	assert (-1 != iSelectedHiddenState);
	Features* pSelectedFeatures = vecFeatureVectors [iSelectedHiddenState];
	for (int i = 0; i < pSelectedFeatures->Size (); ++ i)
		vecTrace [pSelectedFeatures->Index (i)] += pSelectedFeatures->Feature (i);

	// cleanup...
	ITERATE (Feature_vec_t, vecFeatureVectors, ite)
		delete *ite;

	pthread_rwlock_wrlock (&rwl_Weights);
	o_Model.UpdateWeights (_dReward, vecTrace);
	pthread_rwlock_unlock (&rwl_Weights);
}


//													
void HiddenStateVariable::SaveWeights (const char* _zName)
{
	RelevanceModelFeatures::SaveIndices (_zName);
	o_Model.SaveWeights (_zName);
}
