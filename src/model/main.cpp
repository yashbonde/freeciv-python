#include <iostream>
#include <nlp_config.h>
#include <nlp_filesystem.h>
#include "Montecarlo.h"
#include "Game.h"

extern void DumpSource (void);


String GetFileName (String _sPath)
{
	String sTarget;
	size_t iNameStart = _sPath.ReverseFind ("/");
	if (string::npos != iNameStart)
		sTarget << _sPath.substr (iNameStart);
	else
		sTarget << _sPath;
	return sTarget;
};


int main (int argc, const char* argv[])
{
	Config::SetCommandLine (argc, argv);
	if (1 == (int)(config)"dump_source")
	{
		DumpSource ();
		return 0;
	}

	bool bTestMode = (1 == (int)(config)"test_mode");
	if (false == bTestMode)
	{
		// copy config file to output path ...	
		{
			String sConfigFile (Config::GetConfigFileName ());
			String sTarget;
			sTarget << (config)"output_path" << '/'
					<< GetFileName (sConfigFile);
			if (true == Path::Exists (sTarget))
				Path::RemoveFile (sTarget);
			Config::WriteConfig (sTarget);
		}

		// copy binary to output path ...		
		{
			String sTarget;
			sTarget << (config)"output_path" << '/'
					<< GetFileName (argv [0]);
			if (true == Path::Exists (sTarget))
				Path::RemoveFile (sTarget);
			Path::CopyFile (argv [0], sTarget);
		}
	}

	cout << "game steps    : " << (config)"max_game_steps" << endl;
	cout << "rollouts      : " << (config)"max_rollouts" << endl;
	cout << "rollout depth : " << (config)"max_rollout_depth" << endl;
	cout << endl;

	String::InitPorterStemmer ();
	Random::Init ();
	
	MonteCarlo o_Montecarlo;
	Game o_Game;

	if (false == o_Montecarlo.Init ())
		return 1;
	if (false == o_Game.Init ())
		return 1;

	pair<double,double> pairScore = o_Game.Play (o_Montecarlo);

	o_Montecarlo.PrintStats ();
	cout << endl;
	cout << "game steps    : " << (config)"max_game_steps" << endl;
	cout << "rollouts      : " << (config)"max_rollouts" << endl;
	cout << "rollout depth : " << (config)"max_rollout_depth" << endl;
	cout << endl;
	cout << "game score : " << pairScore.first << ", " << pairScore.second << endl;
	cout << "load game  : " << o_Montecarlo.o_Timer2.sStartStopTime () << endl;
	cout << "simulation : " << o_Montecarlo.o_Timer1.sStartStopTime () << endl;
	cout << endl;

	o_Game.Close ();
	o_Montecarlo.Close ();
	sleep (1);

	Random::Destroy ();
	String::DestroyPorterStemmer ();
}
