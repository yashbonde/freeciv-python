#include <assert.h>
#include <math.h>
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_filesystem.h>
#include "Feature.h"
#include "learner_comms.h"



//											
FeatureSpace::FeatureSpace (void)
{
	i_BagOfWordsOffset = 0;
	i_MaxIndex = 0;
	pthread_rwlock_init (&rwl_IndexMap, NULL);
}

FeatureSpace::~FeatureSpace (void)
{
	map_FeatureValueToIndex.clear ();
	pthread_rwlock_destroy (&rwl_IndexMap);
}


//											
int FeatureSpace::GetFeatureIndex (String& _rName)
{
	pthread_rwlock_rdlock (&rwl_IndexMap);

	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_FeatureValueToIndex.find (_rName);
	if (map_FeatureValueToIndex.end () != ite)
	{
		pthread_rwlock_unlock (&rwl_IndexMap);
		return ite->second;
	}
	pthread_rwlock_unlock (&rwl_IndexMap);


	pthread_rwlock_wrlock (&rwl_IndexMap);
	int iIndex = map_FeatureValueToIndex.size ()
					+ i_BagOfWordsOffset;
	if (i_MaxIndex < iIndex)
		i_MaxIndex = iIndex;

	map_FeatureValueToIndex.insert (make_pair (_rName, iIndex));
	pthread_rwlock_unlock (&rwl_IndexMap);
	return iIndex;
}


//											
void FeatureSpace::SetBagOfWordsOffset (int _iOffset)
{
	assert ((0 == i_BagOfWordsOffset) || (i_BagOfWordsOffset == _iOffset));
	i_BagOfWordsOffset = _iOffset;
}


//											
bool FeatureSpace::SaveFeatureMapping (String _sName)
{
	File file;
	if (false == file.Open((config)(_sName + ":feature_mapping_file"), ios_base::out))
		return false;

	ITERATE (FeatureValueToIndex_map_t, map_FeatureValueToIndex, ite)
		file << ite->second << '\x01' << ite->first << endl;
	
	file.Close ();
	return true;
}


//											
bool FeatureSpace::LoadFeatureMapping (String _sName)
{
	String_dq_t dqLines;
	if (false == File::ReadLines ((config)(_sName + ":feature_mapping_file"), dqLines))
		return false;

	map_FeatureValueToIndex.clear ();
	ITERATE (String_dq_t, dqLines, ite)
	{
		String_dq_t dqValues;
		ite->Split (dqValues, '\x01');
		map_FeatureValueToIndex.insert (make_pair (dqValues [1], (int)dqValues [0]));
	}
	return true;
}




//											
Features::Features (void)
{
	i_Current = 0;
}

Features::Features (const Features& _rFeatures)
{
	vec_Indices.Copy (_rFeatures.vec_Indices);
	vec_Features.Copy (_rFeatures.vec_Features);
	i_Current = _rFeatures.i_Current;
}


Features::~Features (void)
{
	i_Current = 0xDEADBEEF + 1;
	i_MaxSize = 0xDEADBEEF;
}


//											
void Features::SetSize (int _iSize)
{
	assert (0 == i_Current);
	
	vec_Indices.Reserve (_iSize);
	vec_Features.Reserve (_iSize);
	i_MaxSize = _iSize;
}


//											
Features& Features::operator= (const Features& _rFeatures)
{
	vec_Indices.Copy (_rFeatures.vec_Indices);
	vec_Features.Copy (_rFeatures.vec_Features);
	i_Current = _rFeatures.i_Current;
	assert (i_Current < i_MaxSize);

	return *this;
}


//											
void Features::Set (FeatureSpace& _rSpace, int _iIndex, float _fValue)
{
	assert (false == isnan (_fValue));
	assert (_iIndex < _rSpace.BagOfWordOffset ());
	assert (_iIndex >= 0);
	if (0 == _fValue)
		return;

	assert (i_Current < i_MaxSize);
	vec_Indices [i_Current] = _iIndex;
	vec_Features [i_Current] = _fValue;
	++ i_Current;
}


//											
void Features::Set (int _iIndex, float _fValue)
{
	assert (false == isnan (_fValue));
	assert (_iIndex >= 0);
	if (0 == _fValue)
		return;

	assert (i_Current < i_MaxSize);
	vec_Indices [i_Current] = _iIndex;
	vec_Features [i_Current] = _fValue;
	++ i_Current;
}


//											
void Features::Set (const Features& _rFeatures)
{
	size_t iNewFeatures = _rFeatures.Size ();
	assert (i_Current + iNewFeatures <= i_MaxSize);
	if (i_Current + iNewFeatures > i_MaxSize)
	{
		cout << i_Current << " + " << iNewFeatures << " > " << i_MaxSize << endl;
		abort ();
	}
	vec_Indices.InsertDataBlock (i_Current, (int*)((Features&)_rFeatures).vec_Indices, iNewFeatures);
	vec_Features.InsertDataBlock (i_Current, (float*)((Features&)_rFeatures).vec_Features, iNewFeatures);
	i_Current += iNewFeatures;
}


//											
void Features::SetBagOfWords (FeatureSpace& _rSpace, const char* _zFeature, float _fValue)
{
	String sFeature (_zFeature);
	int iIndex = _rSpace.GetFeatureIndex (sFeature);
	assert (iIndex >= 0);

	assert (i_Current < i_MaxSize);
	vec_Indices [i_Current] = iIndex;
	vec_Features [i_Current] = _fValue;
	++ i_Current;
}


//											
void Features::SetBagOfWords (FeatureSpace& _rSpace, String& _rFeature, float _fValue)
{
	int iIndex = _rSpace.GetFeatureIndex (_rFeature);
	assert (iIndex >= 0);

	assert (i_Current < i_MaxSize);
	vec_Indices [i_Current] = iIndex;
	vec_Features [i_Current] = _fValue;
	++ i_Current;
}


//											
void Features::SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, char* _zFeatures, float _fValue)
{
	zchar_dq_t dqValues = String::DestructiveSplit (_zFeatures, LCP_BOW_SEPARATOR);
	SetBagOfWords (_rSpace, _zPrefix, dqValues, _fValue);
}


//											
void Features::SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, zchar_dq_t& _rdqFeatures, float _fValue)
{
	ITERATE (zchar_dq_t, _rdqFeatures, iteValue)
	{
		String sName;
		sName << _zPrefix << '\x01' << *iteValue;

		int iIndex = _rSpace.GetFeatureIndex (sName);
		assert (iIndex >= 0);

		if (i_Current >= i_MaxSize)
		{
			cout << "array bounds : " << i_Current << " >= " << i_MaxSize << endl;
			ITERATE (zchar_dq_t, _rdqFeatures, ite)
				cout << '[' << *ite << "], ";
			cout << endl;
			abort ();
		}

		vec_Indices [i_Current] = iIndex;
		vec_Features [i_Current] = _fValue;
		++ i_Current;
	}
}


//											
void Features::SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, String_dq_t& _rdqFeatures, float _fValue)
{
	ITERATE (String_dq_t, _rdqFeatures, iteValue)
	{
		String sName;
		sName << _zPrefix << '\x01' << *iteValue;

		int iIndex = _rSpace.GetFeatureIndex (sName);
		assert (iIndex >= 0);
		assert (i_Current < i_MaxSize);

		vec_Indices [i_Current] = iIndex;
		vec_Features [i_Current] = _fValue;
		++ i_Current;
	}
}


//											
double Features::DotProduct (double_vec_t& _rvecWeights)
{
	double dResult = 0;
	for (int i = 0; i < Size (); ++ i)
		dResult += vec_Features [i] * _rvecWeights [vec_Indices [i]];
	
	return dResult;
}


//											
ostream& operator<< (ostream& _rStream, const Features& _rFeatures)
{
	if (0 == _rFeatures.Size ())
		return _rStream;

	_rStream << _rFeatures.Index (0) << ':' << _rFeatures.Feature (0);
	for (int i = 1; i < _rFeatures.Size (); ++ i)
		_rStream << ", " << _rFeatures.Index (i) << ':' << _rFeatures.Feature (i);
	return _rStream;
}


//											
ostream& operator<< (ostream& _rStream, const double_vec_t& _rvecValues)
{
	if (true == _rvecValues.empty ())
		return _rStream;

	_rStream << _rvecValues [0];
	for (size_t i = 1; i < _rvecValues.size (); ++ i)
		_rStream << ", " << _rvecValues [i];
	return _rStream;
}


bool Features::Check (void) const
{
	for (size_t i = 0; i < i_Current; ++ i)
	{
		if (vec_Indices [i] < 0)
			return false;
	}
	return true;
}

