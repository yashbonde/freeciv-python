#include <nlp_config.h>
#include <nlp_macros.h>
#include <nlp_time.h>
#include <nlp_filesystem.h>
#include "Game.h"
#include "learner_comms.h"


//									
Game::Game (void)
{
}


//									
Game::~Game (void)
{
	// o_GameInterface.Disconnect ();
}


//									
bool Game::Init (void)
{
	cout << "Connecting to game " << flush;
	if (false == o_GameInterface.Init ("main"))
		return false;
	cout << endl;
	if (1 == (int)(config)"enable_early_game_end_detection")
	{
		cout << "   enabling early game end detection." << endl;
		o_GameInterface.EnableEarlyGameEndDetection ();
	}

	i_MaxSteps = (config)"max_game_steps";
	s_InstanceName = (config)"instance_name";

	String sPlayMode = (config)"play_mode";
	if ("random" == sPlayMode)
		e_PlayMode = pm_random;
	else if ("text" == sPlayMode)
		e_PlayMode = pm_text;
	else if ("monte-carlo" == sPlayMode)
		e_PlayMode = pm_montecarlo;
	else
	{
		cout << "[ERROR] Unknown value for play_mode : ["
			 << sPlayMode << ']' << endl;
		return false;
	}
	return true;
}


//									
void Game::Close (void)
{
	o_GameInterface.Disconnect ();
}


//									
pair<double,double> Game::Play (MonteCarlo& _rMontecarlo)
{
	String sGameName = (config)"game_file_name";
	if ("" != sGameName)
	{
		cout << "   loading game : " << sGameName << endl;
		o_GameInterface.LoadGame (sGameName);
		sleep (2);
	}

	bool bTestMode = (1 == (int)(config)"test_mode");

	Time oTimer;
	oTimer.StartTimer ();
	for (int i = 0; i <= i_MaxSteps; ++ i)
	{
		if ((i == i_MaxSteps) && (0 == _rMontecarlo.i_FinalRollouts))
			break;

		cout << i << endl;
		String sStepName;
		sStepName << s_InstanceName << "_" << i;

		String sCommand;
		if (pm_montecarlo == e_PlayMode)
		{
			o_GameInterface.SaveGame (sStepName);
			sleep (1);
			int iStepsToFinalRollout = (i_MaxSteps - i);
			sCommand = _rMontecarlo.SelectCommand (o_GameInterface,
												   sStepName, 
												   i,
												   bTestMode,
												   iStepsToFinalRollout);
		}
		else if (pm_text == e_PlayMode)
			sCommand = SelectCommandUsingText (_rMontecarlo);
		else if (pm_random == e_PlayMode)
			sCommand = SelectRandomCommand ();
		else
			abort ();


		o_GameInterface.SendCommand (sCommand);
		bool bGameFinished = o_GameInterface.EndTurn ();

		String sCopy (sCommand);
		sCopy.Replace ("\n", ", ");
		cout << "   " << sCopy << endl;

		pair<double,double> pairScore = o_GameInterface.GetScore ();
		cout << "step " << i+1 << '/' << i_MaxSteps 
			 << ", score (" << pairScore.first << ", " << pairScore.second
			 << "), [" << oTimer.sTimeToCompletion (i+1, i_MaxSteps) << ']'
			 << endl;

		if (true == bGameFinished)
			break;
	}

	// copy end game to output path ...
	Time oNow;
	String sStepName;
	sStepName << s_InstanceName << "_end_" << oNow.sDateTime ();
	o_GameInterface.SaveGame (sStepName);

	String sGameFile;
	sGameFile << (config)"runin_path"
			  << sStepName << ".sav";
	
	String sTarget ((config)"end_game_path");
	cout << "copying end game file from ["
		 << sStepName << ".sav] to [" << sTarget
		 << ']' << endl;
	if (true == Path::Exists (sTarget))
		Path::RemoveFile (sTarget);
	Path::MoveFile (sGameFile, sTarget);


	pair<double,double> pairScore = o_GameInterface.GetScore ();
	return pairScore;
}


//									
String Game::SelectRandomCommand (void)
{
	String sState;
	if (false == o_GameInterface.MeasureState (sState))
	{
		cerr << "[EE] GameInterface::MeasureState () returned error." << endl;
		abort ();
		return "";
	}
	State oState;
	oState.SetState (sState);

	Sample oSample;
	oSample.Init ();

	String sCompoundAction;
	ITERATE (Agent_vec_t, oState.vec_Agents, iteAgent)
	{
		Agent& rAgent = **iteAgent;

		long iSelectedAction = -1;
		if (1 == rAgent.vec_Actions.size ())
			iSelectedAction = 0;
		else
			iSelectedAction = oSample.SampleUniformCategorical (rAgent.vec_Actions.size ());

		sCompoundAction << rAgent.vec_Actions [iSelectedAction].s_Command << '\n';
	}
	oSample.Destroy ();

	return sCompoundAction;
}



//									
String Game::SelectCommandUsingText (MonteCarlo& _rMontecarlo)
{
	String sState;
	if (false == o_GameInterface.MeasureState (sState))
	{
		cerr << "[EE] GameInterface::MeasureState () returned error." << endl;
		abort ();
		return "";
	}
	State oState;
	oState.SetState (sState);

	Sample oSample;
	oSample.Init ();

	String sCompoundAction;
	ITERATE (Agent_vec_t, oState.vec_Agents, iteAgent)
	{
		Agent& rAgent = **iteAgent;

		long iSelectedAction = -1;
		if (1 == rAgent.vec_Actions.size ())
			iSelectedAction = 0;
		else
			iSelectedAction = Play::SampleAction_TextInfo (oSample, sState, rAgent, 0);

		sCompoundAction << rAgent.vec_Actions [iSelectedAction].s_Command << '\n';
	}
	oSample.Destroy ();

	return sCompoundAction;
}






