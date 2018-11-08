#include <nlp_config.h>
#include <nlp_macros.h>
#include <omp.h>
#include "Montecarlo.h"
#include "learner_comms.h"


//												
MonteCarlo::MonteCarlo (void)
{
	pthread_rwlock_init (&rwl_Model, NULL);
	b_QfnUpdateBeforeCommandSelection = false;
	i_LanguageLearningRollouts = 0;
}


//												
MonteCarlo::~MonteCarlo (void)
{
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
		p_Sample [i].Destroy ();
	delete[] p_Sample;

	for (int i = 0; i < i_SimulationInterfaces; ++ i)
		p_SimulationInterfaces [i].Disconnect ();
	delete[] p_SimulationInterfaces;

	pthread_rwlock_destroy (&rwl_Model);
}


//												
bool MonteCarlo::Init (void)
{
	i_SimulationInterfaces = (config)"simulators";
	p_Sample = new Sample [i_SimulationInterfaces];
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
		p_Sample [i].Init ();

	p_SimulationInterfaces = new GameInterface [i_SimulationInterfaces];
	cout << "Connecting to simulators " << flush;
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
	{
		String sConfig;
		sConfig << "simulation_" << i;
		if (false == p_SimulationInterfaces [i].Init (sConfig))
			return false;
	}
	cout << endl;

	i_LanguageLearningRollouts = (config)"language_learning_rollouts";
	i_MaxRollouts = (config)"max_rollouts";
	i_MaxRolloutDepth = (config)"max_rollout_depth";
	i_FinalRollouts = (config)"final_rollouts";
	i_FinalRolloutDepth = (config)"final_rollout_depth";

	b_SelectActionBasedOnAverageQValue = (1 == (int)(config)"select_action_based_on_mean_q_value");
	d_MinExplorationProbability = (config)"min_exploration_probability";
	double dMaxExplorationProbability = (config)"max_exploration_probability";
	d_ExplorationProbabilityRange = (dMaxExplorationProbability - d_MinExplorationProbability);

	i_LogDatapointsFrom = (config)"log_datapoints_from";
	i_LogDatapointsUntil = (config)"log_datapoints_until";
	i_DatapointLogLength = (config)"datapoint_log_length";

	b_LogAllParamUpdates = (1 == (int)(config)"log_all_param_updates");
	b_UpdateInTestMode = (1 == (int)(config)"update_in_test_mode");


	if (false == Play::Init ())
		return false;

	String sScoreAssignment = (config)"score_assignment";
	cout << "score assignment : " << sScoreAssignment << endl; 
	if ("trivial" == sScoreAssignment)
		e_CommandSelectionMethod = csm_trivial;
	else if ("naive-bayes" == sScoreAssignment)
		e_CommandSelectionMethod = csm_naive_bayes;
	else if ("qfn" == sScoreAssignment)
	{
		e_CommandSelectionMethod = csm_q_fn;
		i_QfnSelectionStart = (config)"qfn_selection_start";
		b_QfnUpdateBeforeCommandSelection = (1 == (int)(config)"qfn_update_before_command_selection");
		if (false == Play::UsingQValueApproxSampling ())
		{
			cout << "[ERROR] The 'qfn' score assignment option is valid "
					"only for the 'linear-qfn-approx' and 'linear-qfn-approx-with-text' "
					"command sampling methods"
				 << endl;
			abort ();
		}
	}
	else
	{
		cerr << "[EE] Unknown score assignment method ["
			 << sScoreAssignment << ']' << endl;
		return false;
	}

	return true;
}


//												
void MonteCarlo::Close (void)
{
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
		p_SimulationInterfaces [i].Disconnect ();

	o_LinearFunctionApprox.WriteDatapointLog ("lqfn", false);
	o_TextInterface.WriteFilteringStats ();
	cout << "Null sentence ratio : "
		 << o_RelevanceModel.NullSentenceRatio ()
		 << endl;
	// o_TextInterface.WriteStats ();
}


//												
String MonteCarlo::SelectCommand (GameInterface& _rActualGame,
								  String& _rStateName, 
								  int _iStep,
								  bool _bTestMode,
								  int _iStepsToFinalRollout)
{
	CommandToScore_mmp_t	mmpCommandToScoreLocal [i_SimulationInterfaces];
	double dThreadBestScore [i_SimulationInterfaces];
	double dThreadWorstScore [i_SimulationInterfaces];
	double dThreadTotalScore [i_SimulationInterfaces];
	float_dq_t dqThreadFinalScores [i_SimulationInterfaces];
	int_dq_t dqThreadFinalWins [i_SimulationInterfaces];
	double dGreedyScore;
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
	{
		dThreadBestScore [i] = -100000000000;
		dThreadWorstScore [i] = 100000000000;
		dThreadTotalScore [i] = 0;
	}

	if (_iStep == i_LogDatapointsFrom)
		o_LinearFunctionApprox.StartDatapointLog (i_DatapointLogLength);
	if (_iStep > i_LogDatapointsUntil)
		o_LinearFunctionApprox.StopDatapointLog ();


	// rollout traces, and keep best next steps.
	bool bFinalStep = (0 == _iStepsToFinalRollout);
	int iRollouts = i_MaxRollouts;
	if (true == bFinalStep)
		iRollouts = i_FinalRollouts;

	omp_set_num_threads (i_SimulationInterfaces);

	// Learn the language parameters...			
	if (i_LanguageLearningRollouts > 0)
	{
		// These rollouts are only for learning language.	
		// The Q function updates that are performed during	
		// these rollouts are discarded at the end so as to	
		// remain comparable with methods that don't use	
		// langauge (i.e. all methods get the same number	
		// of iterations to the Q function from.)			
		o_LinearFunctionApprox.SnapshotWeights ();

		#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < i_LanguageLearningRollouts; ++ i)
		{
			if (0 == (i % 10))
				cout << ':' << flush;
			int iThread = omp_get_thread_num ();
			p_SimulationInterfaces [iThread].LoadGame (_rStateName);

			String sCommand;
			double dScore;
			double dReward;
			RolloutTrace (p_Sample [iThread],
						  p_SimulationInterfaces [iThread],
						  &sCommand, &dReward, &dScore, i,
						  false, false, bFinalStep);
		}

		o_LinearFunctionApprox.ClearShortTermDatapoints ();
		o_LinearFunctionApprox.ClearLongTermDatapoints ();
		o_LinearFunctionApprox.RollbackWeights ();
	}

	// Learn the Q function parameters...		
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < iRollouts; ++ i)
	{
		if (0 == (i % 10))
			cout << '.' << flush;
		bool bGreedyRollout = (i == (iRollouts - 1));
		int iThread = omp_get_thread_num ();
		p_SimulationInterfaces [iThread].LoadGame (_rStateName);

		String sCommand;
		double dScore;
		double dReward;
		RolloutTrace (p_Sample [iThread],
					  p_SimulationInterfaces [iThread],
					  &sCommand, &dReward, &dScore, i,
					  bGreedyRollout, _bTestMode, bFinalStep);

		mmpCommandToScoreLocal [iThread].insert (make_pair (sCommand, dReward));
		dThreadTotalScore [iThread] += dScore;
		if (dThreadBestScore [iThread] < dScore)
			dThreadBestScore [iThread] = dScore;
		if (dThreadWorstScore [iThread] > dScore)
			dThreadWorstScore [iThread] = dScore;
		if (true == bGreedyRollout)
			dGreedyScore = dScore;

		if (true == bFinalStep)
		{
			dqThreadFinalScores [iThread].push_back (dScore);
			int iWin = -1;
			if (1000 == dReward)
				iWin = 1;
			else if (0 == dReward)
				iWin = 0;
			dqThreadFinalWins [iThread].push_back (iWin);
		}
	}
	if (true == UsingQValueApproxSampling ())
		cout << "  " << o_LinearFunctionApprox.WeightVectorNorm () << endl;
	else
		cout << endl;

	// we want to convert a multimap into a map		
	// with scores averaged across equal entries.	
	// So first just insert everything into the map,
	// then retraverse & average ...				
	CommandToScore_map_t	mapCommandToScore;
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
		mapCommandToScore.insert (mmpCommandToScoreLocal [i].begin (), mmpCommandToScoreLocal [i].end ());

	ITERATE (CommandToScore_map_t, mapCommandToScore, iteMap)
	{
		int iCount = 0;
		double dSum = 0;
		double dBest = -100000000;
		for (int i = 0; i < i_SimulationInterfaces; ++ i)
		{
			pair<CommandToScore_mmp_t::iterator, CommandToScore_mmp_t::iterator> pair;
			pair = mmpCommandToScoreLocal [i].equal_range (iteMap->first);

			CommandToScore_mmp_t::iterator	ite;
			for (ite = pair.first; ite != pair.second; ++ ite)
			{
				dSum += ite->second;
				++ iCount;
				if (dBest < ite->second)
					dBest = ite->second;
			}
		}
		if (true == b_SelectActionBasedOnAverageQValue)
			iteMap->second = dSum / iCount;
		else
			iteMap->second = dBest;
	}

	double dTotalScore = 0;
	double dBestScore = -100000000000;
	double dWorstScore = 100000000000;
	for (int i = 0; i < i_SimulationInterfaces; ++ i)
	{
		dTotalScore += dThreadTotalScore [i];
		if (dBestScore < dThreadBestScore [i])
			dBestScore = dThreadBestScore [i];
		if (dWorstScore > dThreadWorstScore [i])
			dWorstScore = dThreadWorstScore [i];
	}
	cout << "Null sentence ratio : "
		 << o_RelevanceModel.NullSentenceRatio ()
		 << endl;
	cout << "greedy score  : " << dGreedyScore << endl;
	cout << "average score : " << dTotalScore / (double)i_MaxRollouts << endl;
	cout << "worst score   : " << dWorstScore << endl;
	cout << "best score    : " << dBestScore << endl;

	if (true == bFinalStep)
	{
		int iWins = 0;
		int iLosses = 0;
		int iTotalCount = 0;
		for (int i = 0; i < i_SimulationInterfaces; ++ i)
		{
			ITERATE (float_dq_t, dqThreadFinalScores [i], ite)
				cout << "final rollout : " << *ite << endl;

			ITERATE (int_dq_t, dqThreadFinalWins [i], ite)
			{
				++ iTotalCount;
				if (1 == *ite)
					++ iWins;
				else if (0 == *ite)
					++ iLosses;
			}
		}

		cout << "final rollout win/loss/count : "
			 << iWins << ", " << iLosses << ", " << iTotalCount << endl;
	}

	// String sCompoundCommand = (this->*pfn_SelectBestCommand) (mapCommandToScore);
	String sCompoundCommand = SelectBestCommand (p_Sample [0],
												 mapCommandToScore,
												 _rActualGame,
												 _iStep);

	bool bStepBeforeFinal = (1 == _iStepsToFinalRollout);
	bool bDontResetWeights = bStepBeforeFinal;
	Play::EndPly (_iStep, bDontResetWeights, _bTestMode);
	o_LinearFunctionApprox.WriteDatapointLog ("lqfn", (0 == _iStep));

	return sCompoundCommand;
}



//												
void MonteCarlo::RolloutTrace (Sample& _rSample,
							   GameInterface& _rGameInterface, 
							   String* _pCommand,
							   double* _pReward,
							   double* _pScore,
							   int _iIteration,
							   bool _bGreedyRollout,
							   bool _bTestMode,
							   bool _bFinalStep)
{
	State_dq_t dqStates;
	Action_dq_t dqActions;

	float fExplorationProbability = 0;
	if (false == _bGreedyRollout)
		fExplorationProbability = d_MinExplorationProbability + 
									_rSample.SampleUniform () *
									d_ExplorationProbabilityRange;

	*_pScore = 0;
	*_pReward = 0;
	State* pState = new State;
	dqStates.push_back (pState);
	if (false == SimulateStep (_rSample,
							   _rGameInterface,
							   0,
							   _pReward,
							   _pScore,
							   pState,
							   &dqActions,
							   fExplorationProbability))
		return;

	int iRolloutDepth = i_MaxRolloutDepth;
	if (true == _bFinalStep)
		iRolloutDepth = i_FinalRolloutDepth;

	for (int i = 1; i < iRolloutDepth; ++ i)
	{
		// cout << '.' << flush;

		// the NULL below is used to separate unit actions 	
		// from different states ...						
		dqActions.push_back (NULL);

		State* pState = new State;
		dqStates.push_back (pState);
		if (false == SimulateStep (_rSample,
								   _rGameInterface,
								   i,
								   _pReward,
								   _pScore,
								   pState,
								   &dqActions,
								   fExplorationProbability))
			break;
	}
	*_pCommand = Play::GetCompoundCommand (dqActions);


	// update parameters of all models used for action selection 	
	if ((false == _bTestMode) || (true == b_UpdateInTestMode))
	{
		pthread_rwlock_wrlock (&rwl_Model);
		UpdateParameters (_rSample, dqStates, dqActions, *_pReward);
		if (true == b_LogAllParamUpdates)
		{
			Play::WriteIterationMarker (_iIteration);
			Play::SaveParams ();
		}
		pthread_rwlock_unlock (&rwl_Model);
	}


	// cleanup...		
	ITERATE (State_dq_t, dqStates, ite)
		delete *ite;
	dqStates.clear ();
}



//												
bool MonteCarlo::SimulateStep (Sample& _rSample,
								GameInterface& _rGameInterface,
								int _iStep,
								double* _pReward,
								double* _pScore,
								State* _pState,
								Action_dq_t* _pdqActions,
								float _fExplorationProbability)
{
	String sState;
	if (false == _rGameInterface.MeasureState (sState))
	{
		cerr << "[EE] GameInterface::MeasureState () returned error." << endl;
		abort ();
		return "";
	}
	_pState->SetState (sState, &o_OperatorWordsModel);


	// for each game agent...		
	pthread_rwlock_rdlock (&rwl_Model);
	ITERATE (Agent_vec_t, _pState->vec_Agents, iteAgent)
	{
		Agent& rAgent = **iteAgent;
		if (true == rAgent.vec_Actions.empty ())
			continue;

		long iSelectedAction = -1;
		if (1 == rAgent.vec_Actions.size ())
			iSelectedAction = 0;

		else
		{
			// if this is not the first step, we always take the 	
			// continue actions...									
			if (_iStep > 0)
			{
				if (-1 != rAgent.i_ContinueActionIndex)
					iSelectedAction = rAgent.i_ContinueActionIndex;
			}

			// select action if we're not continuing a previous one	
			if (-1 == iSelectedAction)
			{
				iSelectedAction = (this->*pfn_SampleAction) (_rSample,
															 sState,
															 rAgent,
															 _fExplorationProbability);
			}
		}

		_pdqActions->push_back (&rAgent.vec_Actions [iSelectedAction]);

		_rGameInterface.SendCommand (rAgent.vec_Actions [iSelectedAction].s_Command);
	}
	pthread_rwlock_unlock (&rwl_Model);

	bool bGameFinished = _rGameInterface.EndTurn ();

	pair<double,double> pairScore = _rGameInterface.GetScore ();
	// *_pScore += pairScore.first; 
	// *_pScore = pairScore.second; 
	*_pScore = pairScore.second; 
	*_pReward = pairScore.first; 

	// if (true == bGameFinished)
	//	cout << "Score on game end : " << *_pScore << endl;

	if (true == bGameFinished)
		return false;
	return true;
}


//												
String MonteCarlo::SelectBestCommand (Sample& _rSample, 
									  CommandToScore_map_t& _rmapCommandToScore,
									  GameInterface& _rActualGame,
									  int _iStep)
{
	if (csm_trivial == e_CommandSelectionMethod)
		return Play::SelectBestCommand_Trivial (_rSample, _rmapCommandToScore);
	else if (csm_naive_bayes == e_CommandSelectionMethod)
		return Play::SelectBestCommand_NaiveBayes (_rSample, _rmapCommandToScore);
	else if (csm_q_fn == e_CommandSelectionMethod)
	{
		if (_iStep >= i_QfnSelectionStart)
			return SelectBestCommand_Qfn (_rSample, _rActualGame);
		else
			return Play::SelectBestCommand_Trivial (_rSample, _rmapCommandToScore);
	}
	else
	{
		cout << "[ERROR] Unknown command selection method in MonteCarlo::SelectBestCommand.\n"
			 << "        This should NOT happen, if it does, there's a trivial code bug."
			 << endl;
		abort ();
	}
	return String ();
}


//												
String MonteCarlo::SelectBestCommand_Qfn (Sample& _rSample, GameInterface& _rActualGame)
{
	String sState;
	if (false == _rActualGame.MeasureState (sState))
	{
		cerr << "[EE] GameInterface::MeasureState () returned error." << endl;
		abort ();
		return "";
	}
	State oState;
	oState.SetState (sState, &o_OperatorWordsModel);


	// for each game agent...		
	pthread_rwlock_rdlock (&rwl_Model);

	// perform an update of the Q function using all the datapoints	
	// we collected during the rollouts for this state...			
	if (true == b_QfnUpdateBeforeCommandSelection)
		o_LinearFunctionApprox.UpdateFunctionApprox (true);

	String sCompoundCommand;
	ITERATE (Agent_vec_t, oState.vec_Agents, iteAgent)
	{
		Agent& rAgent = **iteAgent;

		long iSelectedAction = -1;
		if (1 == rAgent.vec_Actions.size ())
			iSelectedAction = 0;
		else
			iSelectedAction = (this->*pfn_SampleAction) (_rSample, sState, rAgent, -1);

		sCompoundCommand << rAgent.vec_Actions [iSelectedAction].s_Command << '\n';
	}
	pthread_rwlock_unlock (&rwl_Model);

	sCompoundCommand.Strip ();
	return sCompoundCommand;
}



//												
void MonteCarlo::PrintStats (void)
{
	Play::PrintStats (*p_Sample);
}


