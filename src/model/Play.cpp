#include <nlp_config.h>
#include <nlp_macros.h>
#include <nlp_filesystem.h>
#include <omp.h>
#include "Play.h"
#include "Interface.h"
#include "learner_comms.h"


//												
Play::Play (void)
{
	// pfn_ComputeHeuristicScores = NULL;
	// pfn_SelectBestCommand = NULL;
	pfn_SampleAction = NULL;

	f_HeuristicBiasProbability = 0;
	f_HeuristicBiasProbabilityMin = 0;
	f_HeuristicBiasProbabilityStep = 0;

	d_NewActionBias = 0;

	b_UsingSentenceRelevanceModel = false;
	b_UseLabelOverlapSentenceSelection = false;
	b_UseOperatorFeatures = false;
	b_UseOperatorFeaturesOnly = false;
	b_UseOperatorWordModel = false;
	i_QfnUpdateLocalizationDepth = 0;
	b_PerPlyApproximation = false;
	b_ResetAllModelsEndOfPly = false;
	b_UseBackpropUpdate = false;

	pthread_mutex_init (&mtx_SentenceHit, NULL);
}


//												
Play::~Play (void)
{
	pthread_mutex_destroy (&mtx_SentenceHit);
}


//												
bool Play::Init (void)
{
	i_BiasContinue = (config)"bias_continue";
	d_NewActionBias = (config)"new_action_bias";
	b_UseBackpropUpdate = (1 == (int)(config)"use_backpropagation_update");
	cout << "   backprop update : "
		 << ((true == b_UseBackpropUpdate)? "yes" : "no")
		 << endl;

	b_UseHiddenStateModel = (1 == (int)(config)"use_hidden_state_model");
	if (true == b_UseHiddenStateModel)
		o_HiddenStateModel.Init ();

	//						
	String sSamplingMethod = (config)"action_sampling_method";
	cout << "action sampling : " << sSamplingMethod << endl;

	//										
	if ("uniform" == sSamplingMethod)
		pfn_SampleAction = &Play::SampleAction_Uniform;

	//										
	else if ("estimated-prior" == sSamplingMethod)
		pfn_SampleAction = &Play::SampleAction_EstimatedPrior;

	//										
	else if ("text" == sSamplingMethod)
		pfn_SampleAction = &Play::SampleAction_TextInfo;
	
	//										
	else if ("text-and-estimated-prior" == sSamplingMethod)
		pfn_SampleAction = &Play::SampleAction_TextAndEstimatedPrior;

	//										
	else if ("linear-qfn-approx" == sSamplingMethod)
	{
		pfn_SampleAction = &Play::SampleAction_QFnApprox;
		o_LinearFunctionApprox.Init ("lqfn");
		i_QfnUpdateLocalizationDepth = (config)"qfn_update_localization_depth";
		b_PerPlyApproximation = (1 == (int)(config)"per_ply_approximation");
		b_ResetAllModelsEndOfPly = (1 == (int)(config)"reset_all_models_end_of_ply");
		b_OnlineUpdate = (1 == (int)(config)"online_update");
	}
	//										
	else if (("linear-qfn-approx-with-text" == sSamplingMethod) ||
			("linear-qfn-approx-with-text-predicates" == sSamplingMethod))
	{
		pfn_SampleAction = &Play::SampleAction_QFnApproxWithText;
		o_LinearFunctionApprox.Init ("lqfn");
		o_RelevanceModel.Init (o_TextInterface);
		Agent::p_TextInterface = &o_TextInterface;

		f_HeuristicBiasProbability = (config)"heuristic_bias_probability_start";
		f_HeuristicBiasProbabilityMin = (config)"heuristic_bias_probability_min";
		int iHeuristicBiasSteps = (config)"heuristic_bias_steps";
		f_HeuristicBiasProbabilityStep
			= (f_HeuristicBiasProbability - f_HeuristicBiasProbabilityMin)
			   / (float) iHeuristicBiasSteps;

		b_UseLabelOverlapSentenceSelection
				= (1 == (int)(config)"use_label_overlap_sentence_selection");
		b_UsingSentenceRelevanceModel
				= (false == b_UseLabelOverlapSentenceSelection);

		b_UseOperatorFeatures
				= (1 == (int)(config)"use_operator_features");
		b_UseOperatorFeaturesOnly
				= (1 == (int)(config)"use_operator_features_only");
		b_UseOperatorWordModel
				= ("linear-qfn-approx-with-text-predicates" == sSamplingMethod);
		if (true == b_UseOperatorWordModel)
			o_OperatorWordsModel.Init (o_TextInterface);

		i_QfnUpdateLocalizationDepth = (config)"qfn_update_localization_depth";
		b_PerPlyApproximation = (1 == (int)(config)"per_ply_approximation");
		b_ResetAllModelsEndOfPly = (1 == (int)(config)"reset_all_models_end_of_ply");
		b_OnlineUpdate = (1 == (int)(config)"online_update");
	}
	//										
	else if ("mcts-uct" == sSamplingMethod)
	{
		cout << "MCTS-UCT code is out of date, needs minor updates." << endl;
		abort ();

		/*
		pfn_SampleAction = &Play::SampleAction_MCTS_UCT;

		d_ExplorationBonusRatio = (config)"exploration_bonus_ratio";
		cout << "exploration bonus ratio : " << d_ExplorationBonusRatio << endl;
		String sHeuristic = (config)"mcts-uct_default_heuristic";
		cout << "default heuristic : " << sHeuristic << endl;

		if ("uniform" == sHeuristic)
			pfn_ComputeHeuristicScores = &Play::ComputeScore_Uniform;
		else if ("estimated-prior" == sHeuristic)
			pfn_ComputeHeuristicScores = &Play::ComputeScore_EstimatedPrior;
		else if ("text" == sHeuristic)
			pfn_ComputeHeuristicScores = &Play::ComputeScore_TextInfo;
		else if ("text-and-estimated-prior" == sHeuristic)
			pfn_ComputeHeuristicScores = &Play::ComputeScore_TextAndEstimatedPrior;
		else if ("linear-qfn-approx" == sHeuristic)
		{
			o_LinearFunctionApprox.Init ("lqfn");
			pfn_ComputeHeuristicScores = &Play::ComputeScore_QFnApprox;
			i_QfnUpdateLocalizationDepth = (config)"qfn_update_localization_depth";
			b_PerPlyApproximation = (1 == (int)(config)"per_ply_approximation");
		}
		else
		{
			cerr << "[EE] Unknown default sampling method ["
				 << sHeuristic << "] for MCTS-UCT." << endl;
			return false;
		}
		*/
	}
	else
	{
		cerr << "[EE] Unknown action sampling method ["
			 << sSamplingMethod << ']' << endl;
		return false;
	}

	if (false == o_TextInterface.Init (b_UseOperatorWordModel))
		return false;

	return true;
}


//												
void Play::Destroy (void)
{
}


//												
String Play::GetActionTypeAndUnitId (String _sCommand)
{
	String sId;
	zchar_dq_t dqValues = _sCommand.DestructiveSplit (' ');

	char cUnitType = (dqValues [0][0] & ~1);
	if (LCP_NATION_COMMAND == cUnitType)
		sId = dqValues [0][0];
	else if (LCP_CITY_COMMAND == cUnitType)
		sId << dqValues [0][0] << dqValues [1];
	else
		sId << dqValues [0][0] << dqValues [1];

	return sId;
}


//												
void Play::CombineAgentAndActionFeatures (Action& _rAction)
{
	_rAction.o_AugmentedFeatures.SetSize (_rAction.o_Features.Size () + 
										  _rAction.p_Agent->o_Features.Size ());
	_rAction.o_AugmentedFeatures.Set (_rAction.o_Features);
	_rAction.o_AugmentedFeatures.Set (_rAction.p_Agent->o_Features);
}


//												
void Play::CombineHiddenStateAndStateFeatures (Action& _rAction)
{
	Agent* pAgent = _rAction.p_Agent;

	// action * hidden state		
	_rAction.o_LanguageFeatures.SetPrefixFeature (_rAction.i_CommandType,
												  _rAction.p_SelectedHiddenState->i_FeatureId);


	// state labels * hidden state	
	ITERATE (int_dq_t, _rAction.dq_FeatureLabelIndices, ite)
		_rAction.o_LanguageFeatures.SetPrefixFeature (*ite, _rAction.p_SelectedHiddenState->i_FeatureId);

	ITERATE (int_dq_t, pAgent->dq_FeatureLabelIndices, ite)
		_rAction.o_LanguageFeatures.SetPrefixFeature (*ite, _rAction.p_SelectedHiddenState->i_FeatureId);
}


//												
void Play::CombineTextAndStateFeatures (Sentence& _rSentence,
										Action& _rAction)
{
	Agent* pAgent = _rAction.p_Agent;

	// action * words		
	_rAction.o_LanguageFeatures.SetBagOfWords (_rAction.i_CommandType,
											   _rSentence.p_WordIndices,
											   _rSentence.i_Words);

	// unit-action * words	
	_rAction.o_LanguageFeatures.SetBagOfWords (_rAction.i_UnitCommand,
												_rSentence.p_WordIndices,
												_rSentence.i_Words);

	// state labels * words	
	ITERATE (int_dq_t, _rAction.dq_FeatureLabelIndices, ite)
		_rAction.o_LanguageFeatures.SetBagOfWords (*ite,
												   _rSentence.p_WordIndices,
												   _rSentence.i_Words);

	ITERATE (int_dq_t, pAgent->dq_FeatureLabelIndices, ite)
		_rAction.o_LanguageFeatures.SetBagOfWords (*ite,
												   _rSentence.p_WordIndices,
												   _rSentence.i_Words);
}


//												
void Play::CombineTextAndStateOperatorFeatures (Sentence& _rSentence,
												Action& _rAction)
{
	Agent* pAgent = _rAction.p_Agent;

	size_t iWords = _rSentence.i_ActionStateWords;
	_rAction.o_OperatorFeatures.SetSize (iWords * (2 + _rAction.dq_FeatureLabelIndices.size () +
												   pAgent->dq_FeatureLabelIndices.size ()));

	// action * words		
	_rAction.o_LanguageFeatures.SetBagOfWords (_rAction.i_CommandType,
											   _rSentence.p_WordIndices,
											   _rSentence.i_Words);
	_rAction.o_OperatorFeatures.SetBagOfWords (_rAction.i_CommandType,
											   _rSentence.p_ActionStateWordIndices,
											   _rSentence.i_ActionStateWords);

	// unit-action * words	
	_rAction.o_LanguageFeatures.SetBagOfWords (_rAction.i_UnitCommand,
											   _rSentence.p_WordIndices,
											   _rSentence.i_Words);
	_rAction.o_OperatorFeatures.SetBagOfWords (_rAction.i_UnitCommand,
											   _rSentence.p_ActionStateWordIndices,
											   _rSentence.i_ActionStateWords);

	// state labels * words	
	ITERATE (int_dq_t, _rAction.dq_FeatureLabelIndices, ite)
	{
		_rAction.o_LanguageFeatures.SetBagOfWords (*ite,
												   _rSentence.p_WordIndices,
												   _rSentence.i_Words);
		_rAction.o_OperatorFeatures.SetBagOfWords (*ite,
												   _rSentence.p_ActionStateWordIndices,
												   _rSentence.i_ActionStateWords);
	}

	ITERATE (int_dq_t, pAgent->dq_FeatureLabelIndices, ite)
	{
		_rAction.o_LanguageFeatures.SetBagOfWords (*ite,
												   _rSentence.p_WordIndices,
												   _rSentence.i_Words);
		_rAction.o_OperatorFeatures.SetBagOfWords (*ite,
												   _rSentence.p_ActionStateWordIndices,
												   _rSentence.i_ActionStateWords);
	}
}


//												
void Play::CombineTextAndStateOperatorFeaturesOnly (Sentence& _rSentence,
													Action& _rAction)
{
	Agent* pAgent = _rAction.p_Agent;

	size_t iWords = _rSentence.i_ActionStateWords;
	_rAction.o_OperatorFeatures.SetSize (iWords * (2 + _rAction.dq_FeatureLabelIndices.size () +
												   pAgent->dq_FeatureLabelIndices.size ()));

	// action * words		
	_rAction.o_OperatorFeatures.SetBagOfWords (_rAction.i_CommandType,
											   _rSentence.p_ActionStateWordIndices,
											   _rSentence.i_ActionStateWords);

	// unit-action * words	
	_rAction.o_OperatorFeatures.SetBagOfWords (_rAction.i_UnitCommand,
											   _rSentence.p_ActionStateWordIndices,
											   _rSentence.i_ActionStateWords);

	// state labels * words	
	ITERATE (int_dq_t, _rAction.dq_FeatureLabelIndices, ite)
	{
		_rAction.o_OperatorFeatures.SetBagOfWords (*ite,
												   _rSentence.p_ActionStateWordIndices,
												   _rSentence.i_ActionStateWords);
	}

	ITERATE (int_dq_t, pAgent->dq_FeatureLabelIndices, ite)
	{
		_rAction.o_OperatorFeatures.SetBagOfWords (*ite,
												   _rSentence.p_ActionStateWordIndices,
												   _rSentence.i_ActionStateWords);
	}
}


//												
void Play::NormalizeScores (double* _dScores, size_t _iCount)
{
	// we might have negative scores,	
	double dMin = 0;
	for (size_t i = 0; i < _iCount; ++ i)
	{
		if (dMin > _dScores [i])
			dMin = _dScores [i];
	}
	if (dMin < 0)
	{
		for (size_t i = 0; i < _iCount; ++ i)
			_dScores [i] -= dMin;
	}

	// normalize...						
	double dSum = 0;
	for (size_t i = 0; i < _iCount; ++ i)
		dSum += _dScores [i];

	if (0 == dSum)
	{
		for (size_t i = 0; i < _iCount; ++ i)
			_dScores [i] = 1 / (double) _iCount;
	}
	else
	{
		for (size_t i = 0; i < _iCount; ++ i)
			_dScores [i] /= dSum;
	}
}


//												
void Play::ComputeScore_Uniform (Agent& _rAgent, double* _dScore)
{
	size_t iActions = _rAgent.vec_Actions.size ();
	for (size_t i = 0; i < iActions; ++ i)
		_dScore [i] = 1 / (double) iActions;
}


//												
void Play::ComputeScore_EstimatedPrior (Agent& _rAgent, double* _dScore)
{
	double dTotal = 0;
	long lCount = 0;

	int i = 0;

	#pragma omp critical
	ITERATE (Action_vec_t, _rAgent.vec_Actions, iteAction)
	{
		String_dq_t dqCommandType;
		iteAction->s_Command.Split (dqCommandType, ' ', 1);

		ActionToAverage_map_t::iterator ite;
		ite = map_ActionToAverage.find (dqCommandType [0]);
		if (map_ActionToAverage.end () != ite)
		{
			_dScore [i++] = ite->second.first / ite->second.second;
			dTotal += ite->second.first / ite->second.second;
			++ lCount;
		}
		else
			_dScore [i++] = -1;
	}

	if (0 == lCount)
	{
		dTotal = 1;
		lCount = 1;
	}

	// smooth out the cases for which we have no info...
	for (size_t i = 0; i < _rAgent.vec_Actions.size (); ++ i)
	{
		if (-1 == _dScore [i])
			_dScore [i] = dTotal / (double) lCount;
	}
}


//												
void Play::ComputeScore_TextInfo (Agent& _rAgent, double* _dScore)
{
	abort ();
	/*
	size_t iActions = _rAgent.vec_Actions.size ();

	int i = 0;
	ITERATE (Action_vec_t, _rAgent.vec_Actions, iteAction)
	{
		String_dq_t dqCompetingLabels;
		ITERATE (Action_vec_t, _rAgent.vec_Actions, iteOther)
		{
			if (iteOther == iteAction)
				continue;
			dqCompetingLabels.insert (dqCompetingLabels.end (),
									  iteOther->dq_FeatureLabels.begin (),
									  iteOther->dq_FeatureLabels.end ());
		}

		String_dq_t dqLabels;
		dqLabels.insert (dqLabels.end (),
						 iteAction->dq_FeatureLabels.begin (),
						 iteAction->dq_FeatureLabels.end ());
		dqLabels.insert (dqLabels.end (),
						 _rAgent.dq_FeatureLabels.begin (),
						 _rAgent.dq_FeatureLabels.end ());

		_dScore [i] = o_TextInterface.GetScore (dqLabels, dqCompetingLabels, iActions);
		// _dScore [i] += o_TextInterface.GetScore (_rAgent.dq_FeatureLabels);
		++ i;
	}
	*/
}


//												
void Play::ComputeScore_TextAndEstimatedPrior (Agent& _rAgent, double* _dScore)
{
	// pthread_rwlock_rdlock (&rwl_Model);

	size_t iActions = _rAgent.vec_Actions.size ();

	double dTextScore [iActions];
	ComputeScore_TextInfo (_rAgent, dTextScore);
	double dEstimatedPriorScore [iActions];
	ComputeScore_EstimatedPrior (_rAgent, dEstimatedPriorScore);

	for (size_t i = 0; i < iActions; ++ i)
		_dScore [i] = 0.5 * (dTextScore [i] + dEstimatedPriorScore [i]);

	// pthread_rwlock_unlock (&rwl_Model);
}


//												
void Play::ComputeScore_QFnApprox (Sample& _rSample, Agent& _rAgent, double* _dScore)
{
	int i = 0;
	ITERATE (Action_vec_t, _rAgent.vec_Actions, ite)
	{
		Action& rAction = *ite;

		// use hidden state model...							
		rAction.p_SelectedHiddenState = NULL;
		if (true == b_UseHiddenStateModel)
			rAction.p_SelectedHiddenState = o_HiddenStateModel.SelectRelevantState (_rSample,
																					rAction);

		CombineAgentAndActionFeatures (rAction);
		if (NULL != rAction.p_SelectedHiddenState)
		{
			rAction.o_LanguageFeatures.SetSize (1 + rAction.dq_FeatureLabelIndices.size () +
												rAction.p_Agent->dq_FeatureLabelIndices.size ());
			CombineHiddenStateAndStateFeatures (rAction);
		}

		_dScore [i++] = o_LinearFunctionApprox.ComputeFunctionApprox (rAction.o_AugmentedFeatures,
																	  rAction.o_LanguageFeatures,
																	  rAction.o_OperatorFeatures);
	}
}


//												
void Play::ComputeScore_QFnApproxWithText (Sample& _rSample,
										   Agent& _rAgent,
										   double* _dScore)
{
	int i = 0;
	ITERATE (Action_vec_t, _rAgent.vec_Actions, ite)
	{
		Action& rAction = *ite;

		// use hidden state model...							
		rAction.p_SelectedHiddenState = NULL;
		if (true == b_UseHiddenStateModel)
			rAction.p_SelectedHiddenState = o_HiddenStateModel.SelectRelevantState (_rSample,
																					rAction);


		// There may be no filtered sentences for an agent...	
		Sentence* pSentence = NULL;
		if (true == b_UseLabelOverlapSentenceSelection)
		{
			o_TextInterface.GetBestSentenceAndScore (_rSample,
													rAction.dq_FeatureLabels,
													rAction.p_Agent->dq_FeatureLabels,
													rAction.p_Agent->p_FilteredSentences,
													&pSentence);
			// RememberSentenceHit (pSentence);
		}
		else if (NULL != _rAgent.p_FilteredSentences)
		{
			// The call below saves the selected sentence to rAction	
			bool bIsArgmax = false;
			pSentence = o_RelevanceModel.SelectRelevantSentence (_rSample,
																 rAction,
																 *_rAgent.p_FilteredSentences,
																 bIsArgmax);
			if (_rSample.SampleUniform () < f_HeuristicBiasProbability)
			{
				o_TextInterface.GetBestSentenceAndScore (_rSample,
														rAction.dq_FeatureLabels,
														rAction.p_Agent->dq_FeatureLabels,
														rAction.p_Agent->p_FilteredSentences,
														&pSentence);
			}

			rAction.p_SelectedSentence = pSentence;
			rAction.b_IsArgmaxSentence = bIsArgmax;
			// RememberSentenceHit (pSentence);
		}


		// if we're using the operator words model, label the selected	
		// sentence words with operator labels...						
		if ((true == b_UseOperatorWordModel) &&
			(NULL != pSentence) &&
			(false == pSentence->b_IsNullSentence))
		{
			OperatorLabeledSentence* pLabeledSentence
					= new OperatorLabeledSentence (*pSentence);
			o_OperatorWordsModel.PredictOperatorWords (_rSample, *pLabeledSentence);

			// pLabeledSentence is allocated locally here.  It is 		
			// deleted in UpdateQFnApprox...							
			pSentence = pLabeledSentence;
			rAction.p_SelectedSentence = pLabeledSentence;
		}


		// compute Q function approximation using all above info...		
		CombineAgentAndActionFeatures (rAction);

		if (NULL != pSentence)
		{
			size_t iWords = pSentence->i_Words;
			if (NULL != rAction.p_SelectedHiddenState)
				++ iWords;
			rAction.o_LanguageFeatures.SetSize (iWords * (2 + rAction.dq_FeatureLabelIndices.size () +
														  rAction.p_Agent->dq_FeatureLabelIndices.size ()));

			if (true == b_UseOperatorFeaturesOnly)
				CombineTextAndStateOperatorFeaturesOnly (*pSentence, rAction);
			else if (true == b_UseOperatorFeatures)
				CombineTextAndStateOperatorFeatures (*pSentence, rAction);
			else
				CombineTextAndStateFeatures (*pSentence, rAction);
		}
		else
		{
			rAction.o_LanguageFeatures.SetSize (1 + rAction.dq_FeatureLabelIndices.size () +
												rAction.p_Agent->dq_FeatureLabelIndices.size ());
		}

		if (NULL != rAction.p_SelectedHiddenState)
			CombineHiddenStateAndStateFeatures (rAction);

		_dScore [i++] = o_LinearFunctionApprox.ComputeFunctionApprox (rAction.o_AugmentedFeatures,
																	  rAction.o_LanguageFeatures,
																	  rAction.o_OperatorFeatures);
	}
	// pthread_rwlock_unlock (&rwl_Model);
}


//												
void Play::RememberSentenceHit (Sentence* _pSentence)
{
	SentenceToCount_map_t::iterator	ite;
	ite = map_SentenceToHitCount.find (_pSentence->i_Index);
	if (map_SentenceToHitCount.end () == ite)
		map_SentenceToHitCount.insert (make_pair (_pSentence->i_Index, 1));
	else
		++ ite->second;
}


//												
size_t Play::SampleAction_Uniform (Sample& _rSample, 
								   String& _rState,
								   Agent& _rAgent,
								   float _fExplorationProbability)
{
	return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());
}


//												
size_t Play::SampleAction_EstimatedPrior (Sample& _rSample, 
										  String& _rState,
										  Agent& _rAgent,
										  float _fExplorationProbability)
{
	if (_rSample.SampleUniform () < _fExplorationProbability)
		return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());

	size_t iActions = _rAgent.vec_Actions.size ();
	double dScore [iActions];
	ComputeScore_EstimatedPrior (_rAgent, dScore);
	NormalizeScores (dScore, iActions);

	return _rSample.Argmax (dScore, iActions);
}


//												
size_t Play::SampleAction_TextInfo (Sample& _rSample,
									String& _rState,
									Agent& _rAgent,
									float _fExplorationProbability)
{
	if (_rSample.SampleUniform () < _fExplorationProbability)
		return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());

	size_t iActions = _rAgent.vec_Actions.size ();
	double dScore [iActions];
	ComputeScore_TextInfo (_rAgent, dScore);
	NormalizeScores (dScore, iActions);

	return _rSample.Argmax (dScore, iActions);
}


//												
size_t Play::SampleAction_TextAndEstimatedPrior (Sample& _rSample, 
												 String& _rState,
												 Agent& _rAgent,
												 float _fExplorationProbability)
{
	if (_rSample.SampleUniform () < _fExplorationProbability)
		return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());

	size_t iActions = _rAgent.vec_Actions.size ();
	double dScore [iActions];
	ComputeScore_TextAndEstimatedPrior (_rAgent, dScore);
	NormalizeScores (dScore, iActions);

	return _rSample.Argmax (dScore, iActions);
}


//												
size_t Play::SampleAction_MCTS_UCT (Sample& _rSample, 
									String& _rState,
									Agent& _rAgent,
									float _fExplorationProbability)
{
	abort ();
	return 0;

	/*
	size_t iActionIndex = -1;
	TreeUnit* pUnit;

	#pragma omp critical
	pUnit = o_Tree.GetTreeUnit (_rState, _rAgent.s_UnitType);

	size_t iActions = _rAgent.vec_Actions.size ();
	double dHeuristicScores [iActions];
	(this->*pfn_ComputeHeuristicScores) (_rAgent, dHeuristicScores);

	#pragma omp critical
	{
		ActionToIndex_map_t	mapValidActions;
		for (size_t i = 0; i < iActions; ++ i)
		{
			// s_GenericCommand does not contain the unit id,	
			// and is thus generic across units ...				
			String& rCommand = _rAgent.vec_Actions [i].s_GenericCommand;
			pUnit->AddHeuristicQValue (rCommand, dHeuristicScores [i]);
			mapValidActions.insert (make_pair (rCommand, i));
		}

		iActionIndex = pUnit->SelectAction (d_ExplorationBonusRatio,
											mapValidActions);
	}
	return iActionIndex;
	*/
}


//												
size_t Play::SampleAction_QFnApprox (Sample& _rSample,
									 String& _rState,
									 Agent& _rAgent,
									 float _fExplorationProbability)
{
	if (_rSample.SampleUniform () < _fExplorationProbability)
	{
		if (0 == d_NewActionBias)
			return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());

		if ((false == _rAgent.dq_NewActions.empty ()) &&
			(_rSample.SampleUniform () < d_NewActionBias))
		{
			cout << '|' << flush;
			int iActionIndex = _rSample.SampleUniformCategorical (_rAgent.dq_NewActions.size ());
			return _rAgent.dq_NewActions [iActionIndex];
		}
		return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());
	}

	size_t iActions = _rAgent.vec_Actions.size ();
	double dScore [iActions];
	ComputeScore_QFnApprox (_rSample, _rAgent, dScore);

	return _rSample.Argmax (dScore, iActions);
}


//												
size_t Play::SampleAction_QFnApproxWithText (Sample& _rSample, 
											 String& _rState,
											 Agent& _rAgent,
											 float _fExplorationProbability)
{
	if (_rSample.SampleUniform () < _fExplorationProbability)
	{
		if (0 == d_NewActionBias)
			return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());

		if ((false == _rAgent.dq_NewActions.empty ()) && 
			(_rSample.SampleUniform () < d_NewActionBias))
		{
			cout << ':' << flush;
			int iActionIndex = _rSample.SampleUniformCategorical (_rAgent.dq_NewActions.size ());
			return _rAgent.dq_NewActions [iActionIndex];
		}
		return _rSample.SampleUniformCategorical (_rAgent.vec_Actions.size ());
	}

	size_t iActions = _rAgent.vec_Actions.size ();
	double dScore [iActions];
	ComputeScore_QFnApproxWithText (_rSample, _rAgent, dScore);

	return _rSample.Argmax (dScore, iActions);
}


//												
String Play::SelectBestCommand_NaiveBayes (Sample& _rSample,
										   CommandToScore_map_t& _rmapCommandToScore)
{
	UnitToCommandScores_map_t	mapUnitToCommandScores;

	// collate command scores by unit...			
	ITERATE (CommandToScore_map_t, _rmapCommandToScore, iteOptions)
	{
		String sCommand = iteOptions->first;
		double dScore = iteOptions->second;

		String_dq_t dqUnitCommands;
		sCommand.Split (dqUnitCommands, '\n');
		ITERATE (String_dq_t, dqUnitCommands, ite)
		{
			if ("" == *ite)
				continue;
			String sUnit = GetActionTypeAndUnitId (*ite);

			pair <UnitToCommandScores_map_t::iterator, bool> pairInsertUnit;
			pairInsertUnit = mapUnitToCommandScores.insert (make_pair (sUnit, CommandToScoreTotal_map_t ()));
			CommandToScoreTotal_map_t& rmapCommandToScore = pairInsertUnit.first->second;

			pair <CommandToScoreTotal_map_t::iterator, bool> pairInsertScore;
			pairInsertScore = rmapCommandToScore.insert (make_pair (*ite, make_pair (dScore, 1)));
			if (false == pairInsertScore.second)
			{
				pairInsertScore.first->second.first += dScore;
				pairInsertScore.first->second.second += 1;
			}
		}
	}

	// find best command for individual units ...	
	String sCommand;
	ITERATE (UnitToCommandScores_map_t, mapUnitToCommandScores, iteUnit)
	{
		String_dq_t dqBestCommands;
		double dMaxScore = 0;

		CommandToScoreTotal_map_t& rmapCommandToScoreTotal = iteUnit->second;
		ITERATE (CommandToScoreTotal_map_t, rmapCommandToScoreTotal, iteCommand)
		{
			double dScore = iteCommand->second.first / (double)iteCommand->second.second;
			if ((dMaxScore < dScore) || (true == dqBestCommands.empty ()))
			{
				dMaxScore = dScore;
				dqBestCommands.clear ();
				dqBestCommands.push_back (iteCommand->first);
			}
			else if (dMaxScore == dScore)
				dqBestCommands.push_back (iteCommand->first);
		}

		String sUnitCommand = (1 == dqBestCommands.size ())?
								dqBestCommands.front () :
								dqBestCommands [_rSample.SampleUniformCategorical (dqBestCommands.size ())];
		sCommand << sUnitCommand << '\n';
	}

	sCommand.Strip ();
	return sCommand;
}


//												
String Play::SelectBestCommand_Trivial (Sample& _rSample,
										CommandToScore_map_t& _rmapCommandToScore)
{
	// select one out of the best next steps.
	String_dq_t	dqBestCommands;
	double dMaxScore = 0;

	ITERATE (CommandToScore_map_t, _rmapCommandToScore, ite)
	{
		String sCommand = ite->first;
		double dScore = ite->second;

		if ((dMaxScore < dScore) || (true == dqBestCommands.empty ()))
		{
			dMaxScore = dScore;
			dqBestCommands.clear ();
			dqBestCommands.push_back (sCommand);
		}
		else if (dMaxScore == dScore)
			dqBestCommands.push_back (sCommand);
	}

	if (true == dqBestCommands.empty ())
	{
		cerr << "[EE] Rollouts didn't return any commands!" << endl;
		abort ();
		return "";
	}
	if (1 == dqBestCommands.size ())
		return dqBestCommands.front ();

	return dqBestCommands [_rSample.SampleUniformCategorical (dqBestCommands.size ())];
}


//												
void Play::SaveParams (void)
{
	if ((true == UsingQValueApprox ()) ||
		(true == UsingQValueApproxWithText ()))
	{
		o_LinearFunctionApprox.SaveWeights ("lqfn");
		if (true == b_UseHiddenStateModel)
			o_HiddenStateModel.SaveWeights ("hsv");
	}

	if (true == UsingQValueApproxWithText ())
	{
		if (true == b_UsingSentenceRelevanceModel)
		{
			o_RelevanceModel.SaveWeights ("srm");
			if (true == b_UseOperatorWordModel)
				o_OperatorWordsModel.SaveWeights ("opw");
		}
	}
}


//												
void Play::WriteIterationMarker (int _iIteration)
{
	File file;
	if (false == file.Open ((config)"iteration_marker_file", ios_base::out | ios_base::app))
		return;

	file << _iIteration << endl;
	file.Close ();
}


//												
void Play::EndPly (int _iStep, bool _bDontResetWeights, bool _bTestMode)
{
	if (false == _bTestMode)
		SaveParams ();

	if ((true == UsingQValueApprox ()) ||
		(true == UsingQValueApproxWithText ()))
	{
		if ((true == b_PerPlyApproximation) && (false == _bDontResetWeights))
			o_LinearFunctionApprox.ResetWeights ();
		if ((true == b_ResetAllModelsEndOfPly) && (false == _bDontResetWeights))
		{
			o_HiddenStateModel.ResetWeights ();
			o_RelevanceModel.ResetWeights ();
			o_OperatorWordsModel.ResetWeights ();
		}

		o_LinearFunctionApprox.ClearShortTermDatapoints ();
		o_LinearFunctionApprox.ClearLongTermDatapoints ();
	}

	if (true == UsingMCTS_UCT ())
		o_Tree.Clear ();

	if (f_HeuristicBiasProbability > f_HeuristicBiasProbabilityMin)
	{
		f_HeuristicBiasProbability -= f_HeuristicBiasProbabilityStep;
		if (f_HeuristicBiasProbability < f_HeuristicBiasProbabilityMin)
			f_HeuristicBiasProbability = f_HeuristicBiasProbabilityMin;
	}
	cout << "heuristic bias : " << f_HeuristicBiasProbability << endl;
	PrintSentenceHitStats ();
}


//												
void Play::UpdateParameters (Sample& _rSample,
							 State_dq_t& _rdqState,
							 Action_dq_t& _rdqActions,
							 double _dScore)
{
	if (true == UsingEstimatedPrior ())
		EstimateActionPrior (_rdqActions, _dScore);
	if (true == UsingMCTS_UCT ())
		UpdateMCT (*_rdqState [0], _rdqActions, _dScore);
	if (true == UsingQValueApprox ())
		UpdateQFnApprox (_rSample, _rdqState, _rdqActions, _dScore);
	if (true == UsingQValueApproxWithText ())
		UpdateQFnApprox (_rSample, _rdqState, _rdqActions, _dScore);
}


//												
void Play::EstimateActionPrior (Action_dq_t& _rdqActions, double _dScore)
{
	#pragma omp critical
	{
		ITERATE (Action_dq_t, _rdqActions, iteAction)
		{
			Action* pAction = *iteAction;
			if (NULL == pAction)
				break;

			String_dq_t dqCommandType;
			pAction->s_Command.Split (dqCommandType, ' ', 1);

			ActionToAverage_map_t::iterator	ite;
			ite = map_ActionToAverage.find (dqCommandType [0]);
			if (map_ActionToAverage.end () != ite)
			{
				ite->second.first += _dScore;
				ite->second.second += 1;
			}
			else
				map_ActionToAverage.insert (make_pair (dqCommandType [0], make_pair (_dScore, 1)));
		}
	}
}


//												
void Play::UpdateMCT (State& _rState, Action_dq_t& _rdqAction, double _dScore)
{
	#pragma omp critical
	{
		TreeState* pState = o_Tree.GetTreeState (_rState.s_State);
		assert (NULL != pState);

		ITERATE (Action_dq_t, _rdqAction, ite)
		{
			Action* pAction = *ite;
			// a NULL indicates end of actions	
			// for a particular state			
			if (NULL == pAction)
				break;

			TreeUnit* pUnit = pState->GetTreeUnit (pAction->p_Agent->s_UnitType);
			pUnit->AddQValue (pAction->s_GenericCommand, _dScore);
		}
	}
}


//												
void Play::UpdateQFnApprox (Sample& _rSample, 
							State_dq_t& _rdqState,
							Action_dq_t& _rdqAction,
							double _dNewValue)
{
	Action_dq_t::iterator	iteAction = _rdqAction.begin ();
	Action_dq_t::iterator	iteAction_end = _rdqAction.end ();

	State_dq_t::iterator	iteState = _rdqState.begin ();
	State_dq_t::iterator	iteState_end = _rdqState.end ();

	if (true == b_OnlineUpdate)
		o_LinearFunctionApprox.ClearShortTermDatapoints ();

	int iCurrentDepth = 0;
	for (; iteState != iteState_end; ++ iteState)
	{
		State* pState = *iteState;
		for (; iteAction != iteAction_end; ++ iteAction)
		{
			Action* pAction = *iteAction;
			// During rollouts, a set of actions would have been selected	
			// for each observed state.  A NULL action seperates these sets	
			// in _rdqAction.  I.e., for each state, we have a list of 		
			// actions ternimated by a NULL in _rdqAction.					
			if (NULL == pAction)
			{
				++ iteAction;
				break;
			}

			Agent::AddAttemptedAction (pAction->s_GenericCommand);


			double dPredicted =
				o_LinearFunctionApprox.ComputeFunctionApprox (pAction->o_AugmentedFeatures,
															  pAction->o_LanguageFeatures,
															  pAction->o_OperatorFeatures);
			
			o_LinearFunctionApprox.AddDatapoint (_rSample,
												 pAction->o_AugmentedFeatures,
												 pAction->o_LanguageFeatures,
												 pAction->o_OperatorFeatures,
												 _dNewValue, 
												 dPredicted,
												 pState->s_State,
												 pAction->p_Agent->s_UnitType);


			double dInnerLayerReward = _dNewValue;
			if (true == b_UseBackpropUpdate)
				dInnerLayerReward = (dPredicted - _dNewValue) * dPredicted;
				

			// update hidden variable model if we're using it ...	
			if (true == b_UseHiddenStateModel)
			{
				if (NULL != pAction->p_SelectedHiddenState)
					o_HiddenStateModel.UpdateParameters (dInnerLayerReward, *pAction);
			}

			// update sentence relevance model if we're using it ...	
			if (true == b_UsingSentenceRelevanceModel)
			{
				if (NULL != pAction->p_SelectedSentence)
				{
					if (true == pAction->b_IsArgmaxSentence)
						RememberSentenceHit (pAction->p_SelectedSentence);

					assert (NULL != pAction->p_Agent->p_FilteredSentences);
					o_RelevanceModel.UpdateParameters (dInnerLayerReward,
												  *pAction, 
												  *pAction->p_Agent->p_FilteredSentences);

					if ((true == b_UseOperatorWordModel) &&
						(false == pAction->p_SelectedSentence->b_IsNullSentence))
					{
						o_OperatorWordsModel.UpdateParameters (dInnerLayerReward,
															   *(OperatorLabeledSentence*)
															   pAction->p_SelectedSentence);
					}
				}
			}
		}

		++ iCurrentDepth;
		if (iCurrentDepth > i_QfnUpdateLocalizationDepth)
			break;
	}
	assert ((iteState != iteState_end) || (iteAction == iteAction_end));


	// We want to log all observed states, actions & rewards for analysis.	
	// So if we're still in logging mode, and our localization depth is 	
	// smaller than the rollout depth, we should record the remaining		
	// observations without using them for updates...						
	if (true == o_LinearFunctionApprox.LoggingDatapoints ())
	{
		for (; iteState != iteState_end; ++ iteState)
		{
			State* pState = *iteState;
			for (; iteAction != iteAction_end; ++ iteAction)
			{
				Action* pAction = *iteAction;
				if (NULL == pAction)
				{
					++ iteAction;
					break;
				}
				double dPredicted =
					o_LinearFunctionApprox.ComputeFunctionApprox (pAction->o_AugmentedFeatures,
																  pAction->o_LanguageFeatures,
																  pAction->o_OperatorFeatures);

				// RememberDatapoint() only records the datapoint for 		
				// data logging purposes.  It does not change the data that	
				// is used to update the linear function approx. params.	
				o_LinearFunctionApprox.RememberDatapoint (_rSample,
														 pState->s_State,
														 pAction->p_Agent->s_UnitType,
														 pAction->o_AugmentedFeatures,
														 _dNewValue, 
														 dPredicted);

			}
		}
	}


	// update linear function approx. parameters ...						
	o_LinearFunctionApprox.UpdateFunctionApprox (false);
}


//												
String Play::GetCompoundCommand (Action_dq_t& _rdqActions)
{
	String sCompoundCommand;
	ITERATE (Action_dq_t, _rdqActions, ite)
	{
		if (NULL == *ite)
			break;
		sCompoundCommand << (*ite)->s_Command << '\n';
	}
	sCompoundCommand.Strip ();
	return sCompoundCommand;
}


//												
void Play::PrintStats (Sample& _rSample)
{
	// PrintSentenceHitStats (_rSample);
	PrintPredicatePredictions (_rSample);	
}


//												
void Play::PrintEstimatedPrior (Sample& _rSample)
{
	cout << "----------------------------------------" << endl;
	cout << "Estimated prior :" << endl;
	ITERATE (ActionToAverage_map_t, map_ActionToAverage, ite)
	{
		cout << ite->first << " : " 
			 << ite->second.first << '/' << ite->second.second << " = "
			 << ite->second.first / ite->second.second
			 << endl;
	}
	cout << "----------------------------------------" << endl;
}


//												
void Play::PrintSentenceHitStats (void)
{
	File file;
	if (false == file.Open ((config)"sentence_relevance_stats_file", ios_base::out|ios_base::app))
		return;

	pthread_mutex_lock (&mtx_SentenceHit);

	FilteredSentences* pAllSentence = o_TextInterface.GetAllSentences ();
	for (int s = 0; s < (int)pAllSentence->vec_Sentences.Size (); ++ s)
	{
		Sentence* pSentence = pAllSentence->vec_Sentences [s];

		SentenceToCount_map_t::iterator	ite;
		long lCount = 0;
		ite = map_SentenceToHitCount.find (pSentence->i_Index);
		if (map_SentenceToHitCount.end () != ite)
			lCount = ite->second;

		if (0 != s)
			file << ',';
		file << lCount;
	}
	map_SentenceToHitCount.clear ();

	pthread_mutex_unlock (&mtx_SentenceHit);

	file << endl << flush;
	file.Close ();

	cout << "saved hit statistics : " 
		 << pAllSentence->vec_Sentences.Size ()
		 << " sentences." << endl;

	return;
}


//												
void Play::PrintPredicatePredictions (Sample& _rSample)
{
	if (false == b_UseOperatorWordModel)
		return;

	File file;
	if (false == file.Open ((config)"predicate_prediction_output", ios_base::out))
		return;

	FilteredSentences* pAllSentences = o_TextInterface.GetAllSentences ();
	for (int i = 0; i < (int)pAllSentences->vec_Sentences.Size (); ++ i)
	{
		Sentence* pSentence = pAllSentences->vec_Sentences [i];
		if (true == pSentence->b_IsNullSentence)
			continue;

		OperatorLabeledSentence oLabeledSentence (*pSentence);
		o_OperatorWordsModel.PredictOperatorWords (_rSample, oLabeledSentence);

		String sLabeledSentence;
		for (int w = 0; w < pSentence->i_Words; ++ w)
		{
			sLabeledSentence << pSentence->dq_Words [w] << '_'
							 << oLabeledSentence.p_Labels [w] << ' ';
		}
		file << sLabeledSentence << endl;
	}

	file.Close ();
}








