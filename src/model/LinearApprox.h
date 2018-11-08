#ifndef __LINEAR_FUNCTION_APPROX__
#define __LINEAR_FUNCTION_APPROX__

#include <vector>
#include <deque>
#include <pthread.h>
#include <nlp_vector.h>
#include <nlp_distr.h>
#include "FeatureSpecialization.h"
using namespace std;

class Datapoint;
class LogDP;
typedef deque <int>			int_dq_t;
typedef deque <float>		float_dq_t;
typedef deque <LogDP*>		dp_dq_t;
typedef map <String, int>	StateToIndex_map_t;
typedef map <String, int>	AgentToIndex_map_t;
typedef deque <Datapoint*>	Datapoint_dq_t;
typedef Vector <double>		double_Vec_t;


//												
class LogDP
{
	public:
		float		f_PredictedValue;
		float		f_ObservedValue;
		int			i_State;
		int			i_Agent;
};

class LogDPF : public LogDP
{
	public:
		Features*	p_Features;
};


//												
class DatapointLog
{
	protected:
		StateToIndex_map_t	map_StateToIndex;
		AgentToIndex_map_t	map_AgentToIndex;

		LogDP*			p_DPRecord;
		size_t			i_MaxDatapointsToRecord;
		float			f_RetainRatio;
		size_t			i_DatapointsSeen;
		size_t			i_DatapointsSampled;
		bool			b_WriteFeatures;

		int	GetStateIndex (String& _rState);
		int GetAgentIndex (String& _rAgent);
		void ClearLog (void);

	public:
		DatapointLog (void);
		~DatapointLog (void);

		void StartDatapointLog (size_t _iMaxDatapoints);
		void StopDatapointLog (void);
		bool WriteDatapointLog (const char* _zName, bool _bOverwriteLog);
		bool LoggingDatapoints (void)
		{ return (0 != i_MaxDatapointsToRecord); };

		void RememberDatapoint (Sample& _rSample,
								String& _rState,
								String& _rAgent,
								Features& _rFeatures,
								float _fObservedValue,
								float _fPredictedValue);
};


//												
class Datapoint
{
	public:
		double 		d_Value;
		Features	o_Features;
		Features	o_LanguageFeatures;
		Features	o_OperatorFeatures;

		Datapoint (double _dValue, 
				   const Features& _rFeatures,
				   const Features& _rLanguageFeatures,
				   const Features& _rOperatorFeatures)
			: o_Features (_rFeatures),
			  o_LanguageFeatures (_rLanguageFeatures),
			  o_OperatorFeatures (_rOperatorFeatures)
		{ d_Value = _dValue; };
};


//												
class LinearFunctionApprox : public DatapointLog
{
	private:
		double_Vec_t		vec_Weights;
		double_Vec_t		vec_LanguageWeights;
		double_Vec_t		vec_OperatorWeights;
		int					i_Features;
		int					i_LanguageFeatures;
		int					i_OperatorFeatures;

		double_Vec_t		vec_WeightsSnapshot;
		double_Vec_t		vec_LanguageWeightsSnapshot;
		double_Vec_t		vec_OperatorWeightsSnapshot;
		int					i_FeaturesSnapshot;
		int					i_LanguageFeaturesSnapshot;
		int					i_OperatorFeaturesSnapshot;


		double				d_LearningRate;
		double				d_RegularizationFactor;
		int					i_UpdateIterations;
		bool				b_BatchUpdate;

		bool				b_LogisticRegression;
		double				d_LogisticOffset;
		double				d_LogisticScale;

		Datapoint_dq_t		dq_Datapoints;
		Datapoint_dq_t		dq_LongTermDatapoints;

		void CheckForNan (const char* _zId);

	public:
		LinearFunctionApprox (void);
		~LinearFunctionApprox (void);

		void Init (String _sName);

		double ComputeFunctionApprox (Features& _rFeatures,
									  Features& _rLanguageFeatures,
									  Features& _rOperatorFeatures);

		void AddDatapoint (Sample& _rSample,
						   Features& _rFeatures,
						   Features& _rLanguageFeatures,
						   Features& _rOperatorFeatures,
						   double _dObservedValue,
						   double _dPredictedValue,
						   String& _rState,
						   String& _rAgent);

		void UpdateFunctionApprox (bool _bUseLongTermData);
		double WeightVectorNorm (void);

		void ClearLongTermDatapoints (void);
		void ClearShortTermDatapoints (void);
		void SnapshotWeights (void);
		void RollbackWeights (void);
		void ResetWeights (void);
		bool SaveWeights (const char* _zName);
		bool LoadWeights (const char* _zName);
};



#endif
