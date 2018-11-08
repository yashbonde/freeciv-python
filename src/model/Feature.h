#ifndef __FEATURES__
#define __FEATURES__

#include <map>
#include <deque>
#include <vector>
#include <nlp_string.h>
#include <nlp_vector.h>
using namespace std;


class Features;
class LinearFunctionApprox;
class RelevanceModelFeatures;
class OperatorModelFeatures;
typedef map <String, int>		FeatureValueToIndex_map_t;
typedef Vector <int>			int_Vec_t;
typedef Vector <float>			float_Vec_t;
typedef vector <double>			double_vec_t;
typedef vector <Features*>		Feature_vec_t;
typedef deque <Features*>		Feature_dq_t;


//													
class FeatureSpace
{
	private:
		FeatureValueToIndex_map_t	map_FeatureValueToIndex;
		int							i_BagOfWordsOffset;
		int							i_MaxIndex;
		pthread_rwlock_t			rwl_IndexMap;

	public:
		FeatureSpace (void);
		~FeatureSpace (void);

		int GetFeatureIndex (String& _rName);
		int MaxIndex (void)
		{ return i_MaxIndex; };
		int BagOfWordOffset (void)
		{ return i_BagOfWordsOffset; };

		void SetBagOfWordsOffset (int _iOffset);

		bool SaveFeatureMapping (String _sName);
		bool LoadFeatureMapping (String _sName);
};



//													
class Features
{
	friend class LinearFunctionApprox;
	friend class RelevanceModelFeatures;
	friend class OperatorModelFeatures;

	private:
		int_Vec_t	vec_Indices;
		float_Vec_t	vec_Features;
		size_t		i_Current;
		size_t		i_MaxSize;

	public:
		Features (void);
		Features (const Features& _rFeatures);
		~Features (void);

		void SetSize (int _iSize);
		void Set (FeatureSpace& _rSpace, int _iIndex, float _fValue);
		void Set (int _iIndex, float _fValue);
		void Set (const Features& _rFeatures);
		void SetBagOfWords (FeatureSpace& _rSpace, const char* _zFeature, float _fValue = 1);
		void SetBagOfWords (FeatureSpace& _rSpace, String& _rFeature, float _fValue = 1);
		void SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, char* _zFeatures, float _fValue = 1);
		void SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, zchar_dq_t& _rdqFeatures, float _fValue = 1);
		void SetBagOfWords (FeatureSpace& _rSpace, const char* _zPrefix, String_dq_t& _rdqFeatures, float _fValue = 1);

		Features& operator= (const Features& _rFeatures);

		int Size (void) const
		{ return i_Current; };
		int Index (int _i) const
		{
			assert (vec_Indices [_i] >= 0);
			return vec_Indices [_i];
		};
		float Feature (int _i) const
		{ return vec_Features [_i]; };

		double DotProduct (double_vec_t& _rvecWeights);
		bool Check (void) const;
};

ostream& operator<< (ostream& _rStream, const Features& _rFeatures);
ostream& operator<< (ostream& _rStream, const double_vec_t& _rvecValues);

#endif
