#include "LogLinearModel.h"
#include "FeatureSpecialization.h"
#include <nlp_config.h>
#include <nlp_filesystem.h>



//											
LogLinearModel::LogLinearModel (void)
{
	i_Features = 0;
}


//											
LogLinearModel::~LogLinearModel (void)
{
	i_Features = 0;
}


//											
void LogLinearModel::Init (String _sName)
{
	d_LearningRate = (config)(_sName + ":learning_rate");
	d_RegularizationFactor = (config)(_sName + ":regularization_factor");
	if (1 == (int)(config)(_sName + ":load_weights"))
		LoadWeights (_sName);
	else if (true == Path::Exists ((config)(_sName + ":weights_file")))
		Path::RemoveFile ((config)(_sName + ":weights_file"));
}


//											
double LogLinearModel::ComputeLogProb (Features& _rFeatures)
{
	double dResult = 0;

	for (int i = 0; i < _rFeatures.Size (); ++ i)
	{
		int iIndex = _rFeatures.Index (i);
		if (iIndex >= i_Features)
			continue;
		dResult += _rFeatures.Feature (i) * vec_Weights [iIndex];
	}
	
	return dResult;
}


//											
void LogLinearModel::InitializeFeatureExpectationVector (double_Vec_t& _rvecExpectedFeatures,
														 int _iFeatureCount)
{
	// int iFeatures = 1 + LogLinearModelFeatures::MaxIndex ();
	_rvecExpectedFeatures.Create (_iFeatureCount);
	_rvecExpectedFeatures.Memset (0);
}


//											
void LogLinearModel::ComputeNegativeFeatureExpectation (LogProbability& _rLogProb,
														Features_vec_t& _rvecFeatures, 
														double_Vec_t& _rvecExpectedFeatures)
{
	size_t iProbs = _rLogProb.Size ();
	if (0 == iProbs)
		return;

	Probability oProb (_rLogProb);
	for (size_t i = 0; i < iProbs; ++ i)
	{
		Features* pFeatures = _rvecFeatures [i];
		double dProb = oProb [i];

		for (int f = 0; f < pFeatures->Size (); ++ f)
			_rvecExpectedFeatures [pFeatures->Index (f)] -= dProb * pFeatures->Feature (f);
	}
}


//											
void LogLinearModel::UpdateWeights (double _dReward, 
									double_Vec_t& _rvecTrace)
{
	int iSize = _rvecTrace.Size ();
	if ((int)vec_Weights.Size () < iSize)
		vec_Weights.Resize (iSize, 0);
	i_Features = iSize;

	for (int i = 0; i < iSize; ++ i)
	{
		vec_Weights [i] += d_LearningRate * 
							(_dReward * _rvecTrace [i]
							- d_RegularizationFactor * vec_Weights [i]);
	}
}


//											
void LogLinearModel::ResetWeights (void)
{
	if (i_Features > 0)
		vec_Weights.Memset (0);
}


//											
bool LogLinearModel::SaveWeights (const char* _zName)
{
	// LogLinearModelFeatures::SaveIndices (_zName);
	ios_base::openmode eMode = ios_base::out;
	{
		String sName;
		sName << _zName << ":retain_weight_history";
		if (1 == (int)(config)sName)
			eMode |= ios_base::app;
	}

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

	return true;
}


//											
bool LogLinearModel::LoadWeights (const char* _zName)
{
	cout << "Loading weights for log linear model : "
		 << _zName << endl;
	// LogLinearModelFeatures::LoadIndices (_zName);

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

	return true;
}


//														
void LogLinearModel::CheckForNan (const char* _zId)
{
	bool bHasNan = false;
	for (int i = 0; i < i_Features; ++ i)
	{
		if (true == isnan (vec_Weights [i]))
		{
			bHasNan = true;
			break;
		}
	}
	if (true == bHasNan)
	{
		cout << "[ERROR] {" << _zId << "} weigths has nan:" << endl;
		for (int i = 0; i < i_Features; ++ i)
		{
			cout << vec_Weights [i] << ", ";
		}
		cout << endl << endl;
		abort ();
	}
}



