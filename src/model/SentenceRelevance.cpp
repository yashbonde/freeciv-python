#include "SentenceRelevance.h"
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_distr.h>


//													
SentenceRelevance::SentenceRelevance (void)
{
	pthread_rwlock_init (&rwl_Weights, NULL);
	ll_TotalCalls = 0;
	ll_NullSentenceReturns = 0;
}


//													
SentenceRelevance::~SentenceRelevance (void)
{
	pthread_rwlock_destroy (&rwl_Weights);
}


//													
void SentenceRelevance::Init (TextInterface& _rInterface)
{
	p_TextInterface = &_rInterface;
	if (1 == (int)(config)"srm:load_weights")
		RelevanceModelFeatures::LoadIndices ("srm");
	o_Model.Init ("srm");
	d_Epsilon = (config)"srm:epsilon";
	d_SoftmaxBeta = (config)"srm:softmax_beta";
	b_EpsilonGreedy = (1 == (int)(config)"srm:use_epsilon_greedy");
	d_ForcedNullProbability = (config)"srm:forced_null_probability";
	if (true == b_EpsilonGreedy)
		cout << "Sentence relevance : using epsilon greedy" << endl;
	else
		cout << "Sentence relevance : using softmax" << endl;
}


//													
inline int SentenceRelevance::ComputeFeatureVectors (Action& _rAction,
													 FilteredSentences& _rFilteredSentences,
													 Feature_vec_t& _rvecFeatureVectors)
{
	size_t iSentences = _rFilteredSentences.vec_Sentences.Size ();
	_rvecFeatureVectors.reserve (iSentences);

	int iUnitType;
	iUnitType = _rAction.p_Agent->i_UnitType;
	int iActionType;
	iActionType = _rAction.i_CommandType;

	// for each sentence, compute features	
	int iSentenceIndex = 0;
	int iSelectedSentenceIndex = -1;
	for (size_t i = 0; i < iSentences; ++i)
	{
		Sentence* pSentence = _rFilteredSentences.vec_Sentences [i];
		// _rAction.p_SelectedSentence might be a copy of the 	
		// original sentence object, so we can't directly		
		// compare pointers to check if it's the same as		
		// pSentence.  Regardless of whether it's a copy or not,
		// p_OriginalSentence is guaranteed to refer to the 	
		// original sentence object...							
		if (NULL != _rAction.p_SelectedSentence)
		{
			if (pSentence == _rAction.p_SelectedSentence->p_OriginalSentence)
				iSelectedSentenceIndex = iSentenceIndex;
		}

		RelevanceModelFeatures* pFeatures = new RelevanceModelFeatures;

		size_t iFeatureCount = pSentence->o_Features.Size () + 
							   2 * pSentence->i_Words + 2;
		assert (pSentence->o_Features.Size () > 0);

		pFeatures->SetSize (iFeatureCount);

		// words in sentence	
		pFeatures->Set (pSentence->o_Features);

		// heuristic label match score of sentence	
		double dHeuristicScore =
			p_TextInterface->GetSentenceScore (_rAction.dq_FeatureLabels,
												_rAction.p_Agent->dq_FeatureLabels,
												pSentence);
		pFeatures->SetX (0, dHeuristicScore);

		// words * unit type	
		pFeatures->SetBagOfWords (iUnitType, pSentence->p_WordIndices, pSentence->i_Words);

		// words * action type	
		pFeatures->SetBagOfWords (iActionType, pSentence->p_WordIndices, pSentence->i_Words);

		// words with state label overlap	

		_rvecFeatureVectors.push_back (pFeatures);
		++ iSentenceIndex;
	}

	return iSelectedSentenceIndex;
}


//													
Sentence* SentenceRelevance::SelectRelevantSentence (Sample& _rSample,
													 Action& _rAction, 
													 FilteredSentences& _rFilteredSentences,
													 bool& _bIsArgmax)
{
	if (_rSample.SampleUniform () < d_ForcedNullProbability)
		return p_TextInterface->GetNullSentence ();

	pthread_rwlock_rdlock (&rwl_Weights);

	Feature_vec_t vecFeatureVectors;
	ComputeFeatureVectors (_rAction, _rFilteredSentences, vecFeatureVectors);

	// compute log prob ...					
	size_t iSentences = _rFilteredSentences.vec_Sentences.Size ();
	LogProbability oLogProb (iSentences);
	for (size_t i = 0; i < iSentences; ++ i)
		oLogProb [i] = o_Model.ComputeLogProb (*vecFeatureVectors [i]);


	// select sentence ...					
	size_t iSelectedSentence = 0;
	if (true == b_EpsilonGreedy)
	{
		if (_rSample.SampleUniform () < d_Epsilon)
			iSelectedSentence = _rSample.SampleUniformCategorical (iSentences);
		else
			iSelectedSentence = _rSample.Argmax (oLogProb.GetData (), oLogProb.Size ());
	}
	else
		iSelectedSentence = _rSample.SampleFromLogPDF (oLogProb.GetData (), oLogProb.Size (), d_SoftmaxBeta);
	
	assert (iSelectedSentence < iSentences);

	// check if the selected sentence is the argmax	
	// this is for external evaluation reasons ...	
	size_t iArgmax = _rSample.Argmax (oLogProb.GetData (), oLogProb.Size ());
	_bIsArgmax = (iArgmax == iSelectedSentence);

	// cleanup...
	ITERATE (Feature_vec_t, vecFeatureVectors, ite)
		delete *ite;

	//										
	pthread_rwlock_unlock (&rwl_Weights);

	Sentence* pReturn = _rFilteredSentences.vec_Sentences [iSelectedSentence];
	if (true == pReturn->b_IsNullSentence)
		++ ll_NullSentenceReturns;
	++ ll_TotalCalls;

	return pReturn;
}


//													
void SentenceRelevance::UpdateParameters (double _dReward,
										  Action& _rAction,
										  FilteredSentences& _rFilteredSentences)
{
	// Feature_vec_t& vecFeatureVectors = _rAction.vec_SentenceRelevanceFeatures;
	Feature_vec_t vecFeatureVectors;
	int iSelectedSentenceIndex = ComputeFeatureVectors (_rAction,
														_rFilteredSentences,
														vecFeatureVectors);
	size_t iSentences = _rFilteredSentences.vec_Sentences.Size ();

	#ifndef NDEBUG
	size_t iVectors = vecFeatureVectors.size ();
	if (iVectors != iSentences)
		cout << "vector != sentences : " << iVectors << " != " << iSentences << endl;
	#endif

	// compute log prob ...					
	LogProbability oLogProb (iSentences);
	for (size_t i = 0; i < iSentences; ++ i)
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
	assert (-1 != iSelectedSentenceIndex);
	Features* pSelectedFeatures = vecFeatureVectors [iSelectedSentenceIndex];
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
void SentenceRelevance::SaveWeights (const char* _zName)
{
	RelevanceModelFeatures::SaveIndices (_zName);
	o_Model.SaveWeights (_zName);
}
