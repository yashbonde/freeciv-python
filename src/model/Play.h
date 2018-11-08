#ifndef __GAME_PLAY__
#define __GAME_PLAY__

#include <nlp_string.h>
#include <nlp_time.h>
#include <nlp_distr.h>
#include "Interface.h"
#include "Text.h"
#include "SearchTree.h"
#include "LinearApprox.h"
#include "HiddenStateVar.h"
#include "SentenceRelevance.h"
#include "OperatorWords.h"
#include <map>
using namespace std;

class Play;
typedef map <String, size_t>						ActionToIndex_map_t;
typedef map <String, double>						CommandToScore_map_t;
typedef multimap <String, double>					CommandToScore_mmp_t;
typedef map <String, pair <double,int> >			CommandToScoreTotal_map_t;
typedef map <String, CommandToScoreTotal_map_t>		UnitToCommandScores_map_t;
typedef map <String, pair<double,double> >			ActionToAverage_map_t;
typedef map <long, unsigned long>					SentenceToCount_map_t;

typedef String (Play::*SelectBestCommand_pfn)  (CommandToScore_map_t& _rmapCommandToScore);
typedef size_t (Play::*SampleAction_pfn)  (Sample& _rSample,
											String& _rState,
											Agent& _rAgent,
											float _fExplorationProbability);
typedef void (Play::*ComputeHeuristics_pfn) (Agent& _rAgent, double* _dScore);


class Play
{
	protected:
		pthread_mutex_t			mtx_SentenceHit;

		TextInterface			o_TextInterface;
		SearchTree				o_Tree;
		LinearFunctionApprox	o_LinearFunctionApprox;
		HiddenStateVariable		o_HiddenStateModel;
		SentenceRelevance		o_RelevanceModel;
		OperatorWords			o_OperatorWordsModel;

		SampleAction_pfn		pfn_SampleAction;
		ComputeHeuristics_pfn	pfn_ComputeHeuristicScores;

		int						i_BiasContinue;
		double					d_ExplorationBonusRatio;
		float					f_HeuristicBiasProbability;
		float					f_HeuristicBiasProbabilityStep;
		float					f_HeuristicBiasProbabilityMin;
		bool					b_UsingSentenceRelevanceModel;
		bool					b_UseLabelOverlapSentenceSelection;
		bool					b_UseOperatorFeatures;
		bool					b_UseOperatorFeaturesOnly;
		bool					b_UseOperatorWordModel;
		bool					b_UseHiddenStateModel;
		int						i_QfnUpdateLocalizationDepth;
		bool					b_PerPlyApproximation;
		bool					b_ResetAllModelsEndOfPly;
		bool					b_OnlineUpdate;
		bool					b_UseBackpropUpdate;
		double					d_NewActionBias;

		ActionToAverage_map_t	map_ActionToAverage;
		SentenceToCount_map_t	map_SentenceToHitCount;


		void CombineAgentAndActionFeatures (Action& _rAction);
		void CombineHiddenStateAndStateFeatures (Action& _rAction);
		void CombineTextAndStateFeatures (Sentence& _rSentence,
										  Action& _rAction);
		void CombineTextAndStateOperatorFeatures (Sentence& _rSentence,
												  Action& _rAction);
		void CombineTextAndStateOperatorFeaturesOnly (Sentence& _rSentence,
													  Action& _rAction);

		void NormalizeScores (double* _dScores, size_t _iCount);
		void ComputeScore_Uniform (Agent& _rAgent, double* _dScore);
		void ComputeScore_EstimatedPrior (Agent& _rAgent, double* _dScore);
		void ComputeScore_TextInfo (Agent& _rAgent, double* _dScore);
		void ComputeScore_TextAndEstimatedPrior (Agent& _rAgent, double* _dScore);
		void ComputeScore_QFnApprox (Sample& _rSample, Agent& _rAgent, double* _dScore);
		void ComputeScore_QFnApproxWithText (Sample& _rSample, Agent& _rAgent, double* _dScore);

		void RememberSentenceHit (Sentence* _pSentence);

		size_t SampleAction_Uniform (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_EstimatedPrior (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_TextInfo (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_TextAndEstimatedPrior (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_MCTS_UCT (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_QFnApprox (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);
		size_t SampleAction_QFnApproxWithText (Sample& _rSample,
									  String& _rState,
									  Agent& _rAgent,
									  float _fExplorationProbability);

		void UpdateParameters (Sample& _rSample,
							   State_dq_t& _rdqState,
							   Action_dq_t& _rdqAction,
							   double _dScore);
		void EstimateActionPrior (Action_dq_t& _rdqActions, double _dScore);
		void UpdateMCT (State& _rState, Action_dq_t& _rdqAction, double _dScore);
		void UpdateQFnApprox (Sample& _rSample,
							  State_dq_t& _rdqState,
							  Action_dq_t& _rdqAction,
							  double _dScore);

		String GetActionTypeAndUnitId (String _sCommand);
		String SelectBestCommand_Trivial (Sample& _rSample,
										  CommandToScore_map_t& _rmapCommandToScore);
		String SelectBestCommand_NaiveBayes (Sample& _rSample,
											 CommandToScore_map_t& _rmapCommandToScore);

		bool UsingEstimatedPrior (void)
		{ return (&Play::SampleAction_EstimatedPrior == pfn_SampleAction); };
		bool UsingQValueApprox (void)
		{ return (&Play::SampleAction_QFnApprox == pfn_SampleAction); };
		bool UsingQValueApproxWithText (void)
		{ return (&Play::SampleAction_QFnApproxWithText == pfn_SampleAction); };
		bool UsingQValueApproxSampling (void)
		{ return (&Play::SampleAction_QFnApprox == pfn_SampleAction) ||
				 (&Play::SampleAction_QFnApproxWithText == pfn_SampleAction); };
		bool UsingMCTS_UCT (void)
		{ return (&Play::SampleAction_MCTS_UCT == pfn_SampleAction); };

		String GetCompoundCommand (Action_dq_t& _rdqActions);

		void SaveParams (void);
		void WriteIterationMarker (int _iIteration);

		void PrintEstimatedPrior (Sample& _rSample);
		void PrintSentenceHitStats (void);
		void PrintPredicatePredictions (Sample& _rSample);

	public:
		Play (void);
		~Play (void);

		bool Init (void);
		void EndPly (int _iStep, bool _bDontResetWeights, bool _bTestMode);
		void Destroy (void);

		void PrintStats (Sample& _rSample);
};


#endif
