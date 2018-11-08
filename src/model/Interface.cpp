#include <assert.h>
#include <nlp_config.h>
#include <nlp_macros.h>
#include "Interface.h"
#include "learner_comms.h"

#define RECEIVE_BUFFER	81920

TextInterface*	Agent::p_TextInterface = NULL;
String_set_t	Agent::set_AttemptedAction;

/* ACTION FUNCTIONS */

Action::Action (void)
{
	p_Agent = NULL;
	i_CommandType = -1;
	i_UnitCommand = -1;
	p_SelectedSentence = NULL;
	p_SelectedHiddenState = NULL;
	b_IsArgmaxSentence = false;
}


Action::Action (const Action& _rAction)
	: o_Features (_rAction.o_Features),
	  o_AugmentedFeatures (_rAction.o_AugmentedFeatures)
{
	p_Agent = _rAction.p_Agent;
	dq_FeatureLabels = _rAction.dq_FeatureLabels;
	dq_FeatureLabelIndices = _rAction.dq_FeatureLabelIndices;
	s_Command = _rAction.s_Command;
	s_GenericCommand = _rAction.s_GenericCommand;
	s_CommandType = _rAction.s_CommandType;
	i_CommandType = _rAction.i_CommandType;
	i_UnitCommand = _rAction.i_UnitCommand;
	
	p_SelectedSentence = _rAction.p_SelectedSentence;
	p_SelectedHiddenState = _rAction.p_SelectedHiddenState;
	b_IsArgmaxSentence = _rAction.b_IsArgmaxSentence;
}

Action::~Action (void)
{
	if (NULL != p_SelectedSentence)
	{
		if (true == p_SelectedSentence->IsLocallyAllocated ())
			delete (OperatorLabeledSentence*) p_SelectedSentence;
	}
}


//													
bool Action::SetAction (char* _zAction, String_dq_t& _rdqLabels)
{
	zchar_dq_t dqValues = String::DestructiveSplit (_zAction, LCP_COMMAND_TERMINATOR);

	s_Command = dqValues [0];

	// Player actions ('p' and 'q' meta types) don't have a 
	// unit id in the command, so are generic by definition.
	// For other units, we need to remove the unit id...	
	if (('p' == p_Agent->c_MetaType) || ('q' == p_Agent->c_MetaType))
		s_GenericCommand = dqValues [0];
	else
	{
		zchar_dq_t dqCommandParams = String::DestructiveSplit (dqValues [0], ' ', 2);
		s_GenericCommand << dqCommandParams [0];
		if (dqCommandParams.size () > 2)
			s_GenericCommand << ' ' << dqCommandParams [2];
	}

	String_dq_t dqCommand;
	s_Command.Split (dqCommand, ' ', 1);
	s_CommandType = dqCommand [0];
	s_CommandType.Strip ();
	i_CommandType = RelevanceModelFeatures::GetPrefixIndex (s_CommandType);
	String sUnitAction;
	sUnitAction << p_Agent->s_UnitType << '\x01' << s_CommandType;
	i_UnitCommand = RelevanceModelFeatures::GetPrefixIndex (sUnitAction);


	// continue marker ...		
	bool bIsContinueAction = ('c' == dqValues [1][0]);

	// action features ...		
	if (2 == dqValues.size ())
	{
		o_Features.SetSize (1);
		if (true == bIsContinueAction)
			o_Features.Set (40, 1);
		return bIsContinueAction;
	}
	if ('\0' == dqValues [2][0])
	{
		o_Features.SetSize (1);
		if (true == bIsContinueAction)
			o_Features.Set (40, 1);
		return bIsContinueAction;
	}

	size_t iFeatures = 1;
	char* p = dqValues [2];
	while ('\0' != *p)
	{
		iFeatures += ((LCP_FEATURE_SEPARATOR == *p) || (LCP_BOW_SEPARATOR == *p))? 1 : 0;
		++ p;
	}
	o_Features.SetSize (iFeatures + 1);
	if (true == bIsContinueAction)
		o_Features.Set (40, 1);

	char zFeatureType [3] = "u_";
	zchar_dq_t dqFeatures = String::DestructiveSplit (dqValues [2], LCP_FEATURE_SEPARATOR);
	if (false == dqFeatures.empty ())
		s_ActionName = dqFeatures [0];
	for (size_t i = 0; i < dqFeatures.size (); ++ i)
	{
		zFeatureType [1] = '0' + i;
		SetBowFeatures (zFeatureType, dqFeatures [i], _rdqLabels);
	}

	//							
	return bIsContinueAction;
}


//													
void Action::SetBowFeatures (const char* _zPrefix, char* _zFeatures, String_dq_t& _rdqLabels)
{
	String sFeatures (_zFeatures);
	sFeatures.LowerCase ();
	zchar_dq_t dqValues = sFeatures.DestructiveSplit (LCP_BOW_SEPARATOR);
	o_Features.SetBagOfWords (_zPrefix, dqValues);

	ITERATE (zchar_dq_t, dqValues, ite)
	{
		String sPrefix (*ite);
		sPrefix.Strip ();
		_rdqLabels.push_back (sPrefix);
		int iWordIndex = RelevanceModelFeatures::FindWordIndex (GetPorterStem (sPrefix));
		if (-1 != iWordIndex)
			dq_FeatureLabels.push_back (iWordIndex);
		dq_FeatureLabelIndices.push_back (RelevanceModelFeatures::GetPrefixIndex (sPrefix));
	}
}


/* AGENT FUNCTIONS */


Agent::Agent (char _cMetaType)
{
	c_MetaType = _cMetaType;
	i_ContinueActionIndex = -1;
	i_UnitType = -1;
}

Agent::Agent (const Agent& _rAgent)
	: o_Features (_rAgent.o_Features)
{
	vec_Actions = _rAgent.vec_Actions;
	dq_NewActions = _rAgent.dq_NewActions;
	dq_FeatureLabels = _rAgent.dq_FeatureLabels;
	dq_FeatureLabelIndices = _rAgent.dq_FeatureLabelIndices;
	i_ContinueActionIndex = _rAgent.i_ContinueActionIndex;
	c_MetaType = _rAgent.c_MetaType;
	s_UnitType = _rAgent.s_UnitType;
	i_UnitType = _rAgent.i_UnitType;
	p_FilteredSentences = _rAgent.p_FilteredSentences;
}

Agent::~Agent (void)
{
	vec_Actions.clear ();
	dq_NewActions.clear ();
}


//													
void Agent::SetActions (char* _zActions, String_dq_t& _rdqLabels)
{
	zchar_dq_t dqActionsRaw = String::DestructiveSplit (_zActions, LCP_ACTION_TERMINATOR);
	if (0 == dqActionsRaw.size ())
	{
		cerr << "[EE] No actions available : " << _zActions << endl;
		abort ();
	}

	size_t iActions = dqActionsRaw.size ();
	// vec_Actions.resize (iActions, Action ());
	vec_Actions.resize (iActions);

	for (size_t i = 0; i < iActions; ++ i)
	{
		vec_Actions [i].p_Agent = this;
		if (true == vec_Actions [i].SetAction (dqActionsRaw [i], _rdqLabels))
			i_ContinueActionIndex = i;
		if (set_AttemptedAction.end () ==
				set_AttemptedAction.find (vec_Actions [i].s_GenericCommand))
		{
			dq_NewActions.push_back (i);
		}
	}
}


//													
void Agent::SetBowFeatures (const char* _zPrefix, char* _zFeatures, String_dq_t& _rdqLabels)
{
	String sFeatures (_zFeatures);
	sFeatures.LowerCase ();
	zchar_dq_t dqValues = sFeatures.DestructiveSplit (LCP_BOW_SEPARATOR);
	o_Features.SetBagOfWords (_zPrefix, dqValues, 1);

	ITERATE (zchar_dq_t, dqValues, ite)
	{
		String sPrefix (*ite);
		sPrefix.Strip ();
		_rdqLabels.push_back (sPrefix);
		int iWordIndex = RelevanceModelFeatures::FindWordIndex (GetPorterStem (sPrefix));
		if (-1 != iWordIndex)
			dq_FeatureLabels.push_back (iWordIndex);
		dq_FeatureLabelIndices.push_back (RelevanceModelFeatures::GetPrefixIndex (sPrefix));
	}
}


//													
float Agent::Truncate (float _fValue, float _fScale)
{
	float f = _fValue / _fScale;
	if (f > 1)
		f = 1;
	else if (f < -1)
		f = -1;
	return f;
}


//													
bool Agent::SetFeatures (char* _zFeatures, Agent* _pNation, String_dq_t& _rdqLabels)
{
	if ('\0' == *_zFeatures)
		return false;
	assert (true == dq_FeatureLabels.empty ());

	// String sFeatureCopy (_zFeatures);
	size_t iFeatures = 1;
	char* p = _zFeatures;
	while ('\0' != *p)
	{
		iFeatures += ((LCP_FEATURE_SEPARATOR == *p) || (LCP_BOW_SEPARATOR == *p))? 1 : 0;
		++ p;
	}
	// size_t iFeatureCountCopy = iFeatures;

	// cout << '[' << _zFeatures << ']' << endl;
	zchar_dq_t dqValues = String::DestructiveSplit (_zFeatures, LCP_FEATURE_SEPARATOR);
	if ('p' == c_MetaType)
	{
		s_UnitType = "government";
		i_UnitType = RelevanceModelFeatures::GetPrefixIndex (s_UnitType);

		o_Features.SetSize (iFeatures);
		o_Features.Set (0, atof (dqValues [0]));			// % settled world			
		o_Features.Set (1, atof (dqValues [1]));			// % known world			
		o_Features.Set (2, atof (dqValues [2]) / 100);		// my game score			
		o_Features.Set (3, atof (dqValues [3]) / 100);		// opponent's game score	
		o_Features.Set (4, atof (dqValues [4]) / 100);		// total cities				
		o_Features.Set (5, atof (dqValues [5]) / 100);		// total of city sizes		
		o_Features.Set (6, atof (dqValues [6]) / 10);		// average city size		
		o_Features.Set (7, atof (dqValues [7]) / 100);		// total units				
		o_Features.Set (8, atof (dqValues [8]) / 100);		// total veteran units		
		o_Features.Set (9, atof (dqValues [9]) / 1000);		// total gold				
		o_Features.Set (10, atof (dqValues [10]) / 100);	// excess food				
		o_Features.Set (11, atof (dqValues [11]) / 100);	// excess shield			
		o_Features.Set (12, atof (dqValues [12]) / 100);	// excess trade				
		o_Features.Set (13, atof (dqValues [13]) / 100);	// excess science			
		o_Features.Set (14, atof (dqValues [14]) / 100);	// excess gold				
		o_Features.Set (15, atof (dqValues [15]) / 100);	// excess luxury			
		o_Features.Set (16, atof (dqValues [16]));			// not anarchy/revolution	
		SetBowFeatures ("p", dqValues [17], _rdqLabels);
		if (dqValues.size () > 18)
			SetBowFeatures ("t", dqValues [18], _rdqLabels);
	}
	else if ('q' == c_MetaType)
	{
		s_UnitType = "research";
		i_UnitType = RelevanceModelFeatures::GetPrefixIndex (s_UnitType);

		++ iFeatures;	// we locally compute one extra feature for techs...
		if (NULL != _pNation)
		{
			iFeatures += _pNation->o_Features.Size ();
			o_Features.SetSize (iFeatures);
			o_Features.Set (_pNation->o_Features);
		}
		else
			o_Features.SetSize (iFeatures);

		float fRatioComplete = atof (dqValues [0]);
		if (fRatioComplete > 1)
			cout << "[ERROR] research ratio complete > 1! : " << fRatioComplete << endl;
		o_Features.Set (17, fRatioComplete);
		o_Features.Set (18, 1 - fRatioComplete);
		int iTurnsToCompletion = atoi (dqValues [1]);
		if (-1 != iTurnsToCompletion)
			o_Features.Set (19, iTurnsToCompletion / (float)100);
		else
			o_Features.Set (20, 1);

		if (dqValues.size () > 2)
			SetBowFeatures ("r", dqValues [2], _rdqLabels);
	}
	else if ('b' == c_MetaType)
	{
		s_UnitType = "city";
		i_UnitType = RelevanceModelFeatures::GetPrefixIndex (s_UnitType);

		if (NULL != _pNation)
			iFeatures += _pNation->o_Features.Size ();
		o_Features.SetSize (iFeatures - 3);

		o_Features.Set (21, atof (dqValues [3]));
		int iTurnsToGrow = atoi (dqValues [4]);
		if (-1 != iTurnsToGrow)
			o_Features.Set (22, iTurnsToGrow / (float)10);
		else
			o_Features.Set (23, 1);
		o_Features.Set (24, atof (dqValues [5]));
		o_Features.Set (25, Truncate (atof (dqValues [6]), 1000));
		int iTurnsToBuild = atoi (dqValues [7]);
		if (-1 != iTurnsToBuild)
			o_Features.Set (26, iTurnsToBuild / (float)100);
		else
			o_Features.Set (27, 1);
		o_Features.Set (28, atof (dqValues [8]));
		o_Features.Set (29, atof (dqValues [9]));
		o_Features.Set (30, atof (dqValues [10]));
		o_Features.Set (31, atof (dqValues [11]));
		o_Features.Set (32, atof (dqValues [12]));
		o_Features.Set (33, atof (dqValues [13]));
		o_Features.Set (34, atof (dqValues [14]));
		o_Features.Set (35, atof (dqValues [15]));

		SetBowFeatures ("cg", dqValues [16], _rdqLabels);
		SetBowFeatures ("cb", dqValues [17], _rdqLabels);
		if (dqValues.size () > 18)
			SetBowFeatures ("ci", dqValues [18], _rdqLabels);

		if (NULL != _pNation)
			o_Features.Set (_pNation->o_Features);
	}
	else if ('u' == c_MetaType)
	{
		s_UnitType = dqValues [10];	// name of unit type
		s_UnitType.Strip ();
		s_UnitType.LowerCase ();
		i_UnitType = RelevanceModelFeatures::GetPrefixIndex (s_UnitType);

		if (NULL != _pNation)
			iFeatures += _pNation->o_Features.Size ();
		o_Features.SetSize (iFeatures - 5);

		o_Features.Set (36, atof (dqValues [3]));
		o_Features.Set (37, atof (dqValues [4]));
		o_Features.Set (38, atof (dqValues [4]) / atof (dqValues [5]));
		o_Features.Set (39, atof (dqValues [6]));
		// float fDistanceToNearestCity = atof (dqValues [7]);
		// o_Features.Set (19, fDistanceToNearestCity);
		// o_Features.Set (20, atof (dqValues [8]));
		if (-1 != atoi (dqValues [9]))
		{
			// o_Features.Set (21, 1);
			SetBowFeatures ("uc", dqValues [10], _rdqLabels);
		}

		SetBowFeatures ("un", dqValues [10], _rdqLabels);
		// SetBowFeatures ("ud", dqValues [10], fDistanceToNearestCity);
		SetBowFeatures ("ua", dqValues [11], _rdqLabels);
		SetBowFeatures ("ut", dqValues [12], _rdqLabels);
		if (dqValues.size () > 13)
			SetBowFeatures ("uo", dqValues [13], _rdqLabels);

		if (NULL != _pNation)
			o_Features.Set (_pNation->o_Features);
	}

	if (NULL != p_TextInterface)
		p_FilteredSentences = p_TextInterface->GetFilteredSentences (s_UnitType);
	return ('p' == c_MetaType);
}


/* STATE FUNCTIONS */

State::~State (void)
{
    // desctructor
	ITERATE (Agent_vec_t, vec_Agents, ite)
		delete *ite;
	vec_Agents.clear ();
}


//													
void State::SetState (String& _rState, OperatorWords* _pOperatorModel)
{
	s_State = _rState;

	// for each game agent...		
	zchar_dq_t dqAgents = _rState.DestructiveSplit (LCP_AGENT_TERMINATOR);
	vec_Agents.reserve (dqAgents.size ());
	Agent* pAgent = NULL;

	String_dq_t dqStateLabels;
	String_dq_t dqUnitLabels;
	String_dq_t dqActionLabels;

	char cPreviousAgentType = ' ';
	String sPreviousAgentInfo;
	Agent* pNation = NULL;
	ITERATE (zchar_dq_t, dqAgents, iteAgent)
	{
		char* pzAgent = *iteAgent;
		if (0 == strlen (pzAgent))
			continue;

		// agent type ...			
		if (LCP_TYPE_MARKER == *pzAgent)
		{
			cPreviousAgentType = pzAgent [1];
			// vec_Agents.push_back (Agent (pzAgent [1]));
			pAgent = new Agent (pzAgent [1]);
			vec_Agents.push_back (pAgent);
			// vec_Agents.push_back (pzAgent [1]);
			// pAgent = &vec_Agents.back ();
			continue;
		}

		// agent features ...		
		if (LCP_INFO_MARKER == *pzAgent)
		{
			sPreviousAgentInfo = (pzAgent + 1);
			if (true == pAgent->SetFeatures (pzAgent + 1, pNation, dqStateLabels))
				// the "nation" agent is listed first in the	
				// message packet, so getting it this way and	
				// passing it back to the SetFeatures call for	
				// the remaining agents is ok...				
				pNation = pAgent;
			continue;
		}

		// for cities, we have two "agents" 
		// both with the same type & info	
		if (NULL == pAgent)
		{
			assert (' ' != cPreviousAgentType);
			// vec_Agents.push_back (Agent (cPreviousAgentType));
			// pAgent = &vec_Agents.back ();
			pAgent = new Agent (cPreviousAgentType);
			vec_Agents.push_back (pAgent);
			assert (NULL != pNation);
			pAgent->SetFeatures ((char*)(const char*)sPreviousAgentInfo, 
								 pNation,
								 dqStateLabels);
		}
	
		// set actions ...			
		pAgent->SetActions (pzAgent, dqStateLabels);

		// collect state, unit & action labels...
		dqUnitLabels.push_back (pAgent->s_UnitType);
		ITERATE (Action_vec_t, pAgent->vec_Actions, iteAction)
			dqActionLabels.push_back (iteAction->s_ActionName);

		pAgent = NULL;
	}

	if (NULL != _pOperatorModel)
		_pOperatorModel->AddLabels (dqStateLabels, dqUnitLabels, dqActionLabels);

	// we destroy the state variable above, so	
	// restore it before returning...			
	_rState = s_State;
}



/* GAMEINTERFACE FUNCTIONS */

GameInterface::GameInterface (void)
{
	Socket::SetStandalone (true);
	b_RatioOfSums = false;
	b_DiffScore = false;
}


//													
void GameInterface::Send (String _sMessage)
{
	// cout << _sMessage << endl;
	Socket::SendBlocking (_sMessage, _sMessage.length ());
}


//													
String GameInterface::GetResponse (void)
{
	char zData [RECEIVE_BUFFER + 1];
	memset (zData, 0, RECEIVE_BUFFER + 1);
	while (false == o_Data.HasTerminator (LCP_TERMINATOR))
	{
		long lBytes = Socket::ReceiveBlocking (zData, RECEIVE_BUFFER, 100);

		if (false == Socket::IsConnected ())
			return "";
		if (lBytes == 0)
			return "";
		if (lBytes > 0)
			o_Data.Append (zData, lBytes);
	}

	String sMessage = o_Data.PopFirstMessageAsString (LCP_TERMINATOR);

	// cout << sMessage << endl;
	if (true == sMessage.Has ("ERROR"))
		cout << sMessage << endl;

	// dq_ReceivedMsgs.push_back (sMessage);
	// if (dq_ReceivedMsgs.size () > 10)
	//	dq_ReceivedMsgs.pop_front ();
	return sMessage.substr (0, sMessage.length () - 1);
}


//													
bool GameInterface::Init (String _sServer)
{
	LinearQFnFeatures::SetBagOfWordsOffset (41);
	dq_ReceivedMsgs.clear ();
	Connect (_sServer);
	d_MaxCompoundScore = (config)"max_compound_score";
	d_GameInternalScoreWeight = (config)"game_internal_score_weight";
	b_RatioOfSums = (1 == (int)(config)"ratio_of_sums_reward");
	b_DiffScore = (1 == (int)(config)"diff_of_scores_reward");
	return true;
}


//													
bool GameInterface::Connect (String _sServer)
{
    /*
     * Connect to the given input server
     */

	// cout << "Connecting to civ server [" << _sServer << "]..." << endl;
	String sServer = (config)(_sServer + "_game_host");
	String sPort = (config)(_sServer + "_game_service");

	if (("localhost" != sServer) || (true == sPort.IsDigit ()))
	{
		if (false == ClientSocket::ConnectBlocking (sServer, sPort))
		{
			cout << "   [EE] Failed to connect to game server [" << _sServer << "]." << endl;
			return false;
		}
	}
	else
	{
		if (false == ClientSocket::ConnectUnixDomainBlocking (sPort))
		{
			cout << "   [EE] Failed to connect to game server [" << _sServer << "]." << endl;
			return false;
		}
	}

	Socket::SetNoDelay (true);
	ClearConnection (false);

	String sTimeout (LCP_SET_TIMEOUT " ");
	if ("main" == _sServer)
		sTimeout << (config)"main_client_timeout" << '\n';
	else
		sTimeout << (config)"simulation_client_timeout" << '\n';
	Send (sTimeout);

	Send (LCP_CONNECT "\n");
	GetResponse ();
	// cout << "   - connected" << endl;
	cout << '>' << flush;

	b_AlwaysWaitForResponse = (1 == (int)(config)"always_wait_for_response");
	if (true == b_AlwaysWaitForResponse)
		Send ("ack on\n");

	return true;
}


//													
void GameInterface::Disconnect (void)
{
	// if (true == Socket::IsConnected ())
	//	Send (".quit\n");
	Socket::Close ();
}


//													
void GameInterface::EnableEarlyGameEndDetection (void)
{
	Send (LCP_ENABLE_EARLY_END "\n");
}


//													
bool GameInterface::MeasureState (String& _rState)
{
	Send (LCP_OBSERVE "\n");
	_rState = GetResponse ();
	return ("" != _rState);
}


//													
bool GameInterface::SendCommand (String& _rCommand)
{
	if (true == b_AlwaysWaitForResponse)
	{
		Send (_rCommand + "\n");
		GetResponse ();
	}
	else
		Send (_rCommand + "\n");
	return true;
}


//													
bool GameInterface::EndTurn (void)
{
	if ("" != s_SendBuffer)
	{
		s_SendBuffer << LCP_TURNDONE "\n";
		Send (s_SendBuffer);
		s_SendBuffer = "";
	}
	else
		Send (LCP_TURNDONE "\n");

	String sResponse = GetResponse ();
	while (true == sResponse.Has ("ERROR"))
		sResponse = GetResponse ();

	if (true == sResponse.StartsWith (LCP_GAMEFINISHED))
		return true;
	return false;
}


//													
double GameInterface::GetScoreRatio (GameScores_dq_t& dqPlayers, int _iIndex)
{
	// Currently this function is setup to compute	
	// a score difference & not a score ratio.		
	// If this is changed to a score ratio, GetScore
	// also needs to be modified - particularly the	
	// score returned by GetScore for a game loss.	
	double dMe = dqPlayers [0][_iIndex];

	// the game score is always non-negative...		
	double dMax = -10000;
	for (size_t i = 1; i < dqPlayers.size (); ++ i)
	{
		int iValue = dqPlayers [i][_iIndex];
		if (dMax < iValue)
			dMax = iValue;
	}
	// return (dMe - dMax);

	// these values are integers, so substituting	
	// 0.1 for 0 should still produce a reasonable	
	// score as far as the game is concerned...		
	if (0 == dMe)
		return 0;
	if (0 == dMax)
		dMax = 0.1;

	double dRatio = dMe / dMax;
	assert (dRatio >= 0);
	// if (dRatio > 10)
	// 	dRatio = 10;
	// else if (dRatio < -10)
	// 	dRatio = -10;
	if (dRatio > 100)
		dRatio = 100;
	else if (dRatio < 0)
		dRatio = 0;
	return dRatio;
}


//													
double GameInterface::GetOpponentScore (GameScores_dq_t& _dqPlayers, int _iIndex)
{
	double dMax = _dqPlayers [1][_iIndex];
	for (size_t i = 2; i < _dqPlayers.size (); ++ i)
	{
		double dValue = _dqPlayers [i][_iIndex];
		if (dMax < dValue)
			dMax = dValue;
	}
	return dMax;
}


//													
pair<double,double> GameInterface::GetScore (void)
{
	if (true == b_DiffScore)
	{
		abort ();
		// return GetScore_Diff ();
	}
	else if (true == b_RatioOfSums)
	{
		abort ();
		// return GetScore_RatioOfSums ();
	}
	else
		return GetScore_SumOfRatios ();
}


//													
pair<double,double> GameInterface::GetScore_SumOfRatios (void)
{
	GameScores_dq_t	dqScores;

	while (true)
	{
		dqScores.clear ();

		Send (LCP_GAMESCORE "\n");
		String sScore = GetResponse ();
		while (true == sScore.Has ("ERROR"))
			sScore = GetResponse ();

		String_dq_t dqPlayers;
		sScore.Split (dqPlayers, LCP_AGENT_TERMINATOR);

		if (dqPlayers.size () < 2)
			continue;

		bool bValidScores = true;
		dqScores.resize (dqPlayers.size () - 1);
		for (size_t i = 0; i < dqPlayers.size () - 1; ++ i)
		{
			dqPlayers[i].Split (dqScores[i], ',');
			bValidScores &= (36 == dqScores [i].size ());
		}

		if (true == bValidScores)
			break;
	}

	double dMyScore = dqScores [0][0];
	double dOpponentScore = -1000;
	for (size_t i = 1; i < dqScores.size (); ++ i)
	{
		int iValue = dqScores [i][0];
		if (dOpponentScore < iValue)
			dOpponentScore = iValue;
	}
	double dScoreDiff = dMyScore - dOpponentScore;

	double dScore = GetScoreRatio (dqScores, 0);
	double dTotalUnits = GetScoreRatio (dqScores, 25);
	double dTotalCities = GetScoreRatio (dqScores, 26);
	double dTotalTechs = GetScoreRatio (dqScores, 27);
	double dSurplusFood = GetScoreRatio (dqScores, 28);
	double dSurplusShield = GetScoreRatio (dqScores, 29);
	double dSurplusTrade = GetScoreRatio (dqScores, 30);
	double dSurplusGold = GetScoreRatio (dqScores, 31);
	double dSurplusLuxury = GetScoreRatio (dqScores, 32);
	double dSurplusScience = GetScoreRatio (dqScores, 33);

	int iIsAlive = dqScores [0][1];
	int iNotAnarchy = dqScores [0][34];
	int iGameWon = dqScores [0][35];


	// Check for player death, and return -1	
	// The lines below need to tally with 		
	// whether GetScoreRatio returns a ratio or	
	// a difference.  Otherwise player behaviour
	// can become inexplicably weird ...		
	if (1 == iGameWon)
		return make_pair (1000, dScoreDiff);
	if ((1 != iIsAlive) || (-1 == iGameWon))
		// return make_pair (-1000, dScore);
		return make_pair (0, dScoreDiff);

	double dCompoundScore = 0.1 * (d_GameInternalScoreWeight * dScore +
									dTotalUnits + 2 * dTotalCities + dTotalTechs +
									1.0 * (dSurplusFood + dSurplusShield + dSurplusTrade + 
										   dSurplusGold + dSurplusLuxury + dSurplusScience) +
									0.5 * iNotAnarchy);

	return make_pair (dCompoundScore, dScoreDiff);
}


//													
pair<double,double> GameInterface::GetScore_RatioOfSums (void)
{
	GameScores_dq_t	dqScores;

	while (true)
	{
		dqScores.clear ();

		Send (LCP_GAMESCORE "\n");
		String sScore = GetResponse ();
		while (true == sScore.Has ("ERROR"))
			sScore = GetResponse ();

		String_dq_t dqPlayers;
		sScore.Split (dqPlayers, LCP_AGENT_TERMINATOR);

		if (dqPlayers.size () < 3)
			continue;

		bool bValidScores = true;
		dqScores.resize (dqPlayers.size () - 1);
		for (size_t i = 0; i < dqPlayers.size () - 1; ++ i)
		{
			dqPlayers[i].Split (dqScores[i], ',');
			bValidScores &= (36 == dqScores [i].size ());
		}

		if (true == bValidScores)
			break;
	}


	double dScore = GetScoreRatio (dqScores, 0);

	double dMyReward = 0.1 * (10 * (double)dqScores[0][0] + (double)dqScores[0][25] +
									2 * (double)dqScores[0][26] + (double)dqScores[0][27] +
								0.3 * ((double)dqScores[0][28] + (double)dqScores[0][29] +
										(double)dqScores[0][30] + (double)dqScores[0][31] +
										(double)dqScores[0][32] + (double)dqScores[0][33]) +
								0.5 * (double)dqScores[0][34]);

	double dOpponentReward = 0.1 * (10 * GetOpponentScore (dqScores,0) + GetOpponentScore (dqScores,25) +
									2 * GetOpponentScore (dqScores,26) + GetOpponentScore (dqScores,27) +
								0.3 * (GetOpponentScore (dqScores,28) + GetOpponentScore (dqScores,29) +
										GetOpponentScore (dqScores,30) + GetOpponentScore (dqScores,31) +
										GetOpponentScore (dqScores,32) + GetOpponentScore (dqScores,33)) +
								0.5 * GetOpponentScore (dqScores,34));

	double dCompoundScore = 0;
	if (0 == dOpponentReward)
		dOpponentReward = 0.1;
	if (0 != dMyReward)
	{
		dCompoundScore = dMyReward / dOpponentReward;
		assert (dCompoundScore >= 0);
		if (dCompoundScore > d_MaxCompoundScore)
			dCompoundScore = d_MaxCompoundScore;
		else if (dCompoundScore < 0)
			dCompoundScore = 0;
	}

	int iIsAlive = dqScores [0][1];
	int iGameWon = dqScores [0][35];
	// cout << iIsAlive << ' ' << iGameWon << endl;

	// Check for player death, and return -1	
	// The lines below need to tally with 		
	// whether GetScoreRatio returns a ratio or	
	// a difference.  Otherwise player behaviour
	// can become inexplicably weird ...		
	if (1 == iGameWon)
		return make_pair (d_MaxCompoundScore, dScore);
	if ((1 != iIsAlive) || (-1 == iGameWon))
		return make_pair (0, dScore);

	return make_pair (dCompoundScore, dScore);
}


//													
pair<double,double> GameInterface::GetScore_Diff (void)
{
	GameScores_dq_t	dqScores;

	while (true)
	{
		dqScores.clear ();

		Send (LCP_GAMESCORE "\n");
		String sScore = GetResponse ();
		while (true == sScore.Has ("ERROR"))
			sScore = GetResponse ();

		String_dq_t dqPlayers;
		sScore.Split (dqPlayers, LCP_AGENT_TERMINATOR);

		if (dqPlayers.size () < 3)
			continue;

		bool bValidScores = true;
		dqScores.resize (dqPlayers.size () - 1);
		for (size_t i = 0; i < dqPlayers.size () - 1; ++ i)
		{
			dqPlayers[i].Split (dqScores[i], ',');
			bValidScores &= (36 == dqScores [i].size ());
		}

		if (true == bValidScores)
			break;
	}


	// double dScore = GetScoreRatio (dqScores, 0);
	double dMyScore = dqScores [0][0];
	double dOpponentScore = -1000;
	for (size_t i = 1; i < dqScores.size (); ++ i)
	{
		int iValue = dqScores [i][0];
		if (dOpponentScore < iValue)
			dOpponentScore = iValue;
	}
	double dScore = dMyScore - dOpponentScore;


	double dMyReward = 0.1 * (10 * (double)dqScores[0][0] + (double)dqScores[0][25] +
									2 * (double)dqScores[0][26] + (double)dqScores[0][27] +
								0.3 * ((double)dqScores[0][28] + (double)dqScores[0][29] +
										(double)dqScores[0][30] + (double)dqScores[0][31] +
										(double)dqScores[0][32] + (double)dqScores[0][33]) +
								0.5 * (double)dqScores[0][34]);

	double dOpponentReward = 0.1 * (10 * GetOpponentScore (dqScores,0) + GetOpponentScore (dqScores,25) +
									2 * GetOpponentScore (dqScores,26) + GetOpponentScore (dqScores,27) +
								0.3 * (GetOpponentScore (dqScores,28) + GetOpponentScore (dqScores,29) +
										GetOpponentScore (dqScores,30) + GetOpponentScore (dqScores,31) +
										GetOpponentScore (dqScores,32) + GetOpponentScore (dqScores,33)) +
								0.5 * GetOpponentScore (dqScores,34));

	double dCompoundScore = (dMyReward - dOpponentReward) / dMyReward;

	int iIsAlive = dqScores [0][1];
	int iGameWon = dqScores [0][35];
	// cout << iIsAlive << ' ' << iGameWon << endl;

	// Check for player death, and return -1	
	// The lines below need to tally with 		
	// whether GetScoreRatio returns a ratio or	
	// a difference.  Otherwise player behaviour
	// can become inexplicably weird ...		
	if (1 == iGameWon)
		return make_pair (d_MaxCompoundScore, dScore);
	if ((1 != iIsAlive) || (-1 == iGameWon))
		return make_pair (-d_MaxCompoundScore, dScore);

	return make_pair (dCompoundScore, dScore);
}


//													
bool GameInterface::SaveGame (String& _rSaveName)
{
	Send (LCP_SAVEGAME " " + _rSaveName + "\n");
	GetResponse ();
	return true;
}


//													
bool GameInterface::LoadGame (String& _rSaveName)
{
	Send (LCP_LOADGAME " " + _rSaveName + "\n");
	GetResponse ();
	return true;
}


//													
void GameInterface::ClearConnection (bool _bWarn)
{
	char zData [RECEIVE_BUFFER + 1];
	long lBytes = ReceiveBlocking (zData, RECEIVE_BUFFER, 100);
	long lTotalBytes = lBytes;

	while (lBytes > 0)
	{
		long lBytes = ReceiveBlocking (zData, RECEIVE_BUFFER, 100);
		if (lBytes > 0)
			lTotalBytes += lBytes;
	}

	if ((true == _bWarn) && (lTotalBytes > 0))
		cout << "[INFO] Discarding " << lTotalBytes << " of unexpected data." << endl;
}


//													
void GameInterface::OnReceive (const void* _zData, long _lBytes)
{
	if (0 == _lBytes)
		return;
	char zCopy [_lBytes + 1];
	memcpy (zCopy, _zData, _lBytes);
	zCopy [_lBytes] = '\0';

	cout << "[EE] OnReceived called.  This should not happen, "
			"all data is supposed to be received in a blocking "
			"fashion. " << endl
		 << "Data [" << _lBytes << "] " << zCopy << endl;
	// assert (false);
}


//													
void GameInterface::OnDisconnect (void)
{
	cout << "[EE] Socket connection to game server lost." << endl;
}

