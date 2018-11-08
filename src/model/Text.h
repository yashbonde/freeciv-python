#ifndef __TEXT__
#define __TEXT__

#include <set>
#include <map>
#include <deque>
#include <nlp_string.h>
#include <nlp_vector.h>
#include <nlp_distr.h>
#include "FeatureSpecialization.h"
using namespace std;

class Hit;
class Sentence;
class FilteredSentences;
typedef set <int>							int_set_t;
typedef set <long>							long_set_t;
typedef set <String>						String_set_t;
typedef deque <int>							int_dq_t;
typedef deque <String_dq_t*>				SentenceStems_dq_t;
typedef deque <Sentence*>					Sentence_dq_t;
typedef map <int, long_set_t>				WordToPhrase_map_t;
typedef map <long, Hit*>					PhraseToHit_map_t;
typedef map <String, FilteredSentences*>	UnitNameToFilteredSentences_map_t;
typedef map <String, size_t>				StemToCount_map_t;
typedef multimap <String, String>			StemToLabel_mmp_t;
typedef Vector <Sentence*>					Sentence_Vec_t;


//														
String GetPorterStem (String& _rWord);


//											
class Hit
{
	public:
		long		l_PhraseIndex;
		long 		l_HitCount;
		String_dq_t	dq_StateLabels;
};


//											
class Sentence
{
	protected:
		Sentence (void);

	public:
		static StemToCount_map_t	map_StemToCount;
		static String_set_t			set_StopStems;
		static size_t				i_MinimumStemCount;
		static size_t				i_SentencesWithDependencyCycles;
		static bool					b_IncludeNonPredicateWords;

		long					i_Index;
		String					s_Sentence;
		String_dq_t				dq_Words;
		int						i_Words;
		int*					p_WordIndices;
		int*					p_DependencyOrder;
		int*					p_HeuristicLabels;
		int*					p_ParentIndex;

		int*					p_LabelXWord;
		int*					p_LabelXTag;
		int*					p_LabelXDependencyType;
		int*					p_LabelXParentWord;
		int*					p_LabelXParentTag;
		int*					p_LabelXIsLeaf;
		int*					p_LabelXHeuristicLabel;
		int*					p_LabelXParentLabel;
		int*					p_LabelXIsStateLabel;
		int*					p_LabelXIsUnitLabel;
		int*					p_LabelXIsActionLabel;
		int*					p_Labels;

		char*					p_WordPresent;

		String					s_ActionStateLabels;
		int						i_ActionStateWords;
		int*					p_ActionStateWordIndices;

		int*					p_LabeledWordIndexPrecomputed;

		Sentence*				p_OriginalSentence;
		bool					b_IsNullSentence;


		RelevanceModelFeatures	o_Features;
		RelevanceModelFeatures	o_FeaturesWithPrevSentenceMarker;

		// Sentence (String& _rSentence, String& _rLabels);
		static Sentence* CreateSentenceWithStateActionLabels (String& _rSentence,
															  String& _rLabels);
		static Sentence* CreateSentenceWithDependencyInfo (String& _rSentence,
														   String& _rDependencies,
														   String& _rHeuristicLabels);
		static Sentence* CreateNullSentence (void);
		~Sentence (void);

		void ComputeFeatures (void);

		bool IsLocallyAllocated (void)
		{ return (this != p_OriginalSentence); };
};


//											
class OperatorLabeledSentence : public Sentence
{
	public:
		OperatorLabeledSentence (Sentence& _rSentence);
		~OperatorLabeledSentence (void);

		void SetWordLabel (int _iWordLoc, int _iLabel)
		{
			p_Labels [_iWordLoc] = _iLabel;
			if ((0 != _iLabel) || (true == Sentence::b_IncludeNonPredicateWords))
			{
				// p_ActionStateWordIndices [_iWordLoc] = 
				p_ActionStateWordIndices [i_ActionStateWords] = 
					p_LabeledWordIndexPrecomputed [3 * _iWordLoc + _iLabel];
				++ i_ActionStateWords;
			}
		}

		int GetParentLabel (int _w)
		{
			if (-1 == p_ParentIndex [_w])
				return -1;
			return p_Labels [p_ParentIndex [_w]];
		}
};




//											
class FilteredSentences
{
	public:
		Sentence_Vec_t	vec_Sentences;
		long_set_t		set_SentenceIndices;	
};


//											
class TextInterface
{
	private:
		WordToPhrase_map_t					map_WordToPhrase;
		PhraseToHit_map_t					map_PhraseToHit;
		UnitNameToFilteredSentences_map_t	map_UnitNameToFilteredSentences;
		FilteredSentences					o_AllSentences;
		Sentence*							p_NullSentence;
		bool								b_FilterSentencesByUnitName;
		bool								b_UseDependencyInfo;

		bool Load (void);
		FilteredSentences* CreateFilteredSentenceList (String& _rUnitName);

	public:
		Sentence_dq_t		dq_Sentence;

		TextInterface (void);
		~TextInterface (void);

		bool Init (bool _bUseDependencyInfo);
		double GetSentenceScore (int_dq_t& _rdqActionLabels,
								 int_dq_t& _rdqAgentLabels,
								 Sentence* _pSentence);
		double GetBestSentenceAndScore (Sample& _rSample,
										int_dq_t& _rdqActionLabels,
										int_dq_t& _rdqAgentLabels,
										FilteredSentences* _pFilteredSentences,
										Sentence** _ppSentence);
		FilteredSentences* GetFilteredSentences (String& _rUnitName);
		FilteredSentences* GetAllSentences (void)
		{ return &o_AllSentences; };
		Sentence* GetNullSentence (void)
		{ return p_NullSentence; };

		void WriteFilteringStats (void);
		bool WriteStats (void);
};


#endif

