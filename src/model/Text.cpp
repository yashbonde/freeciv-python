#include "Text.h"
#include <nlp_config.h>
#include <nlp_filesystem.h>
#include <nlp_macros.h>
#include <nlp_distr.h>

typedef map <long, int>	PhraseToCount_map_t;

String_set_t		Sentence::set_StopStems;
StemToCount_map_t	Sentence::map_StemToCount;
size_t				Sentence::i_MinimumStemCount = 0;
size_t				Sentence::i_SentencesWithDependencyCycles = 0;
bool				Sentence::b_IncludeNonPredicateWords = false;


//													
String GetPorterStem (String& _rWord)
{
	stemmer* pStemmer = PorterStemmer::CreateStemmer ();

	size_t iLength = _rWord.length ();
	char zStem [iLength + 10];
	strncpy (zStem, (const char*)_rWord, iLength);
	zStem [iLength] = '\0';

	size_t ret = PorterStemmer::Stem (pStemmer, zStem, iLength);
	zStem [(ret < iLength)? ret + 1 : iLength] = '\0';
	String sStem = zStem;

	PorterStemmer::FreeStemmer (pStemmer);

	return sStem;
}



//											
Sentence::Sentence (void)
{
	i_Index = -1;
	i_Words = 0;
	i_ActionStateWords = 0;

	p_WordIndices = NULL;
	p_DependencyOrder = NULL;
	p_HeuristicLabels = NULL;
	p_ParentIndex = NULL;

	// the following store pre-computed	
	// feature values for label * item	
	p_LabelXWord = NULL;
	p_LabelXTag = NULL;
	p_LabelXDependencyType = NULL;
	p_LabelXParentWord = NULL;
	p_LabelXParentTag = NULL;
	p_LabelXIsLeaf = NULL;
	p_LabelXHeuristicLabel = NULL;
	p_LabelXParentLabel = NULL;
	p_LabelXIsStateLabel = NULL;
	p_LabelXIsUnitLabel = NULL;
	p_LabelXIsActionLabel = NULL;
	p_Labels = NULL;

	p_WordPresent = NULL;
	p_ActionStateWordIndices = NULL;
	p_LabeledWordIndexPrecomputed = NULL;

	// This pointer reference is needed by the		
	// sentence relevance code...					
	p_OriginalSentence = this;
	b_IsNullSentence = false;
}


//											
Sentence* Sentence::CreateSentenceWithStateActionLabels (String& _rSentence,
														 String& _rLabels)
{
	Sentence* p = new Sentence;

	p->s_Sentence = _rSentence;
	p->s_ActionStateLabels = _rLabels;

	String_dq_t dqWords;
	p->s_Sentence.Split (dqWords);
	String_dq_t dqLabels;
	p->s_ActionStateLabels.Split (dqLabels, ',');

	if (dqWords.size () != dqLabels.size ())
	{
		cout << "[ERROR] Count mismatch between words ("
			 << dqWords.size () << ") and labels ("
			 << dqLabels.size () << ") in sentence.\n"
			 << p->s_Sentence << '\n'
			 << p->s_ActionStateLabels << endl;
		abort ();
	}

	StemToCount_map_t mapStemCount;
	StemToLabel_mmp_t mmpStemToLabel;
	for (size_t w = 0; w < dqWords.size (); ++ w)
	{
		String sStem = dqWords [w].GetPorterStem ();

		StemToCount_map_t::iterator	iteStem;
		iteStem = Sentence::map_StemToCount.find (sStem);
		if (Sentence::map_StemToCount.end () == iteStem)
			continue;
		if (iteStem->second < Sentence::i_MinimumStemCount)
			continue;

		p->dq_Words.push_back (sStem);
		StemToCount_map_t::iterator	iteCount;
		iteCount = mapStemCount.find (sStem);
		if (mapStemCount.end () == iteCount)
			mapStemCount.insert (make_pair (sStem, 1));
		else
			++ iteCount->second;
		mmpStemToLabel.insert (make_pair (sStem, dqLabels [w]));
	}

	p->i_Words = mapStemCount.size ();

	p->p_WordIndices = new int [p->i_Words];
	#ifndef NDEBUG
	memset (p->p_WordIndices, -1, p->i_Words);
	#endif

	// sort word indices before inserting to	
	// p_WordIndices to improve cache locality	
	// when used later...						
	{
		int_set_t setStems;
		ITERATE (StemToCount_map_t, mapStemCount, ite)
		{
			int iStem = RelevanceModelFeatures::GetWordIndex (ite->first);
			setStems.insert (iStem);
		}

		int i = 0;
		ITERATE (int_set_t, setStems, ite)
			p->p_WordIndices [i++] = *ite;
	}

	// create action & state word array			
	{
		int_set_t setStems;
		ITERATE (StemToLabel_mmp_t, mmpStemToLabel, ite)
		{
			if (false == b_IncludeNonPredicateWords)
			{
				if (" " == ite->second)
					continue;
			}

			String sToken;
			sToken << ite->second << '\x01' << ite->first;
			int iStem = RelevanceModelFeatures::GetOperatorIndex (sToken);
			setStems.insert (iStem);
		}

		p->i_ActionStateWords = setStems.size ();
		p->p_ActionStateWordIndices = new int [p->i_ActionStateWords];

		int i = 0;
		ITERATE (int_set_t, setStems, ite)
			p->p_ActionStateWordIndices [i++] = *ite;
	}

	return p;
}


//											
Sentence* Sentence::CreateSentenceWithDependencyInfo (String& _rSentence,
													  String& _rDependencies,
													  String& _rHeuristicLabels)
{
	Sentence* p = new Sentence;
	p->s_Sentence = _rSentence;

	String_dq_t dqWords;
	p->s_Sentence.Split (dqWords);

	String_dq_t dqDependencies;
	_rDependencies.Split (dqDependencies, '\x02');
	if (dqWords.size () != dqDependencies.size ())
	{
		cout << "[ERROR] Size mismatch between words in sentence & dependency information.\n"
			 << "        " << p->s_Sentence << '\n'
			 << "        " << _rDependencies << endl;
		return NULL;
	}

	String_dq_t dqLabels;
	_rHeuristicLabels.Split (dqLabels, ',');
	if (dqWords.size () != dqLabels.size ())
	{
		cout << "[ERROR] Count mismatch between words ("
			 << dqWords.size () << ") and labels ("
			 << dqLabels.size () << ") in sentence.\n"
			 << p->s_Sentence << '\n'
			 << _rHeuristicLabels << endl;
		abort ();
	}


	size_t iWordCount = 0;
	for (size_t w = 0; w < dqWords.size (); ++ w)
	{
		// word information ...		
		String sStem = dqWords [w].GetPorterStem ();

		StemToCount_map_t::iterator	iteStem;
		iteStem = Sentence::map_StemToCount.find (sStem);
		if (Sentence::map_StemToCount.end () == iteStem)
			continue;
		if (iteStem->second < Sentence::i_MinimumStemCount)
			continue;

		++ iWordCount;
	}


	p->p_WordIndices = new int [iWordCount];
	p->p_DependencyOrder = new int [iWordCount];
	p->p_HeuristicLabels = new int [iWordCount];
	p->p_ParentIndex = new int [iWordCount];

	p->p_LabelXWord = new int [3 * iWordCount];
	p->p_LabelXTag = new int [3 * iWordCount];
	p->p_LabelXDependencyType = new int [3 * iWordCount];
	p->p_LabelXParentWord = new int [3 * iWordCount];
	p->p_LabelXParentTag = new int [3 * iWordCount];
	p->p_LabelXParentLabel = new int [4 * 3 * iWordCount];
	p->p_LabelXIsLeaf = new int [3 * iWordCount];
	p->p_LabelXHeuristicLabel = new int [3 * iWordCount];
	p->p_LabelXIsStateLabel = new int [3 * 2];
	p->p_LabelXIsUnitLabel = new int [3 * 2];
	p->p_LabelXIsActionLabel = new int [3 * 2];
	p->p_LabeledWordIndexPrecomputed = new int [3 * iWordCount];

	int pCompleteParentIndex [dqWords.size ()];
	int pAdjustedWordIndex [dqWords.size ()];

	p->i_Words = 0;
	for (size_t w = 0; w < dqWords.size (); ++ w)
	{
		#define DEP_TYPE	0
		#define FROM_INDEX	1
		#define FROM_WORD	2
		#define FROM_TAG	3
		#define TO_WORD		4
		#define TO_TAG		5
		#define IS_LEAF		6

		String_dq_t	dqDeps;
		dqDependencies [w].Split (dqDeps, '\x01');
		if (dqDeps.size () != 7)
		{
			cout << "[ERROR] Dependency file format error. Number of information items for word is not equal to 7 : " << dqDeps.size () << endl;
			cout << "        " << dqDependencies [w] << endl;
			abort ();
		}

		pCompleteParentIndex [w] = dqDeps [FROM_INDEX];
		pAdjustedWordIndex [w] = -1;
		if ((int)dqDeps [FROM_INDEX] == (int)w)
		{
			pCompleteParentIndex [w] = -1;

			// cout << dqDeps [FROM_INDEX] << " == " << w << endl;
			// cout << dqDependencies [w] << endl;
			// cout << _rDependencies << endl;
		}
		// assert ((int)dqDeps [FROM_INDEX] != w);

		String sStem = dqWords [w].GetPorterStem ();
		StemToCount_map_t::iterator	iteStem;
		iteStem = Sentence::map_StemToCount.find (sStem);
		if (Sentence::map_StemToCount.end () == iteStem)
			continue;
		if (iteStem->second < Sentence::i_MinimumStemCount)
			continue;

		pAdjustedWordIndex [w] = p->i_Words;
		p->dq_Words.push_back (sStem);

		int iStem = RelevanceModelFeatures::GetWordIndex (sStem);
		p->p_WordIndices [p->i_Words] = iStem;
		int iHeuristicLabel = RelevanceModelFeatures::GetWordIndex (dqLabels [w]);
		p->p_HeuristicLabels [p->i_Words] = iHeuristicLabel;

		// dependency information ...
		// [dependency type][from index][from word][from tag]to word][to tag][leaf]
		assert ((dqWords [w] == dqDeps [TO_WORD]) || (" " == dqDeps [TO_WORD]));

		// The following are pre-computed feature values	
		// for each possible label value * word info.		
		for (int l = 0; l < 3; ++ l)
		{
			String sWord;
			sWord << l << '\x01' << sStem;
			p->p_LabelXWord [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sWord);

			String sTag;
			sTag << l << '\x02' << dqDeps [TO_TAG];
			p->p_LabelXTag [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sTag);

			String sDep;
			sDep << l << '\x03' << dqDeps [DEP_TYPE];
			p->p_LabelXDependencyType [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sDep);

			String sParentWord;
			sParentWord << l << '\x04' << dqDeps [FROM_WORD].GetPorterStem ();
			p->p_LabelXParentWord [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sParentWord);

			String sParentTag;
			sParentTag << l << '\x05' << dqDeps [FROM_TAG];
			p->p_LabelXParentTag [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sParentTag);

			String sLeaf;
			sLeaf << l << '\x06' << dqDeps [IS_LEAF];
			p->p_LabelXIsLeaf [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sLeaf);

			String sHeuristicLabel;
			sHeuristicLabel << l << '\x07' << iHeuristicLabel;
			p->p_LabelXHeuristicLabel [3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sHeuristicLabel);

			for (int pl = 0; pl < 4; ++ pl)
			{
				String sLabelsX;
				sLabelsX << pl << '\x08' << l;
				p->p_LabelXParentLabel [(3 * iWordCount) * pl + 3 * p->i_Words + l]
					= OperatorModelFeatures::GetIndex (sLabelsX);
			}
		}

		// precompute label assignment feature indices for each word ...
		for (int l = 0; l < 3; ++ l)
		{
			String sLabeledStem;
			sLabeledStem << l << '\x01' << sStem;
			p->p_LabeledWordIndexPrecomputed [3 * p->i_Words + l]
					= RelevanceModelFeatures::GetOperatorIndex (sLabeledStem);
		}
	
		++ p->i_Words;
	}

	for (int i = 0; i < 2; ++ i)
	{
		for (int l = 0; l < 3; ++ l)
		{
			{
				String sToken;
				sToken << l << '\x09' << "__STATE_LABEL__" << i;
				p->p_LabelXIsStateLabel [3 * i + l] = OperatorModelFeatures::GetIndex (sToken);
			}
			{
				String sToken;
				sToken << l << '\x09' << "__UNIT_LABEL__" << i;
				p->p_LabelXIsUnitLabel [3 * i + l] = OperatorModelFeatures::GetIndex (sToken);
			}
			{
				String sToken;
				sToken << l << '\x09' << "__ACTION_LABEL__" << i;
				p->p_LabelXIsActionLabel [3 * i + l] = OperatorModelFeatures::GetIndex (sToken);
			}
		}
	}

	// compute dependency word order ...				
	// 1. Adjust parent word indices to account for the	
	//    words we ignored above...						
	for (int w = 0; w < (int)dqWords.size (); ++ w)
	{
		if (-1 != pAdjustedWordIndex [w])
			continue;
		
		// This word was not retained. We need to 		
		// correct the parent indices of it's children.	
		for (int x = 0; x < (int)dqWords.size (); ++ x)
		{
			if (pCompleteParentIndex [x] == w)
				pCompleteParentIndex [x] = pCompleteParentIndex [w];
		}
	}

	p->p_ParentIndex = new int [iWordCount];
	for (size_t w = 0; w < iWordCount; ++ w)
		p->p_ParentIndex [w] = -1;

	for (size_t w = 0; w < dqWords.size (); ++ w)
	{
		if (-1 == pAdjustedWordIndex [w])
			continue;
		int iAdjustedParentIndex = -1;
		if (-1 != pCompleteParentIndex [w])
			iAdjustedParentIndex = pAdjustedWordIndex [pCompleteParentIndex [w]];
		p->p_ParentIndex [pAdjustedWordIndex [w]] = iAdjustedParentIndex;
	}
	for (int i = 0; i < (int)iWordCount; ++ i)
	{
		if (p->p_ParentIndex [i] == i)
			p->p_ParentIndex [i] = -1;
	}


	// 2. Construct dependency order...					
	int iMoved [iWordCount];
	memset (iMoved, 0, iWordCount * sizeof (int));

	bool bDone = false;
	int d = 0;
	while (false == bDone)
	{
		bDone = true;
		for (size_t w = 0; w < iWordCount; ++ w)
		{
			if (1 == iMoved [w])
				continue;
			if ((-1 != p->p_ParentIndex [w]) &&
				(1 != iMoved [p->p_ParentIndex [w]]))
				continue;

			p->p_DependencyOrder [d ++] = w;
			iMoved [w] = 1;
			bDone = false;
		}
	}

	// At this point, all the words should have been	
	// moved into a dependency order, but it seems there
	// are cyclic dependencies in the parse trees, so	
	// this is an arbitrary cycle breaker...			
	bool bHadCycles = false;
	for (size_t w = 0; w < iWordCount; ++ w)
	{
		if (1 == iMoved [w])
			continue;
		p->p_DependencyOrder [d ++] = w;
		iMoved [w] = 1;
		bHadCycles = true;
	}
	if (true == bHadCycles)
		++ i_SentencesWithDependencyCycles;


	if (d != (int)iWordCount)
	{
		cout << "[ERROR] bounds check failed on index used to initialize dependency order: "
			 << d << " > " << iWordCount << " (word count)" << endl;
		abort ();
	}

	return p;
}

//											
Sentence* Sentence::CreateNullSentence (void)
{
	Sentence* p = new Sentence;
	p->b_IsNullSentence = true;
	p->i_Words = 1;
	p->p_WordIndices = new int [1];
	p->p_WordIndices [0] = RelevanceModelFeatures::GetWordIndex ("__NULL_SENTENCE_WORD__");
	return p;
}


//											
Sentence::~Sentence (void)
{
	delete[] p_WordIndices;
	delete[] p_DependencyOrder;
	delete[] p_HeuristicLabels;
	delete[] p_ParentIndex;

	delete[] p_LabelXWord;
	delete[] p_LabelXTag;
	delete[] p_LabelXDependencyType;
	delete[] p_LabelXParentWord;
	delete[] p_LabelXParentTag;
	delete[] p_LabelXIsLeaf;
	delete[] p_LabelXHeuristicLabel;
	delete[] p_LabelXParentLabel;
	delete[] p_LabelXIsStateLabel;
	delete[] p_LabelXIsUnitLabel;
	delete[] p_LabelXIsActionLabel;

	delete[] p_WordPresent;
	delete[] p_ActionStateWordIndices;
	delete[] p_LabeledWordIndexPrecomputed;
}


//											
void Sentence::ComputeFeatures (void)
{
	String sPrefix;
	sPrefix = "w";
	int iPrefix = RelevanceModelFeatures::GetPrefixIndex (sPrefix);
	o_Features.SetSize (i_Words);
	o_Features.SetBagOfWords (iPrefix, p_WordIndices, i_Words);

	sPrefix = "psw";
	iPrefix = RelevanceModelFeatures::GetPrefixIndex (sPrefix);
	o_FeaturesWithPrevSentenceMarker.SetSize (i_Words);
	o_FeaturesWithPrevSentenceMarker.SetBagOfWords (iPrefix, p_WordIndices, i_Words);

	size_t iTotalWords = RelevanceModelFeatures::WordCount ();
	p_WordPresent = new char [iTotalWords];
	memset (p_WordPresent, 0, sizeof(char) * iTotalWords);
	for (int i = 0; i < i_Words; ++i)
		p_WordPresent [p_WordIndices [i]] = 1;
}


//											
OperatorLabeledSentence::OperatorLabeledSentence (Sentence& _rSentence)
{
	// This pointer reference is needed by the		
	// sentence relevance code...					
	p_OriginalSentence = &_rSentence;

	i_Index = _rSentence.i_Index;
	// we don't copy over s_Sentence or dq_Words	
	// here since it is	expensive to copy, and is	
	// not needed within OperatorLabeledSentence.	
	i_Words = _rSentence.i_Words;

	// We do a shallow copy for the following since	
	// they are reference data and are not changed	
	// after loadup.  A deep copy is redundant.		
	p_WordIndices = _rSentence.p_WordIndices;
	p_DependencyOrder = _rSentence.p_DependencyOrder;
	p_HeuristicLabels = _rSentence.p_HeuristicLabels;
	p_ParentIndex = _rSentence.p_ParentIndex;

	p_LabelXWord = _rSentence.p_LabelXWord;
	p_LabelXTag = _rSentence.p_LabelXTag;
	p_LabelXDependencyType = _rSentence.p_LabelXDependencyType;
	p_LabelXParentWord = _rSentence.p_LabelXParentWord;
	p_LabelXParentTag = _rSentence.p_LabelXParentTag;
	p_LabelXIsLeaf = _rSentence.p_LabelXIsLeaf;
	p_LabelXHeuristicLabel = _rSentence.p_LabelXHeuristicLabel;
	p_LabelXParentLabel = _rSentence.p_LabelXParentLabel;
	p_LabelXIsStateLabel = _rSentence.p_LabelXIsStateLabel;
	p_LabelXIsUnitLabel = _rSentence.p_LabelXIsUnitLabel;
	p_LabelXIsActionLabel = _rSentence.p_LabelXIsActionLabel;

	p_WordPresent = _rSentence.p_WordPresent;
	p_LabeledWordIndexPrecomputed = _rSentence.p_LabeledWordIndexPrecomputed;
	b_IsNullSentence = _rSentence.b_IsNullSentence;

	// We don't copy s_ActionStateLabels, 			
	// i_ActionStateWords & p_ActionStateWordIndices
	// because those need to be set for each word	
	// by the model that predicts the word op labels

	// i_ActionStateWords = i_Words;
	i_ActionStateWords = 0;
	p_ActionStateWordIndices = new int [i_Words];

	p_Labels = new int [i_Words];
	memset (p_Labels, 0, i_Words * sizeof (int));
}


OperatorLabeledSentence::~OperatorLabeledSentence (void)
{
	// If p_OriginalSentence does not refer to this		
	// object, then we have a OperatorLabeledSentence.	
	// Otherwise, it's a base class Sentence object, 	
	// and we let the base class destructor take care	
	// of it...											
	if (this != p_OriginalSentence)
	{
		// The only pointer data item that is locally	
		// allocated is p_ActionStateWordIndices. So 	
		// that's the only thing that needs to be 		
		// deallocated here.  DO NOT deallocate the 	
		// shallow copied pointers.  See constructor.	
		p_WordIndices = NULL;
		p_DependencyOrder = NULL;
		p_HeuristicLabels = NULL;
		p_ParentIndex = NULL;

		p_LabelXWord = NULL;
		p_LabelXTag = NULL;
		p_LabelXDependencyType = NULL;
		p_LabelXParentWord = NULL;
		p_LabelXParentTag = NULL;
		p_LabelXIsLeaf = NULL;
		p_LabelXHeuristicLabel = NULL;
		p_LabelXParentLabel = NULL;
		p_LabelXIsStateLabel = NULL;
		p_LabelXIsUnitLabel = NULL;
		p_LabelXIsActionLabel = NULL;

		p_WordPresent = NULL;
		p_LabeledWordIndexPrecomputed = NULL;

		delete[] p_ActionStateWordIndices;
		p_ActionStateWordIndices = NULL;
		delete[] p_Labels;
		p_Labels = NULL;
		i_ActionStateWords = -1;
	}
}




//											
TextInterface::TextInterface (void)
{
	b_UseDependencyInfo = false;
}


//											
TextInterface::~TextInterface (void)
{
	ITERATE (Sentence_dq_t, dq_Sentence, ite)
		delete *ite;
	dq_Sentence.clear ();

	ITERATE (UnitNameToFilteredSentences_map_t, map_UnitNameToFilteredSentences, ite)
	{
		if (&o_AllSentences != ite->second)
			delete ite->second;
	}
	map_UnitNameToFilteredSentences.clear ();
}


//											
bool TextInterface::Init (bool _bUseDependencyInfo)
{
	b_FilterSentencesByUnitName = (1 == (int)(config)"filter_sentences_by_unit_name");
	b_UseDependencyInfo = _bUseDependencyInfo;
	Sentence::i_MinimumStemCount = (long)(config)"minimum_stem_count";
	Sentence::b_IncludeNonPredicateWords = (1 == (int)(config)"include_non_predicate_words");
	return Load ();
}


//											
bool TextInterface::Load (void)
{
	// load stop words ...			
	String_dq_t dqStops;
	if (false == File::ReadLines ((config)"stopword_file", dqStops))
		return false;
	ITERATE (String_dq_t, dqStops, ite)
	{
		ite->LowerCase ();
		String sStem = ite->GetPorterStem ();
		Sentence::set_StopStems.insert (sStem);
	}


	// load text ...				
	String_dq_t dqSentences;
	if (false == File::ReadLines ((config)"text_file", dqSentences))
		return false;

	// load up word operator annotations ...	
	String_dq_t dqLabels;
	if (false == File::ReadLines ((config)"action_state_label_file", dqLabels))
		return false;

	if (dqSentences.size () != dqLabels.size ())
	{
		cout << "[ERROR] Count mismatch between sentences ("
			 << dqSentences.size () << " lines) and action-state labels ("
			 << dqLabels.size () << " lines)." << endl;
		return false;
	}

	// load dependency information if we're 	
	// using the dependency model...			
	String_dq_t dqDependencies;
	if (true == b_UseDependencyInfo)
	{
		String_dq_t dqLines;
		if (false == File::ReadLines ((config)"dependency_file", dqLines))
			return false;
		ITERATE (String_dq_t, dqLines, ite)
		{
			if (true == ite->StartsWith ("#"))
				continue;
			dqDependencies.push_back (*ite);
		}

		if (dqSentences.size () != dqDependencies.size ())
		{
			cout << "[ERROR] Count mismatch between sentences ("
				 << dqSentences.size () << " lines) and dependency information ("
				 << dqDependencies.size () << " lines)." << endl;
			return false;
		}
	}


	// Count stems so we can drop rare stems...			
	ITERATE (String_dq_t, dqSentences, iteLine)
	{
		iteLine->LowerCase ();
		// we're going to assume tokenized text since we're also	
		// assuming parse information here...						
		// iteLine->PennTokenize ();

		String_dq_t dqWords;
		iteLine->Split (dqWords);

		ITERATE (String_dq_t, dqWords, ite)
		{
			String s = *ite;
			s.Replace ("_", "");
			if (false == s.IsAlNum ())
				continue;
			String sStem = ite->GetPorterStem ();
			if (Sentence::set_StopStems.end () != Sentence::set_StopStems.find (sStem))
				continue;

			StemToCount_map_t::iterator	iteStem;
			iteStem = Sentence::map_StemToCount.find (sStem);
			if (Sentence::map_StemToCount.end () == iteStem)
				Sentence::map_StemToCount.insert (make_pair (sStem, 1));
			else
				++ iteStem->second;
		}
	}


	// load sentences information...					
	long iPhrase = 0;
	for (size_t l = 0; l < dqSentences.size (); ++ l)
	{
		Sentence* pSentence = NULL;
		if (true == b_UseDependencyInfo)
			pSentence = Sentence::CreateSentenceWithDependencyInfo (dqSentences [l],
																	dqDependencies [l],
																	dqLabels [l]);
		else
			pSentence = Sentence::CreateSentenceWithStateActionLabels (dqSentences [l],
																	   dqLabels [l]);
		pSentence->i_Index = dq_Sentence.size ();
		dq_Sentence.push_back (pSentence);

		for (int i = 0; i < pSentence->i_Words; ++ i)
		{
			pair <WordToPhrase_map_t::iterator, bool> pairWord;
			pairWord = map_WordToPhrase.insert (make_pair (pSentence->p_WordIndices [i],
														   long_set_t()));
			pairWord.first->second.insert (iPhrase);
		}

		++ iPhrase;
	}

	p_NullSentence = Sentence::CreateNullSentence ();
	p_NullSentence->i_Index = dq_Sentence.size ();
	dq_Sentence.push_back (p_NullSentence);

	ITERATE (Sentence_dq_t, dq_Sentence, ite)
		(*ite)->ComputeFeatures ();


	// create a FilteredSentences object containing all	
	// sentences ...									
	o_AllSentences.vec_Sentences.Create (dq_Sentence.size ());
	#ifndef NDEBUG
	o_AllSentences.vec_Sentences.Memset (0);
	#endif

	size_t i = 0;
	ITERATE (Sentence_dq_t, dq_Sentence, ite)
	{
		Sentence* pSentence = *ite;
		o_AllSentences.vec_Sentences [i++] = pSentence;
		o_AllSentences.set_SentenceIndices.insert (pSentence->i_Index);
	}


	//													
	cout << "Text loaded.\n"
		 << "   unique words      : "
		 << RelevanceModelFeatures::MaxWordIndex () << '\n'
		 << "   total sentences   : " << dq_Sentence.size () << '\n'
		 << "   dependency cycles : " << Sentence::i_SentencesWithDependencyCycles
		 << endl;
	return true;
}


//											
double TextInterface::GetSentenceScore (int_dq_t& _rdqActionLabels,
										int_dq_t& _rdqAgentLabels,
										Sentence* _pSentence)
{
	double dScore = 0;
	ITERATE (int_dq_t, _rdqActionLabels, ite)
	{
		// if (_pSentence->set_Words.end () != _pSentence->set_Words.find (*ite))
		if (0 != _pSentence->p_WordPresent [*ite])
			++ dScore;
	}
	ITERATE (int_dq_t, _rdqAgentLabels, ite)
	{
		// if (_pSentence->set_Words.end () != _pSentence->set_Words.find (*ite))
		if (0 != _pSentence->p_WordPresent [*ite])
			++ dScore;
	}
	return dScore;
}


//											
double TextInterface::GetBestSentenceAndScore (Sample& _rSample,
												int_dq_t& _rdqActionLabels,
												int_dq_t& _rdqAgentLabels,
												FilteredSentences* _pFilteredSentences,
												Sentence** _ppSentence)
{
	if (NULL == _pFilteredSentences)
	{
		*_ppSentence = NULL;
		return 0;
	}


	double dScore = 0;
	long_dq_t dqMaxMatchPhraseIndices;

	PhraseToCount_map_t	mapPhraseToCount;
	ITERATE (int_dq_t, _rdqActionLabels, iteWord)
	{
		WordToPhrase_map_t::iterator	ite;
		ite = map_WordToPhrase.find (*iteWord);
		if (map_WordToPhrase.end () == ite)
			continue;

		long_set_t& setPhraseIds = ite->second;
		ITERATE (long_set_t, setPhraseIds, iteId)
		{
			// only use sentences in the subset to 	
			// look for overlaps...					
			if (_pFilteredSentences->set_SentenceIndices.end ()
					== _pFilteredSentences->set_SentenceIndices.find (*iteId))
				continue;

			pair <PhraseToCount_map_t::iterator, bool> pairInsert;
			pairInsert = mapPhraseToCount.insert (make_pair (*iteId, 1));
			if (false == pairInsert.second)
				++ pairInsert.first->second;

			if (dScore < pairInsert.first->second)
			{
				dScore = pairInsert.first->second;
				dqMaxMatchPhraseIndices.clear ();
				dqMaxMatchPhraseIndices.push_back (*iteId);
			}
			else if (dScore == pairInsert.first->second)
				dqMaxMatchPhraseIndices.push_back (*iteId);
		}
	}

	ITERATE (int_dq_t, _rdqAgentLabels, iteWord)
	{
		WordToPhrase_map_t::iterator	ite;
		ite = map_WordToPhrase.find (*iteWord);
		if (map_WordToPhrase.end () == ite)
			continue;

		long_set_t& setPhraseIds = ite->second;
		ITERATE (long_set_t, setPhraseIds, iteId)
		{
			// only use sentences in the subset to 	
			// look for overlaps...					
			if (_pFilteredSentences->set_SentenceIndices.end ()
					== _pFilteredSentences->set_SentenceIndices.find (*iteId))
				continue;

			pair <PhraseToCount_map_t::iterator, bool> pairInsert;
			pairInsert = mapPhraseToCount.insert (make_pair (*iteId, 1));
			if (false == pairInsert.second)
				++ pairInsert.first->second;

			if (dScore < pairInsert.first->second)
			{
				dScore = pairInsert.first->second;
				dqMaxMatchPhraseIndices.clear ();
				dqMaxMatchPhraseIndices.push_back (*iteId);
			}
			else if (dScore == pairInsert.first->second)
				dqMaxMatchPhraseIndices.push_back (*iteId);
		}
	}


	// keep hit stats...		
	/*
	#pragma omp critical
	{
		if (-1 != lMaxMatchPhraseIndex)
		{
			Hit* pHit = NULL;
			PhraseToHit_map_t::iterator	iteHit;
			iteHit = map_PhraseToHit.find (lMaxMatchPhraseIndex);
			if (map_PhraseToHit.end () == iteHit)
			{
				pHit = new Hit;
				pHit->l_HitCount = 0;
				pHit->l_PhraseIndex = lMaxMatchPhraseIndex;
				map_PhraseToHit.insert (make_pair (lMaxMatchPhraseIndex, pHit));
			}
			else
				pHit = iteHit->second;

			++ pHit->l_HitCount;
			String sLabels;
			sLabels.Join (_rdqActionLabels);
			pHit->dq_StateLabels.push_back (sLabels);
		}
	}
	*/

	if ((NULL != _ppSentence) && (false == dqMaxMatchPhraseIndices.empty ()))
	{
		size_t lSample = _rSample.SampleUniformCategorical (dqMaxMatchPhraseIndices.size ());
		*_ppSentence = dq_Sentence [dqMaxMatchPhraseIndices [lSample]];
	}
	else
		*_ppSentence = NULL;

	//							
	if (0 == dScore)
		dScore = 1;
	return dScore;
}


//											
FilteredSentences* TextInterface::CreateFilteredSentenceList (String& _rUnitName)
{
	if (true == b_FilterSentencesByUnitName)
	{
		int iStem = RelevanceModelFeatures::FindWordIndex (GetPorterStem (_rUnitName));

		WordToPhrase_map_t::iterator	ite;
		ite = map_WordToPhrase.find (iStem);
		if (map_WordToPhrase.end () == ite)
			return NULL;

		long_set_t& setPhraseIds = ite->second;

		FilteredSentences* pFilteredSentences = new FilteredSentences;
		pFilteredSentences->vec_Sentences.Create (setPhraseIds.size () + 1);
		#ifndef NDEBUG
		pFilteredSentences->vec_Sentences.Memset (0);
		#endif

		size_t i = 0;
		ITERATE (long_set_t, setPhraseIds, iteId)
		{
			Sentence* pSentence = dq_Sentence [*iteId];
			pFilteredSentences->vec_Sentences [i++] = pSentence;
			pFilteredSentences->set_SentenceIndices.insert (pSentence->i_Index);
		}
		pFilteredSentences->vec_Sentences [i++] = p_NullSentence;
		pFilteredSentences->set_SentenceIndices.insert (p_NullSentence->i_Index);

		cout << "Created sentence list for [" << _rUnitName << "] : " 
			 << pFilteredSentences->vec_Sentences.Size () << endl;
		return pFilteredSentences;
	}
	else
	{
		cout << "returning all sentences for [" << _rUnitName << "] : " 
			 << o_AllSentences.vec_Sentences.Size () << endl;
		return &o_AllSentences;
	}
}


//											
FilteredSentences* TextInterface::GetFilteredSentences (String& _rUnitName)
{
	UnitNameToFilteredSentences_map_t::iterator	ite;
	ite = map_UnitNameToFilteredSentences.find (_rUnitName);
	if (map_UnitNameToFilteredSentences.end () != ite)
		return ite->second;

	FilteredSentences* pFilteredSentences;
	#pragma omp critical
	{
		// we need to check again first, just in case	
		// another thread has already created the list.	
		ite = map_UnitNameToFilteredSentences.find (_rUnitName);
		if (map_UnitNameToFilteredSentences.end () != ite)
			pFilteredSentences = ite->second;

		else
		{
			pFilteredSentences = CreateFilteredSentenceList (_rUnitName);
			map_UnitNameToFilteredSentences.insert (make_pair (_rUnitName, pFilteredSentences));
		}
	}
	return pFilteredSentences;
}


//											
void TextInterface::WriteFilteringStats (void)
{
	cout << "Sentence filtering stats:" << endl;

	size_t iTotal = 0;
	ITERATE (UnitNameToFilteredSentences_map_t, map_UnitNameToFilteredSentences, ite)
	{
		FilteredSentences* pFiltered = ite->second;
		size_t iSentences = 0;
		if (NULL != pFiltered)
			iSentences = pFiltered->vec_Sentences.Size ();
		iTotal += iSentences;

		cout << "   " << ite->first << " : " << iSentences << endl;
	}

	double dAverageSentences = iTotal / (double)map_UnitNameToFilteredSentences.size ();
	cout << "\n    average sentences after filtering : " << dAverageSentences << '\n'
		 << "    total sentences                   : " << dq_Sentence.size ()
		 << endl;
}


//											
bool TextInterface::WriteStats (void)
{
	typedef multimap <long, Hit*>	CountToHits_mmp_t;

	CountToHits_mmp_t	mmpCountToHits;
	ITERATE (PhraseToHit_map_t, map_PhraseToHit, ite)
	{
		Hit* pHit = ite->second;
		mmpCountToHits.insert (make_pair (pHit->l_HitCount, pHit));
	}

	File file;
	if (false == file.Open ((config)"text_stat_file", ios_base::out))
		return false;

	int iHitsToRecord = (config)"hits_to_record";
	int iRecorded = 0;
	CountToHits_mmp_t::reverse_iterator	iteHit;
	for (iteHit = mmpCountToHits.rbegin ();
		iteHit != mmpCountToHits.rend ();
		++ iteHit)
	{
		Hit* pHit = iteHit->second;
		file << pHit->l_HitCount << endl;
		file << pHit->l_PhraseIndex << endl;
		file << "---------------------" << endl;
		file << dq_Sentence [pHit->l_PhraseIndex]->s_Sentence << endl;
		file << "---------------------" << endl;
		ITERATE (String_dq_t, (pHit->dq_StateLabels), iteLabel)
			file << *iteLabel << endl;
		file << "---------------------" << endl << endl << endl;

		++ iRecorded;
		if (iRecorded > iHitsToRecord)
			break;
	}
	file.close ();
	return true;
}



