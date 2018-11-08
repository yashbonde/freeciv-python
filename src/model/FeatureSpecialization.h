#ifndef __FEATURE_SPECIALIZATION__
#define __FEATURE_SPECIALIZATION__

#include <deque>
#include "Feature.h"
using namespace std;

typedef deque <int>	int_dq_t;


//													
class LinearQFnFeatures : public Features
{
	private:
		static FeatureSpace	o_FeatureSpace;

	public:
		LinearQFnFeatures (void);
		LinearQFnFeatures (const LinearQFnFeatures& _rFeatures)
			: Features (_rFeatures) {};

		static void SetBagOfWordsOffset (int _iOffset)
		{ o_FeatureSpace.SetBagOfWordsOffset (_iOffset); };
		static int MaxIndex (void)
		{ return o_FeatureSpace.MaxIndex (); };

		inline void Set (const LinearQFnFeatures& _rFeatures)
		{ Features::Set (_rFeatures); };

		inline void Set (int _iIndex, float _fValue)
		{ Features::Set (o_FeatureSpace, _iIndex, _fValue); };

		inline void SetBagOfWords (const char* _zFeature, float _fValue = 1)
		{ Features::SetBagOfWords (o_FeatureSpace, _zFeature, _fValue); };

		inline void SetBagOfWords (String& _rFeature, float _fValue = 1)
		{ Features::SetBagOfWords (o_FeatureSpace, _rFeature, _fValue); };

		inline void SetBagOfWords (const char* _zPrefix, char* _zFeatures, float _fValue = 1)
		{ Features::SetBagOfWords (o_FeatureSpace, _zPrefix, _zFeatures, _fValue); };

		inline void SetBagOfWords (const char* _zPrefix, zchar_dq_t& _rdqFeatures, float _fValue = 1)
		{ Features::SetBagOfWords (o_FeatureSpace, _zPrefix, _rdqFeatures, _fValue); };

		inline void SetBagOfWords (const char* _zPrefix, String_dq_t& _rdqFeatures, float _fValue = 1)
		{ Features::SetBagOfWords (o_FeatureSpace, _zPrefix, _rdqFeatures, _fValue); };

		static bool SaveFeatureMapping (void)
		{ return o_FeatureSpace.SaveFeatureMapping ("lqfn"); };
		static bool LoadFeatureMapping (void)
		{ return o_FeatureSpace.LoadFeatureMapping ("lqfn"); };
};


//													
class RelevanceModelFeatures : public Features
{
	private:
		static FeatureValueToIndex_map_t	map_WordToIndex;
		static FeatureValueToIndex_map_t	map_PrefixToIndex;
		static FeatureValueToIndex_map_t	map_OperatorToIndex;
		static int							i_WordCount;
		static int							i_PrefixCount;
		static int							i_OperatorCount;
		static pthread_rwlock_t				rwl_IndexMap;

	public:
		RelevanceModelFeatures (void)
			: Features () {};
		RelevanceModelFeatures (const RelevanceModelFeatures& _rFeatures)
			: Features (_rFeatures) {};

		static int FindWordIndex (const String& _rWord);
		static int GetWordIndex (const String& _rWord);
		static int GetOperatorIndex (const String& _rWord);
		static int GetPrefixIndex (const String& _rPrefix);
		static int MaxWordIndex (void)
		{ return map_WordToIndex.size (); };

		static int WordCount (void)
		{ return i_WordCount; };
		static int OperatorCount (void)
		{ return i_OperatorCount; };
		static int MaxIndex (void)
		{ return i_WordCount * i_PrefixCount; };

		inline void Set (const RelevanceModelFeatures& _rFeatures)
		{ Features::Set (_rFeatures); };

		inline void SetX (int _iIndex, float _fValue)
		{ Features::Set (i_WordCount + _iIndex, _fValue); };

		void SetPrefixFeature (int _iPrefix, int _iFeatureId);
		void SetBagOfWords (int _iPrefix, int* _pFeatureIds, int _iFeatures, float _fValue = 1);

		static bool SaveIndices (const char* _zName);
		static bool LoadIndices (const char* _zName);
};


//														
class OperatorModelFeatures : public Features
{
	private:
		static FeatureValueToIndex_map_t	map_TokenToIndex;
		static int							i_TokenCount;
		static pthread_rwlock_t				rwl_IndexMap;

	public:
		OperatorModelFeatures (void)
			: Features () {};
		OperatorModelFeatures (const OperatorModelFeatures& _rFeatures)
			: Features (_rFeatures) {};

		static int GetIndex (const String& _rToken);
		static int MaxIndex (void)
		{ return i_TokenCount; };

		inline void Set (int _iIndex, float _fValue)
		{ Features::Set (_iIndex, _fValue); };

		static bool SaveIndices (const char* _zName);
		static bool LoadIndices (const char* _zName);
};


#endif
