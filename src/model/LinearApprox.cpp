#include "LinearApprox.h"
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_filesystem.h>
#include <nlp_distr.h>
#include <math.h>
#include <assert.h>


//														
LinearFunctionApprox::LinearFunctionApprox (void)
	: DatapointLog ()
{
	i_Features = 0;
	i_LanguageFeatures = 0;
	i_OperatorFeatures = 0;
}

LinearFunctionApprox::~LinearFunctionApprox (void)
{
	i_Features = 0;
	i_LanguageFeatures = 0;
	i_OperatorFeatures = 0;
}


//														
void LinearFunctionApprox::Init (String _rName)
{
	d_LearningRate = (config)(_rName + ":learning_rate");
	d_RegularizationFactor = (config)(_rName + ":regularization_factor");
	if (1 == (int)(config)(_rName + ":load_weights"))
		LoadWeights (_rName);
	else if (true == Path::Exists ((config)(_rName + ":weights_file")))
		Path::RemoveFile ((config)(_rName + ":weights_file"));

	f_RetainRatio = (config)(_rName + ":datapoint_sampling_ratio");
	i_UpdateIterations = (config)(_rName + ":update_iterations");
	b_BatchUpdate = (1 == (int)(config)(_rName + ":batch_update"));

	b_LogisticRegression = (1 == (int)(config)(_rName + ":use_logistic_regression"));
	double dLogisticMax = 1;
	double dLogisticMin = 0;
	if (true == b_LogisticRegression)
	{
		dLogisticMin = (config)(_rName + ":logistic_min");
		dLogisticMax = (config)(_rName + ":logistic_max");
	}
	d_LogisticScale = (dLogisticMax - dLogisticMin);
	d_LogisticOffset = dLogisticMin;

	cout << "Linear function approx (" << _rName << ')' << endl
		 << "   learning rate     : " << d_LearningRate << endl
		 << "   regularization    : " << d_RegularizationFactor << endl
		 << "   update iterations : " << i_UpdateIterations << endl
		 << "   batch update      : " << ((true == b_BatchUpdate)? "yes" : "no") << endl
		 << "   logistic          : " << ((true == b_LogisticRegression)? "yes" : "no") << endl;
	if  (true == b_LogisticRegression)
	{
		 cout << "   logistic min      : " << dLogisticMin << endl
			  << "   logistic max      : " << dLogisticMax << endl;
	}
}


//														
double LinearFunctionApprox::ComputeFunctionApprox (Features& _rFeatures,
													Features& _rLanguageFeatures,
													Features& _rOperatorFeatures)
{
	double dResult = 0;

	// normal features		
	{
		int iFeatureCount = _rFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			if (_rFeatures.Index (i) >= i_Features)
				continue;
			dResult += _rFeatures.Feature (i) * vec_Weights [_rFeatures.Index (i)];
		}
	}

	// language features	
	{
		int iFeatureCount = _rLanguageFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			if (_rLanguageFeatures.Index (i) >= i_LanguageFeatures)
				continue;
			dResult += _rLanguageFeatures.Feature (i) * vec_LanguageWeights [_rLanguageFeatures.Index (i)];
		}
	}
	
	// operator features	
	{
		int iFeatureCount = _rOperatorFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			if (_rOperatorFeatures.Index (i) >= i_OperatorFeatures)
				continue;
			dResult += _rOperatorFeatures.Feature (i) * vec_OperatorWeights [_rOperatorFeatures.Index (i)];
		}
	}
	

	if (true == b_LogisticRegression)
	{
		dResult = 1 / (1 + exp (- dResult));
		dResult = d_LogisticOffset + dResult * d_LogisticScale;
	}

	return dResult;
}


//														
void LinearFunctionApprox::AddDatapoint (Sample& _rSample,
										 Features& _rFeatures,
										 Features& _rLanguageFeatures,
										 Features& _rOperatorFeatures,
										 double _dObservedValue,
										 double _dPredictedValue,
										 String& _rState,
										 String& _rAgent)
{
	// Remember datapoint for subsequent batch updates	
	if (true == b_LogisticRegression)
	{
		double x = (_dObservedValue - d_LogisticOffset) / d_LogisticScale;
		_dObservedValue = log (x / (1 - x));

		if (_dObservedValue > 1)
		{
			_dObservedValue = 1;
			cout << "[ERROR] datapoint with logit value " << _dObservedValue
				 << " during logistic regression!" << endl;
		}
	}
	Datapoint* pDP = new Datapoint (_dObservedValue,
									_rFeatures,
									_rLanguageFeatures,
									_rOperatorFeatures);
	dq_Datapoints.push_back (pDP);
	dq_LongTermDatapoints.push_back (pDP);


	// the following RememberDatapoint() is only for 	
	// logging/debugging purposes.						
	RememberDatapoint (_rSample, _rState, _rAgent, _rFeatures, _dObservedValue, _dPredictedValue);

	// extend feature vector as necessary ...			
	{
		int iMaxIndex = 0;
		int iFeatureCount = _rFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			int iIndex = _rFeatures.Index (i);
			if (iMaxIndex < iIndex)
				iMaxIndex = iIndex;
		}
		if (i_Features < iMaxIndex + 1)
		{
			i_Features = iMaxIndex + 1;
			vec_Weights.Resize (i_Features, 0);
		};
	}

	// extend language feature vector as necessary ...	
	{
		int iMaxIndex = 0;
		int iFeatureCount = _rLanguageFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			int iIndex = _rLanguageFeatures.Index (i);
			if (iMaxIndex < iIndex)
				iMaxIndex = iIndex;
		}
		if (i_LanguageFeatures < iMaxIndex + 1)
		{
			i_LanguageFeatures = iMaxIndex + 1;
			vec_LanguageWeights.Resize (i_LanguageFeatures, 0);
		};
	}

	// extend operator feature vector as necessary ...	
	{
		int iMaxIndex = 0;
		int iFeatureCount = _rOperatorFeatures.Size ();
		for (int i = 0; i < iFeatureCount; ++ i)
		{
			int iIndex = _rOperatorFeatures.Index (i);
			if (iMaxIndex < iIndex)
				iMaxIndex = iIndex;
		}
		if (i_OperatorFeatures < iMaxIndex + 1)
		{
			i_OperatorFeatures = iMaxIndex + 1;
			vec_OperatorWeights.Resize (i_OperatorFeatures, 0);
		};
	}
}


//														
void LinearFunctionApprox::UpdateFunctionApprox (bool _bUseLongTermData)
{
	// reset weights - we're doing a batch update ...	
	if (true == b_BatchUpdate)
		ResetWeights ();

	// perform batch update ...							
	if (true == _bUseLongTermData)
	{
		for (int i = 0; i < i_UpdateIterations; ++ i)
		{
			ITERATE (Datapoint_dq_t, dq_LongTermDatapoints, ite)
			{
				Datapoint* pDatapoint = *ite;
				Features& rFeatures = pDatapoint->o_Features;
				Features& rLanguageFeatures = pDatapoint->o_LanguageFeatures;
				Features& rOperatorFeatures = pDatapoint->o_OperatorFeatures;
				double dNewValue = pDatapoint->d_Value;


				// compute old value ...		
				double dOldValue = 0;
				for (int i = 0; i < rFeatures.Size (); ++ i)
				{
					if (rFeatures.Index (i) >= i_Features)
						continue;
					dOldValue += rFeatures.Feature (i) * vec_Weights [rFeatures.Index (i)];
				}
				for (int i = 0; i < rLanguageFeatures.Size (); ++ i)
				{
					if (rLanguageFeatures.Index (i) >= i_LanguageFeatures)
						continue;
					dOldValue += rLanguageFeatures.Feature (i) * vec_LanguageWeights [rLanguageFeatures.Index (i)];
				}
				for (int i = 0; i < rOperatorFeatures.Size (); ++ i)
				{
					if (rOperatorFeatures.Index (i) >= i_OperatorFeatures)
						continue;
					dOldValue += rOperatorFeatures.Feature (i) * vec_OperatorWeights [rOperatorFeatures.Index (i)];
				}


				// update weights ...			
				for (int f = 0; f < rFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rFeatures.Feature (f);
					size_t iIndex = rFeatures.Index (f);
					vec_Weights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_Weights [iIndex]);
				}
				for (int f = 0; f < rLanguageFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rLanguageFeatures.Feature (f);
					size_t iIndex = rLanguageFeatures.Index (f);
					vec_LanguageWeights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_LanguageWeights [iIndex]);
				}
				for (int f = 0; f < rOperatorFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rOperatorFeatures.Feature (f);
					size_t iIndex = rOperatorFeatures.Index (f);
					vec_OperatorWeights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_OperatorWeights [iIndex]);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < i_UpdateIterations; ++ i)
		{
			ITERATE (Datapoint_dq_t, dq_Datapoints, ite)
			{
				Datapoint* pDatapoint = *ite;
				Features& rFeatures = pDatapoint->o_Features;
				Features& rLanguageFeatures = pDatapoint->o_LanguageFeatures;
				Features& rOperatorFeatures = pDatapoint->o_OperatorFeatures;
				double dNewValue = pDatapoint->d_Value;


				// compute old value ...		
				double dOldValue = 0;
				for (int i = 0; i < rFeatures.Size (); ++ i)
				{
					if (rFeatures.Index (i) >= i_Features)
						continue;
					dOldValue += rFeatures.Feature (i) * vec_Weights [rFeatures.Index (i)];
				}
				for (int i = 0; i < rLanguageFeatures.Size (); ++ i)
				{
					if (rLanguageFeatures.Index (i) >= i_LanguageFeatures)
						continue;
					dOldValue += rLanguageFeatures.Feature (i) * vec_LanguageWeights [rLanguageFeatures.Index (i)];
				}
				for (int i = 0; i < rOperatorFeatures.Size (); ++ i)
				{
					if (rOperatorFeatures.Index (i) >= i_OperatorFeatures)
						continue;
					dOldValue += rOperatorFeatures.Feature (i) * vec_OperatorWeights [rOperatorFeatures.Index (i)];
				}


				// update weights ...			
				for (int f = 0; f < rFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rFeatures.Feature (f);
					size_t iIndex = rFeatures.Index (f);
					vec_Weights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_Weights [iIndex]);
				}
				for (int f = 0; f < rLanguageFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rLanguageFeatures.Feature (f);
					size_t iIndex = rLanguageFeatures.Index (f);
					vec_LanguageWeights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_LanguageWeights [iIndex]);
				}
				for (int f = 0; f < rOperatorFeatures.Size (); ++ f)
				{
					double dUpdate = (dNewValue - dOldValue) * rOperatorFeatures.Feature (f);
					size_t iIndex = rOperatorFeatures.Index (f);
					vec_OperatorWeights [iIndex] += d_LearningRate *
							(dUpdate - d_RegularizationFactor * vec_OperatorWeights [iIndex]);
				}
			}
		}
	}
}


//														
void LinearFunctionApprox::CheckForNan (const char* _zId)
{
	bool bHasNan = false;
	for (int i = 0; i < i_Features; ++ i)
	{
		if ((true == isnan (vec_Weights [i])) || (true == isinf (vec_Weights [i])))
		{
			bHasNan = true;
			break;
		}
	}
	if (true == bHasNan)
	{
		cout << "[ERROR] {" << _zId << "} weigths has nan/inf:" << endl;
		for (int i = 0; i < i_Features; ++ i)
		{
			cout << vec_Weights [i] << ", ";
		}
		cout << endl << endl;
		abort ();
	}
}


//														
double LinearFunctionApprox::WeightVectorNorm (void)
{
	double dSum = 0;
	
	for (size_t i = 0; i < vec_Weights.Size (); ++ i)
		dSum += vec_Weights [i] * vec_Weights [i];
	for (size_t i = 0; i < vec_LanguageWeights.Size (); ++ i)
		dSum += vec_LanguageWeights [i] * vec_LanguageWeights [i];
	for (size_t i = 0; i < vec_OperatorWeights.Size (); ++ i)
		dSum += vec_OperatorWeights [i] * vec_OperatorWeights [i];

	return sqrt (dSum);
}



//														
void LinearFunctionApprox::SnapshotWeights (void)
{
	vec_WeightsSnapshot.CopyData (vec_Weights);
	vec_LanguageWeightsSnapshot.CopyData (vec_LanguageWeights);
	vec_OperatorWeightsSnapshot.CopyData (vec_OperatorWeights);
	i_FeaturesSnapshot = i_Features;
	i_LanguageFeaturesSnapshot = i_LanguageFeatures;
	i_OperatorFeaturesSnapshot = i_OperatorFeatures;
}


//														
void LinearFunctionApprox::RollbackWeights (void)
{
	vec_Weights.CopyData (vec_WeightsSnapshot);
	vec_LanguageWeights.CopyData (vec_LanguageWeightsSnapshot);
	vec_OperatorWeights.CopyData (vec_OperatorWeightsSnapshot);
	i_Features = i_FeaturesSnapshot;
	i_LanguageFeatures = i_LanguageFeaturesSnapshot;
	i_OperatorFeatures = i_OperatorFeaturesSnapshot;
}


//														
void LinearFunctionApprox::ResetWeights (void)
{
	vec_Weights.Memset (0);
	vec_LanguageWeights.Memset (0);
	vec_OperatorWeights.Memset (0);
}


//														
void LinearFunctionApprox::ClearLongTermDatapoints (void)
{
	ITERATE (Datapoint_dq_t, dq_LongTermDatapoints, ite)
		delete *ite;
	dq_LongTermDatapoints.clear ();
}


//														
void LinearFunctionApprox::ClearShortTermDatapoints (void)
{
	dq_Datapoints.clear ();
}


//														
bool LinearFunctionApprox::SaveWeights (const char* _zName)
{
	ios_base::openmode eMode = ios_base::out;
	{
		String sName;
		sName << _zName << ":retain_weight_history";
		if (1 == (int)(config)sName)
			eMode |= ios_base::app;
	}

	// State features		
	{
		String sName;
		sName << _zName << ":weights_file";

		CsvFile file;
		if (false == file.Open ((config)sName, eMode))
			return false;
		if (0 == vec_Weights.Size ())
			return true;

		file << vec_Weights [0];
		for (size_t i = 1; i < vec_Weights.Size (); ++ i)
			file << ',' << vec_Weights [i];
		file << endl;
		file.Close ();
	}
	// Language features	
	{
		String sName;
		sName << _zName << ":language_weights_file";

		CsvFile file;
		if (false == file.Open ((config)sName, eMode))
			return false;
		if (0 == vec_LanguageWeights.Size ())
			return true;

		file << vec_LanguageWeights [0];
		for (size_t i = 1; i < vec_LanguageWeights.Size (); ++ i)
			file << ',' << vec_LanguageWeights [i];
		file << endl;
		file.Close ();
	}
	// Operator features	
	{
		String sName;
		sName << _zName << ":operator_weights_file";

		CsvFile file;
		if (false == file.Open ((config)sName, eMode))
			return false;
		if (0 == vec_OperatorWeights.Size ())
			return true;

		file << vec_OperatorWeights [0];
		for (size_t i = 1; i < vec_OperatorWeights.Size (); ++ i)
			file << ',' << vec_OperatorWeights [i];
		file << endl;
		file.Close ();
	}

	return true;
}


//														
bool LinearFunctionApprox::LoadWeights (const char* _zName)
{
	cout << "Loading saved linear fn approx weights for "
		 << _zName << endl;

	// State features		
	{
		String sName;
		sName << _zName << ":weights_file";

		File file;
		if (false == file.Open ((config)sName))
			return false;
		String sLine;
		if (false == file.ReadLastLine (sLine))
			return false;

		String_dq_t dqValues;
		sLine.Split (dqValues, ',');
		
		i_Features = dqValues.size ();
		vec_Weights.Resize (i_Features, 0);
		for (int i = 0; i < i_Features; ++ i)
			vec_Weights [i] = dqValues [i];
	}
	// Language features	
	{
		String sName;
		sName << _zName << ":language_weights_file";

		File file;
		if (false == file.Open ((config)sName))
			return false;
		String sLine;
		if (false == file.ReadLastLine (sLine))
			return false;

		String_dq_t dqValues;
		sLine.Split (dqValues, ',');

		i_LanguageFeatures = dqValues.size ();
		vec_LanguageWeights.Resize (i_LanguageFeatures, 0);
		for (int i = 0; i < i_LanguageFeatures; ++ i)
			vec_LanguageWeights [i] = dqValues [i];
	}
	// Operator features	
	{
		String sName;
		sName << _zName << ":operator_weights_file";

		File file;
		if (false == file.Open ((config)sName))
			return false;
		String sLine;
		if (false == file.ReadLastLine (sLine))
			return false;

		String_dq_t dqValues;
		sLine.Split (dqValues, ',');

		i_OperatorFeatures = dqValues.size ();
		vec_OperatorWeights.Resize (i_OperatorFeatures, 0);
		for (int i = 0; i < i_OperatorFeatures; ++ i)
			vec_OperatorWeights [i] = dqValues [i];
	}

	cout << "   state/action weights : " << i_Features << endl;
	cout << "   language weights     : " << i_LanguageFeatures << endl;
	cout << "   operator weights     : " << i_OperatorFeatures << endl;
	cout << "   norm                 : " << WeightVectorNorm () << endl;
	return true;
}


//														
DatapointLog::DatapointLog (void)
{
	i_MaxDatapointsToRecord = 0;
	i_DatapointsSeen = 0;
	i_DatapointsSampled = 0;
	f_RetainRatio = 0;
	p_DPRecord = NULL;
}

DatapointLog::~DatapointLog (void)
{
	map_StateToIndex.clear ();
	map_AgentToIndex.clear ();
	if (NULL != p_DPRecord)
		delete[] p_DPRecord;
}

//														
void DatapointLog::StartDatapointLog (size_t _iMaxDatapoints)
{
	cout << "Starting datapoint log" << endl;
	i_MaxDatapointsToRecord = _iMaxDatapoints;
	i_DatapointsSeen = 0;
	i_DatapointsSampled = 0;

	b_WriteFeatures = (1 == (int)(config)"log_datapoint_features");

	if (NULL != p_DPRecord)
		delete[] p_DPRecord;
	if (true == b_WriteFeatures)
		p_DPRecord = new LogDPF [i_MaxDatapointsToRecord];
	else
		p_DPRecord = new LogDP [i_MaxDatapointsToRecord];
}


//														
void DatapointLog::StopDatapointLog (void)
{
	if (0 != i_MaxDatapointsToRecord)
		cout << "Stopping datapoint log" << endl;
	i_MaxDatapointsToRecord = 0;
}


//														
int	DatapointLog::GetStateIndex (String& _rState)
{
	StateToIndex_map_t::iterator	ite;
	ite = map_StateToIndex.find (_rState);
	if (map_StateToIndex.end () != ite)
		return ite->second;

	int iIndex = map_StateToIndex.size ();
	map_StateToIndex.insert (make_pair (_rState, iIndex));
	return iIndex;
}


//														
int DatapointLog::GetAgentIndex (String& _rAgent)
{
	AgentToIndex_map_t::iterator	ite;
	ite = map_AgentToIndex.find (_rAgent);
	if (map_AgentToIndex.end () != ite)
		return ite->second;

	int iIndex = map_AgentToIndex.size ();
	map_AgentToIndex.insert (make_pair (_rAgent, iIndex));
	return iIndex;
}


//														
void DatapointLog::RememberDatapoint (Sample& _rSample,
									  String& _rState,
									  String& _rAgent,
									  Features& _rFeatures,
									  float _fObservedValue,
									  float _fPredictedValue)
{
	if (0 == i_MaxDatapointsToRecord)
		return;
	++ i_DatapointsSeen;
	if (f_RetainRatio < _rSample.SampleUniform ())
		return;
	if (i_DatapointsSampled > i_MaxDatapointsToRecord)
		return;

	LogDP& rDP = p_DPRecord [i_DatapointsSampled];
	++ i_DatapointsSampled;

	if (true == b_WriteFeatures)
		((LogDPF&)rDP).p_Features = new Features (_rFeatures);

	rDP.f_ObservedValue = _fObservedValue;
	rDP.f_PredictedValue = _fPredictedValue;
	rDP.i_State = GetStateIndex (_rState);
	rDP.i_Agent = GetAgentIndex (_rAgent);
}


//														
bool DatapointLog::WriteDatapointLog (const char* _zName, bool _bOverwriteLog)
{
	cout << "datapoint log." << endl;
	cout << "   seen     : " << i_DatapointsSeen << endl;
	cout << "   sampled  : " << i_DatapointsSampled << endl;

	String sName;
	sName << _zName << ":datapoint_log_file";

	CsvFile file;
	if (true == _bOverwriteLog)
	{
		if (false == file.Open ((config)sName, ios_base::out))
		{
			cout << "[ERROR] Failed to open datapoint log file." << endl;
			return false;
		}
	}
	else
	{
		if (false == file.Open ((config)sName, ios_base::out|ios_base::app))
		{
			cout << "[ERROR] Failed to open datapoint log file." << endl;
			return false;
		}
	}

	size_t iMaxFeatures = LinearQFnFeatures::MaxIndex ();
	size_t iPoints = i_DatapointsSampled;
	for (size_t i = 0; i < iPoints; ++ i)
	{
		LogDP& rDP = p_DPRecord [i];
		file << rDP.i_State << ','
			 << rDP.i_Agent << ','
			 << rDP.f_ObservedValue << ','
			 << rDP.f_PredictedValue;

		if (true == b_WriteFeatures)
		{
			file << ',';
			Features* pFeatures = ((LogDPF&)rDP).p_Features;
			float_vec_t	vecFeatures;
			vecFeatures.resize (iMaxFeatures, 0);
			for (int f = 0; f < pFeatures->Size (); ++ f)
				vecFeatures [pFeatures->Index (f)] = pFeatures->Feature (f);
			
			if (0 == vecFeatures [0])
				file << '0';
			else
				file << vecFeatures [0];
			for (size_t f = 1; f < vecFeatures.size (); ++ f)
			{
				if (0 == vecFeatures [f])
					file << ",0";
				else
					file << ',' << vecFeatures [f];
			}
		}
		file << '\n';
	}
	file.Close ();

	ClearLog ();
	return true;
}


void DatapointLog::ClearLog (void)
{
	cout << "clearing datapoint log." << endl;
	i_DatapointsSampled = 0;
	i_DatapointsSeen = 0;
}






