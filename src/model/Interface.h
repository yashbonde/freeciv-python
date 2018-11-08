#ifndef __GAME_INTERFACE__
#define __GAME_INTERFACE__

#include <deque>
#include <nlp_socket.h>
#include "FeatureSpecialization.h"
#include "Text.h"
#include "HiddenState.h"
#include "OperatorWords.h"
using namespace std;

class Action;
class Agent;
class State;
typedef deque<Action*>		Action_dq_t;
typedef deque<State*>		State_dq_t;
typedef vector<Action>		Action_vec_t;
typedef vector<Action*>		ActionPointer_vec_t;
typedef vector<Agent*>		Agent_vec_t;
typedef deque<String_dq_t>	GameScores_dq_t;


//														
class Action
{
	public:
		Agent*				p_Agent;
		LinearQFnFeatures	o_Features;
		int_dq_t			dq_FeatureLabelIndices;
		int_dq_t			dq_FeatureLabels;
		String				s_Command;
		String				s_GenericCommand;
		String				s_CommandType;
		String				s_ActionName;
		int					i_CommandType;
		int					i_UnitCommand;

		Features					o_AugmentedFeatures;
		RelevanceModelFeatures		o_LanguageFeatures;
		RelevanceModelFeatures		o_OperatorFeatures;
		Sentence*					p_SelectedSentence;
		HiddenState*				p_SelectedHiddenState;
		bool						b_IsArgmaxSentence;


		Action (void);
		Action (const Action& _rAction);
		~Action (void);
		bool SetAction (char* _zAction, String_dq_t& _rdqLabels);
		void SetBowFeatures (const char* _zPrefix, char* _zFeatures, String_dq_t& _rdqLabels);
};


//														
class Agent
{
	private:
		static String_set_t	set_AttemptedAction;
		float Truncate (float _fValue, float _fScale);

	public:
		static TextInterface*	p_TextInterface;

		Action_vec_t		vec_Actions;
		int_dq_t			dq_NewActions;
		LinearQFnFeatures	o_Features;
		int_dq_t			dq_FeatureLabelIndices;
		int_dq_t			dq_FeatureLabels;
		int					i_ContinueActionIndex;
		char				c_MetaType;
		String				s_UnitType;
		int					i_UnitType;
		FilteredSentences*	p_FilteredSentences;

		Agent (char _cType);
		Agent (const Agent& _rAgent);
		~Agent (void);

		void SetActions (char* _zActions, String_dq_t& _rdqLabels);
		bool SetFeatures (char* _zFeatures, Agent* _pNation, String_dq_t& _rdqLabels);
		void SetBowFeatures (const char* _zPrefix, char* _zFeatures, String_dq_t& _rdqLabels);

		static void AddAttemptedAction (const char* _zGenericAction)
		{ set_AttemptedAction.insert (_zGenericAction); };
};


//														
class State
{
	public:
		Agent_vec_t		vec_Agents;
		String			s_State;

		~State (void);
		void SetState (String& _rState, OperatorWords* _pOperatorModel = NULL);
};



//														
class GameInterface : public ClientSocket
{
	private:
		Buffer	o_Data;
		String	s_SendBuffer;
		bool	b_AlwaysWaitForResponse;
		double	d_MaxCompoundScore;
		double	d_GameInternalScoreWeight;
		bool	b_RatioOfSums;
		bool	b_DiffScore;
		String_dq_t	dq_ReceivedMsgs;

		void Send (String _sMessage);
		String GetResponse (void);
		double GetScoreRatio (GameScores_dq_t& dqPlayers, int _iIndex);
		double GetOpponentScore (GameScores_dq_t& _dqPlayers, int _iIndex);
		pair <double,double> GetScore_SumOfRatios (void);
		pair <double,double> GetScore_RatioOfSums (void);
		pair <double,double> GetScore_Diff (void);

	public:
		GameInterface (void);

		bool Init (String _sServer); /* initialise */
		bool Connect (String _sServer);
		void Disconnect (void);
		void EnableEarlyGameEndDetection (void);

		bool MeasureState (String& _rState);
		bool SendCommand (String& _rCommand);
		bool EndTurn (void);
		pair <double,double> GetScore (void);

		bool SaveGame (String& _rSaveName);
		bool LoadGame (String& _rSaveName);

		void ClearConnection (bool _bWarn);

		//											
		void OnReceive (const void* _zData, long _lBytes);
		void OnDisconnect (void);
};


#endif
