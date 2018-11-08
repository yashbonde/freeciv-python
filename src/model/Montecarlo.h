#ifndef __GAME_MONTECARLO_SEARCH__
#define __GAME_MONTECARLO_SEARCH__

#include <nlp_string.h>
#include <nlp_time.h>
#include <nlp_distr.h>
#include "Interface.h"
#include "Play.h"
#include <map>
using namespace std;

enum CommandSelectionMethon_e
{
	csm_trivial,
	csm_naive_bayes,
	csm_q_fn
};


class MonteCarlo : public Play
{
	private:
		pthread_rwlock_t			rwl_Model;
		GameInterface*				p_SimulationInterfaces;
		Sample*						p_Sample;
		int							i_SimulationInterfaces;

		int							i_LanguageLearningRollouts;
		int							i_MaxRollouts;
		int							i_MaxRolloutDepth;
		int							i_FinalRolloutDepth;

		CommandSelectionMethon_e	e_CommandSelectionMethod;
		bool						b_QfnUpdateBeforeCommandSelection;
		int							i_QfnSelectionStart;
		bool						b_SelectActionBasedOnAverageQValue;
		double						d_MinExplorationProbability;
		double						d_ExplorationProbabilityRange;

		int							i_LogDatapointsFrom;
		int							i_LogDatapointsUntil;
		int							i_DatapointLogLength;
		bool						b_LogAllParamUpdates;

		bool						b_UpdateInTestMode;

		void RolloutTrace (Sample& _rSample,
						   GameInterface& _rGameInterface, 
						   String* _pCommand,
						   double* _pReward,
						   double* _pScore,
						   int _iIteration,
						   bool _bGreedyRollout,
						   bool _bTestMode,
						   bool _bFinalStep);
		bool SimulateStep (Sample& _rSample,
							GameInterface& _rGameInterface,
							int _iStep,
						    double* _pReward,
							double* _pScore,
							State* _pState,
							Action_dq_t* _pdqActions,
							float _fExplorationProbability);

		String SelectBestCommand (Sample& _rSample,
								  CommandToScore_map_t& _rmapCommandToScore,
								  GameInterface& _rActualGame,
								  int _iStep);
		String SelectBestCommand_Qfn (Sample& _rSample,
									  GameInterface& _rActualGame);

	public:
		int							i_FinalRollouts;
		Time			o_Timer1, o_Timer2;

		MonteCarlo (void);
		~MonteCarlo (void);

		bool Init (void);
		void Close (void);
		String SelectCommand (GameInterface& _rActualGame,
							  String& _rStateName,
							  int _iStep,
							  bool _bTestMode,
							  int _iStepsToFinalRollout);
		void PrintStats (void);
};


#endif
