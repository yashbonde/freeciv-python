#include "FeatureSpecialization.h"
#include <nlp_macros.h>
#include <nlp_config.h>
#include <nlp_filesystem.h>
#include <math.h>
#include <assert.h>

FeatureSpace				LinearQFnFeatures::o_FeatureSpace;
FeatureValueToIndex_map_t	RelevanceModelFeatures::map_WordToIndex;
FeatureValueToIndex_map_t	RelevanceModelFeatures::map_OperatorToIndex;
FeatureValueToIndex_map_t	RelevanceModelFeatures::map_PrefixToIndex;
int							RelevanceModelFeatures::i_WordCount = 0;
int							RelevanceModelFeatures::i_OperatorCount = 0;
int							RelevanceModelFeatures::i_PrefixCount = 0;
pthread_rwlock_t			RelevanceModelFeatures::rwl_IndexMap;

FeatureValueToIndex_map_t	OperatorModelFeatures::map_TokenToIndex;
int							OperatorModelFeatures::i_TokenCount = 0;
pthread_rwlock_t			OperatorModelFeatures::rwl_IndexMap;

//														
LinearQFnFeatures::LinearQFnFeatures (void)
	: Features ()
{
}


//														
int RelevanceModelFeatures::FindWordIndex (const String& _rWord)
{
	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_WordToIndex.find (_rWord);
	if (map_WordToIndex.end () != ite)
		return ite->second;

	return -1;
}


//														
int RelevanceModelFeatures::GetWordIndex (const String& _rWord)
{
	pthread_rwlock_rdlock (&rwl_IndexMap);
	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_WordToIndex.find (_rWord);
	if (map_WordToIndex.end () != ite)
	{
		pthread_rwlock_unlock (&rwl_IndexMap);
		return ite->second;
	}
	pthread_rwlock_unlock (&rwl_IndexMap);

	pthread_rwlock_wrlock (&rwl_IndexMap);
	int iIndex = map_WordToIndex.size ();
	map_WordToIndex.insert (make_pair (_rWord, iIndex));
	i_WordCount = map_WordToIndex.size ();
	pthread_rwlock_unlock (&rwl_IndexMap);
	return iIndex;
}


//														
int RelevanceModelFeatures::GetOperatorIndex (const String& _rWord)
{
	pthread_rwlock_rdlock (&rwl_IndexMap);
	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_OperatorToIndex.find (_rWord);
	if (map_OperatorToIndex.end () != ite)
	{
		pthread_rwlock_unlock (&rwl_IndexMap);
		return ite->second;
	}
	pthread_rwlock_unlock (&rwl_IndexMap);

	pthread_rwlock_wrlock (&rwl_IndexMap);
	int iIndex = map_OperatorToIndex.size ();
	map_OperatorToIndex.insert (make_pair (_rWord, iIndex));
	i_OperatorCount = map_OperatorToIndex.size ();
	pthread_rwlock_unlock (&rwl_IndexMap);
	return iIndex;
}


//														
int RelevanceModelFeatures::GetPrefixIndex (const String& _rPrefix)
{
	pthread_rwlock_rdlock (&rwl_IndexMap);
	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_PrefixToIndex.find (_rPrefix);
	if (map_PrefixToIndex.end () != ite)
	{
		pthread_rwlock_unlock (&rwl_IndexMap);
		return ite->second;
	}
	pthread_rwlock_unlock (&rwl_IndexMap);

	pthread_rwlock_wrlock (&rwl_IndexMap);
	int iIndex = map_PrefixToIndex.size ();
	map_PrefixToIndex.insert (make_pair (_rPrefix, iIndex));
	i_PrefixCount = map_PrefixToIndex.size ();
	pthread_rwlock_unlock (&rwl_IndexMap);

	return iIndex;
}


//														
void RelevanceModelFeatures::SetPrefixFeature (int _iPrefix, int _iFeatureId)
{
	int iIndex = _iPrefix * i_WordCount + _iFeatureId;
	vec_Indices [i_Current] = iIndex;
	vec_Features [i_Current] = 1;
	++ i_Current;
}


//														
void RelevanceModelFeatures::SetBagOfWords (int _iPrefix, int* _pFeatureIds, int _iFeatures, float _fValue)
{
	assert (false == isnan (_fValue));
	if (0 == _fValue)
		return;

	for (int i = 0; i < _iFeatures; ++ i)
	{
		int iFeatureId = _pFeatureIds [i];
		// The +1 below is to account for the insertion of the	
		// heuristic sentence score as a feature.				
		int iIndex = _iPrefix * i_WordCount + iFeatureId + 1;
		assert ((size_t)iIndex == _iPrefix * i_WordCount + iFeatureId + 1);

		vec_Indices [i_Current] = iIndex;
		vec_Features [i_Current] = _fValue;
		++ i_Current;
	}
}


//														
bool RelevanceModelFeatures::SaveIndices (const char* _zName)
{
	String sName;
	sName << _zName << ":feature_index_file";

	File file;
	if (false == file.Open ((config)sName, ios_base::out))
		return false;

	file << "[prefixes]" << endl;
	ITERATE (FeatureValueToIndex_map_t, map_PrefixToIndex, ite)
		file << ite->second << '\x01' << ite->first << endl;
	
	file << endl;
	file << "[words]" << endl;
	ITERATE (FeatureValueToIndex_map_t, map_WordToIndex, ite)
		file << ite->second << '\x01' << ite->first << endl;

	file.Close ();
	return true;
}


//														
bool RelevanceModelFeatures::LoadIndices (const char* _zName)
{
	String sName;
	sName << _zName << ":feature_index_file";

	File file;
	if (false == file.Open ((config)sName))
		return false;

	String_dq_t dqLines;
	file.ReadLines (dqLines);

	char cSection = ' ';
	ITERATE (String_dq_t, dqLines, ite)
	{
		if ("[prefixes]" == *ite)
		{
			cSection = 'c';
			continue;
		}
		else if ("[words]" == *ite)
		{
			cSection = 'w';
			continue;
		}

		if (false == ite->Has ("\x01"))
			continue;

		String_dq_t dqValues;
		ite->Split (dqValues, '\x01');

		assert (2 == dqValues.size ());
		assert (true == dqValues [0].IsDigit ());

		if ('c' == cSection)
			map_PrefixToIndex.insert (make_pair (dqValues [1], dqValues [0]));
		else if ('w' == cSection)
			map_WordToIndex.insert (make_pair (dqValues [1], dqValues [0]));
	}

	i_WordCount = map_WordToIndex.size ();
	i_PrefixCount = map_PrefixToIndex.size ();

	return true;
}





//														
int OperatorModelFeatures::GetIndex (const String& _rToken)
{
	pthread_rwlock_rdlock (&rwl_IndexMap);
	FeatureValueToIndex_map_t::iterator	ite;
	ite = map_TokenToIndex.find (_rToken);
	if (map_TokenToIndex.end () != ite)
	{
		pthread_rwlock_unlock (&rwl_IndexMap);
		return ite->second;
	}
	pthread_rwlock_unlock (&rwl_IndexMap);

	pthread_rwlock_wrlock (&rwl_IndexMap);
	int iIndex = map_TokenToIndex.size ();
	map_TokenToIndex.insert (make_pair (_rToken, iIndex));
	i_TokenCount = map_TokenToIndex.size ();
	pthread_rwlock_unlock (&rwl_IndexMap);

	return iIndex;
}


//														
bool OperatorModelFeatures::SaveIndices (const char* _zName)
{
	String sName;
	sName << _zName << ":feature_index_file";

	File file;
	if (false == file.Open ((config)sName, ios_base::out))
		return false;
	
	ITERATE (FeatureValueToIndex_map_t, map_TokenToIndex, ite)
		file << ite->second << '\x01' << ite->first << endl;

	file.Close ();
	return true;
}


//														
bool OperatorModelFeatures::LoadIndices (const char* _zName)
{
	String sName;
	sName << _zName << ":feature_index_file";

	File file;
	if (false == file.Open ((config)sName))
		return false;

	String_dq_t dqLines;
	file.ReadLines (dqLines);

	ITERATE (String_dq_t, dqLines, ite)
	{
		if (false == ite->Has ("\x01"))
			continue;

		String_dq_t dqValues;
		ite->Split (dqValues, '\x01');

		assert (2 == dqValues.size ());
		assert (true == dqValues [0].IsDigit ());

		map_TokenToIndex.insert (make_pair (dqValues [1], dqValues [0]));
	}

	i_TokenCount = map_TokenToIndex.size ();

	return true;
}






