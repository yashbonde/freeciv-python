#include "OperatorWords.h"
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_distr.h>
#include "FeatureSpecialization.h"


//													
OperatorWords::OperatorWords (void)
{
	pthread_rwlock_init (&rwl_Weights, NULL);
	pthread_mutex_init (&mtx_Labels, NULL);
}


//													
OperatorWords::~OperatorWords (void)
{
	pthread_rwlock_destroy (&rwl_Weights);
	pthread_mutex_destroy (&mtx_Labels);
}


//													
void OperatorWords::Init (TextInterface& _rInterface)
{
	p_TextInterface = &_rInterface;
	o_Model.Init ("opw");
	d_Epsilon = (config)"opw:epsilon";
	d_SoftmaxBeta = (config)"opw:softmax_beta";
	b_EpsilonGreedy = (1 == (int)(config)"opw:use_epsilon_greedy");
	if (true == b_EpsilonGreedy)
		cout << "Predicates : using epsilon greedy" << endl;
	else
		cout << "Predicates : using softmax" << endl;
}


//													
void OperatorWords::AddLabels (String_dq_t& _rdqState, 
								String_dq_t& _rdqUnit, 
								String_dq_t& _rdqAction)
{
	pthread_mutex_lock (&mtx_Labels);

	ITERATE (String_dq_t, _rdqState, ite)
		set_StateLabels.insert (*ite);
	ITERATE (String_dq_t, _rdqUnit, ite)
		set_UnitLabels.insert (*ite);
	ITERATE (String_dq_t, _rdqAction, ite)
		set_ActionLabels.insert (*ite);

	pthread_mutex_unlock (&mtx_Labels);
}


//													
inline int OperatorWords::ComputeFeatureVectors (int _iWordLoc, 
												 OperatorLabeledSentence& _rSentence,
												 Feature_vec_t& _rvecFeatureVectors)
{
	_rvecFeatureVectors.reserve (3);

	// we offset the parent label here because 0 is used to	
	// indicate the absense of a parent node.				
	int iParentLabel = 1 + _rSentence.GetParentLabel (_iWordLoc);
	size_t iParentLabelOffset = (3 * _rSentence.i_Words) * iParentLabel;

	String& sWord = _rSentence.p_OriginalSentence->dq_Words [_iWordLoc];
	int iIsStateLabel = (set_StateLabels.end () != set_StateLabels.find (sWord))? 1 : 0;
	int iIsUnitLabel = (set_UnitLabels.end () != set_UnitLabels.find (sWord))? 1 : 0;
	int iIsActionLabel = (set_ActionLabels.end () != set_ActionLabels.find (sWord))? 1 : 0;

	// for each label option, compute features	
	for (size_t l = 0; l < 3; ++l)
	{
		OperatorModelFeatures* pFeatures = new OperatorModelFeatures;

		size_t iFeatureCount = 11;
		pFeatures->SetSize (iFeatureCount);

		// label * current word			
		pFeatures->Set (_rSentence.p_LabelXWord [3 * _iWordLoc + l], 1);

		// label * current word tag 	
		pFeatures->Set (_rSentence.p_LabelXTag [3 * _iWordLoc + l], 1);

		// label * dependency type 		
		pFeatures->Set (_rSentence.p_LabelXDependencyType [3 * _iWordLoc + l], 1);

		// label * parent word			
		pFeatures->Set (_rSentence.p_LabelXParentWord [3 * _iWordLoc + l], 1);

		// label * parent word tag		
		pFeatures->Set (_rSentence.p_LabelXParentTag [3 * _iWordLoc + l], 1);

		// label * is-leaf				
		pFeatures->Set (_rSentence.p_LabelXIsLeaf [3 * _iWordLoc + l], 1);

		// label * parent label			
		pFeatures->Set (_rSentence.p_LabelXParentLabel [iParentLabelOffset + 3*_iWordLoc + l], 1);

		// label * heuristic label		
		pFeatures->Set (_rSentence.p_LabelXHeuristicLabel [3 * _iWordLoc + l], 1);

		// label * state,unit,action labels	
		pFeatures->Set (_rSentence.p_LabelXIsStateLabel [3 * iIsStateLabel + l], 1);
		pFeatures->Set (_rSentence.p_LabelXIsUnitLabel [3 * iIsUnitLabel + l], 1);
		pFeatures->Set (_rSentence.p_LabelXIsActionLabel [3 * iIsActionLabel + l], 1);

		_rvecFeatureVectors.push_back (pFeatures);
	}

	return _rSentence.p_Labels [_iWordLoc];
}


//													
void OperatorWords::PredictOperatorWords (Sample& _rSample,
										  OperatorLabeledSentence& _rSentence)
{
	pthread_rwlock_rdlock (&rwl_Weights);

	for (int w = 0; w < _rSentence.i_Words; ++ w)
	{
		// we predict word operator labels in	
		// dependency order...					
		int d = _rSentence.p_DependencyOrder [w];

		Feature_vec_t vecFeatureVectors;
		ComputeFeatureVectors (d, _rSentence, vecFeatureVectors);

		// compute log prob ...					
		LogProbability oLogProb (3);
		for (size_t i = 0; i < 3; ++ i)
			oLogProb [i] = o_Model.ComputeLogProb (*vecFeatureVectors [i]);


		// select sentence ...					
		size_t iWordLabel = 0;
		if (true == b_EpsilonGreedy)
		{
			if (_rSample.SampleUniform () < d_Epsilon)
				iWordLabel = _rSample.SampleUniformCategorical (3);
			else
				iWordLabel = _rSample.Argmax (oLogProb.GetData (), 3);
		}
		else
			iWordLabel = _rSample.SampleFromLogPDF (oLogProb.GetData (), 3, d_SoftmaxBeta);
		assert (iWordLabel < 3);

		// label Sentence with selected word label...
		_rSentence.SetWordLabel (d, iWordLabel);

		// cleanup...
		ITERATE (Feature_vec_t, vecFeatureVectors, ite)
			delete *ite;
	}

	//										
	pthread_rwlock_unlock (&rwl_Weights);
}


//													
void OperatorWords::UpdateParameters (double _dReward, 
									  OperatorLabeledSentence& _rLabeledSentence)
{
	double_Vec_t vecTrace;
	int iFeatureCount = 1 + OperatorModelFeatures::MaxIndex ();
	o_Model.InitializeFeatureExpectationVector (vecTrace, iFeatureCount);

	for (int w = 0; w < _rLabeledSentence.i_Words; ++ w)
	{
		Feature_vec_t vecFeatureVectors;
		int iSelectedLabel = ComputeFeatureVectors (w,
													_rLabeledSentence,
													vecFeatureVectors);
		size_t iVectors = vecFeatureVectors.size ();
		if (iVectors != 3)
			cout << "vector != label options : " << iVectors << " != 3" << endl;

		// compute log prob ...					
		LogProbability oLogProb (3);
		for (size_t i = 0; i < 3; ++ i)
			oLogProb [i] = o_Model.ComputeLogProb (*vecFeatureVectors [i]);

		// compute expected feature values 		
		// for later parameter updates ...		
		o_Model.ComputeNegativeFeatureExpectation (oLogProb, 
												   vecFeatureVectors,
												   vecTrace);

		// update trace with feature of selected
		// action ...							
		assert (-1 != iSelectedLabel);
		Features* pSelectedFeatures = vecFeatureVectors [iSelectedLabel];
		for (int i = 0; i < pSelectedFeatures->Size (); ++ i)
			vecTrace [pSelectedFeatures->Index (i)] += pSelectedFeatures->Feature (i);

		// cleanup...
		ITERATE (Feature_vec_t, vecFeatureVectors, ite)
			delete *ite;
	}

	pthread_rwlock_wrlock (&rwl_Weights);
	o_Model.UpdateWeights (_dReward, vecTrace);
	pthread_rwlock_unlock (&rwl_Weights);
}


//													
void OperatorWords::SaveWeights (const char* _zName)
{
	o_Model.SaveWeights (_zName);
}
