//                                  
//  AUTHOR      : S.R.K. Branavan	
//  DATE        : 11/09/2006		
//  FILE        : String.cpp		
//                                  

#include <iostream>
#include <assert.h>
#include <regex.h>
#include "nlp_string.h"
using namespace std;

stemmer* String::pg_Stemmer = NULL;


//----------------------------------------------------------------------------------------------
void String::Capitalize (void)
{
    cerr << "In libnlp_string.so : String::Capitalize not implemented yet." << endl;
    abort ();
}


//----------------------------------------------------------------------------------------------
void String::LowerCase (void)
{
    char* p = (char*) data ();
    while ('\0' != *p)
    {
        if (('A' <= *p) && ('Z' >= *p))
            *p |= 0x20;
        ++ p;
    }
}


//----------------------------------------------------------------------------------------------
void String::UpperCase (void)
{
    char* p = (char*) data ();
    while ('\0' != *p)
    {
        if (('a' <= *p) && ('z' >= *p))
            *p &= 0xDF;
        ++ p;
    }
}


//----------------------------------------------------------------------------------------------
void String::TitleCase (void)
{
    cerr << "In libnlp_string.so : String::Capitalize not implemented yet." << endl;
    abort ();
}


//----------------------------------------------------------------------------------------------
void String::Replace (const char* _zSearch, const char* _zReplace)
{
    register long lSearchTextLength = strlen (_zSearch);
    register long lReplaceTextLength = strlen (_zReplace);
    register long lReplacementCount = 0;

    if (lSearchTextLength >= lReplaceTextLength)
    {
        // If the replace text is shorter or equal in length to the search text,
        // the length of the string will not increase. We can use this to make  
        // Replace run faster...                                                

        register char* pStart = (char*) data ();
        register char* pCopyPoint = pStart;
        register char* pMatch = strstr (pStart, _zSearch);
        register long lCopyLength = (pMatch - pStart);
        while (NULL != pMatch)
        {
            ++ lReplacementCount;
			if (pCopyPoint != pStart)
				memcpy (pCopyPoint, pStart, lCopyLength); 
            pCopyPoint += lCopyLength;
            memcpy (pCopyPoint, _zReplace, lReplaceTextLength);
            pCopyPoint += lReplaceTextLength;

            pStart = pMatch + lSearchTextLength;
            pMatch = strstr (pStart, _zSearch);
            lCopyLength = (pMatch - pStart);
        }
        lCopyLength = strlen (pStart);
        if (lCopyLength > 0)
		{
			if (pCopyPoint != pStart)
				strncpy (pCopyPoint, pStart, lCopyLength);
		}
        erase ((pCopyPoint - data ()) + lCopyLength);
    }
    else
    {
        // If the replace text is longer than the search text, the resulting text   
        // will become longer, complicating the replacement operation a little      
        // In this case, we first get a list of all the replacements that need to   
        // be done, and then do the replacements in reverse after lengthing the     
        // string first...                                                          
        list <long> lstMatchStart;
        register char* pStart = (char*) data ();
        register char* pMatch = strstr (pStart, _zSearch);
        while (NULL != pMatch)
        {
            lstMatchStart.push_front (pMatch - pStart);
            pMatch = strstr (pMatch + lSearchTextLength, _zSearch);
        }
        lReplacementCount = lstMatchStart.size ();
        register long lLengthIncrement = (lReplaceTextLength - lSearchTextLength);
        register long lOffset = lReplacementCount * lLengthIncrement;
		long lOriginalLength = length ();
        resize (lOriginalLength + lOffset);

        pStart = (char*) data ();
        register char* pMoveEnd = pStart + lOriginalLength;
        list <long>::iterator  ite;
        for (ite = lstMatchStart.begin ();
            ite != lstMatchStart.end ();
            ++ ite)
        {
            register char* pMatch = pStart + *ite;
            register char* pMoveStart = pMatch + lSearchTextLength;
            memmove (pMoveStart + lOffset, pMoveStart, (pMoveEnd - pMoveStart));
            lOffset -= lLengthIncrement;
            memcpy (pMatch + lOffset, _zReplace, lReplaceTextLength);
            pMoveEnd = pMatch;
        }
        lstMatchStart.clear ();
    }
}


//----------------------------------------------------------------------------------------------
void String::ReplacePattern (const char* _zPattern, const char* _zReplace)
{
    regex_t regexp;
    int ret = regcomp (&regexp, _zPattern, REG_EXTENDED);
    if (0 != ret)
    {
        char zError [1000];
        regerror (ret, &regexp, zError, 1000);
        cerr << "Regular expression '" << _zPattern
             << "' failed to compile. Error : "
             << zError << endl;
        return;
    }

    int iPatternGroups = 0;
    const char* p = _zPattern;
    while ('\0' != *p)
    {
        if ('(' == *p)
            ++ iPatternGroups;
        ++ p;
    }
    ++ iPatternGroups;
    
    //----------------------------------------------------------------------
    class ReplaceInfo
    {
        public:
            char* pz_Text;
            long  l_Length;
            int   i_Group;
    };

    ReplaceInfo* pReplaceInfo = new ReplaceInfo [2 * iPatternGroups];
	memset (pReplaceInfo, 0, sizeof (ReplaceInfo) * 2 * iPatternGroups);
    int iReplaceGroupCount = 0;
    long lTextReplaceLength = 0;
    char cPatternInUse [2 * iPatternGroups];
    memset (cPatternInUse, 0, sizeof (char) * 2 * iPatternGroups);
    //----------------------------------------------------------------------
    {
        ReplaceInfo* pInfo = pReplaceInfo;

        const char* pStart = _zReplace;
        const char* pCurrent = pStart;
        char cPrevious = ' ';
        while ('\0' != *pCurrent)
        {
            if (('\\' == *pCurrent) && ('\\' == cPrevious))
            {
                cPrevious = ' ';
                continue;
            }

            if (('\\' == cPrevious) && ('0' <= *pCurrent) && ('9' >= *pCurrent))
            {
                long lLength = pCurrent - pStart - 1;
                if (lLength > 0)
                {
                    char* pzText = new char [lLength + 1];
                    strncpy (pzText, pStart, lLength);
                    pzText [lLength] = '\0';

                    pInfo->pz_Text = pzText;
                    pInfo->l_Length = lLength;
                    lTextReplaceLength += lLength;
                    ++ pInfo;
                    ++ iReplaceGroupCount;
                }
                
                pInfo->pz_Text = NULL;
                pInfo->i_Group = atoi (pCurrent);
                cPatternInUse [pInfo->i_Group] = 1;
                ++ pInfo;
                ++ iReplaceGroupCount;

                while (('0' <= *pCurrent) && ('9' >= *pCurrent))
                {
                    if ('\0' == *pCurrent)
                        break;
                    ++ pCurrent;
                }
                pStart = pCurrent;
            }
            cPrevious = *pCurrent;
            ++ pCurrent;
        }

        long lLength = strlen (pStart);
		if (lLength > 0)
		{
			char* pzText = new char [lLength + 1];
			strncpy (pzText, pStart, lLength);
			pzText [lLength] = '\0';
			pInfo->pz_Text = pzText;
			pInfo->l_Length = lLength;
			lTextReplaceLength += lLength;
			++ iReplaceGroupCount;
		}
    }    

    // ---------------------------------------------------------------------
    list <Match*>   lstMatches;
    long lShift = 0;
    char* pStart = (char*) data ();
    char* pSearch = pStart;
    regmatch_t* pRegMatch = new regmatch_t [iPatternGroups + 2];
    while (0 == regexec (&regexp, pSearch, iPatternGroups + 2, pRegMatch, 0))
    {
        long lOffset = pSearch - pStart;
        long lGroupTextLength = 0;
        regmatch_t* m = pRegMatch + 1;
        int iGroupIndex = 1;
        while (-1 != m->rm_so)
        {
            if (1 == cPatternInUse [iGroupIndex])
                lGroupTextLength += m->rm_eo - m->rm_so;
            ++ m;
            ++ iGroupIndex;
        }
        lShift += lTextReplaceLength + lGroupTextLength - (pRegMatch->rm_eo - pRegMatch->rm_so);


        Match* pMatch = new Match;
        lstMatches.push_front (pMatch);
        pMatch->l_StartOffset = pRegMatch->rm_so + lOffset;
        pMatch->l_EndOffset = pRegMatch->rm_eo + lOffset;
        pMatch->p_SubMatches = new SubMatch [iReplaceGroupCount];

        ReplaceInfo* pInfo = pReplaceInfo;
        SubMatch* pSubMatch = pMatch->p_SubMatches;

        for (int i = 0; i < iReplaceGroupCount; ++ i)
        {
            pSubMatch->p_ReplaceText = pInfo->pz_Text;
            if (NULL != pInfo->pz_Text)
            {
                regmatch_t& regmatch = pRegMatch [i + 1];
                pSubMatch->l_SourceOffset = regmatch.rm_so + lOffset;
                pSubMatch->l_Length = pInfo->l_Length;
            }
            else
            {
                regmatch_t& regmatch = pRegMatch [pInfo->i_Group];
                pSubMatch->l_SourceOffset = regmatch.rm_so + lOffset;
                pSubMatch->l_Length = regmatch.rm_eo - regmatch.rm_so;
            }
            ++ pSubMatch;
            ++ pInfo;
        }
        pMatch->l_Shift = lShift;
        pSearch += pRegMatch->rm_eo;
    }
	delete[] pRegMatch;

	//----------------------------------------------------------------------
	// If the string is becoming longer with the replacements, we do the  	
	// replacements in reverse from the end of the string...				
	//----------------------------------------------------------------------
	long lOriginalLength = length ();
	
	if (lShift >= 0)
	{
		resize (length () + lShift);

		long lMoveEnd = lOriginalLength;
		pStart = (char*) data ();
		list <Match*>::iterator  ite;
		for (ite = lstMatches.begin ();
			ite != lstMatches.end ();
			++ ite)
		{
			Match* pMatch = *ite;
			char* pMoveStart = pStart + pMatch->l_EndOffset;
			if (pMatch->l_Shift != 0)
				memmove (pMoveStart + pMatch->l_Shift, pMoveStart, lMoveEnd - pMatch->l_EndOffset);

			lMoveEnd = pMatch->l_EndOffset + pMatch->l_Shift;
			for (int i = iReplaceGroupCount - 1; i >= 0; -- i)
			{
				SubMatch& rSubMatch = pMatch->p_SubMatches [i];
				if (NULL != rSubMatch.p_ReplaceText)
				{
					memcpy (pStart + lMoveEnd - rSubMatch.l_Length, 
							rSubMatch.p_ReplaceText, 
							rSubMatch.l_Length);
					lMoveEnd -= rSubMatch.l_Length;
				}
				else
				{
					memmove (pStart + lMoveEnd - rSubMatch.l_Length, 
							 pStart + rSubMatch.l_SourceOffset, 
							 rSubMatch.l_Length);
					lMoveEnd -= rSubMatch.l_Length;
				}
			}

			lMoveEnd = pMatch->l_StartOffset;
		}
	}

	//----------------------------------------------------------------------
	// If the string is becoming sorter with the replacements, we do the  	
	// replacements in the normal direction - from the begining of the 		
	// string...															
	//----------------------------------------------------------------------
	if (lShift < 0)
	{
		lstMatches.reverse ();

		long lMoveEnd = 0;
		long lLastShift = 0;
		pStart = (char*) data ();
		char* pMoveStart = pStart;
		list <Match*>::iterator  ite;
		for (ite = lstMatches.begin ();
			ite != lstMatches.end ();
			++ ite)
		{
			Match* pMatch = *ite;

			if (lLastShift != 0)
				memmove (pMoveStart + lLastShift, pMoveStart, pMatch->l_StartOffset - lMoveEnd);

			lLastShift = pMatch->l_Shift;

			pMoveStart = pStart + pMatch->l_EndOffset;
			lMoveEnd = pMatch->l_EndOffset + pMatch->l_Shift;
			for (int i = iReplaceGroupCount - 1; i >= 0; -- i)
			{
				SubMatch& rSubMatch = pMatch->p_SubMatches [i];
				if (NULL != rSubMatch.p_ReplaceText)
				{
					memcpy (pStart + lMoveEnd - rSubMatch.l_Length,
							rSubMatch.p_ReplaceText, 
							rSubMatch.l_Length);
					lMoveEnd += rSubMatch.l_Length;
				}
				else
				{
					memmove (pStart + lMoveEnd - rSubMatch.l_Length, 
							 pStart + rSubMatch.l_SourceOffset, 
							 rSubMatch.l_Length);
					lMoveEnd += rSubMatch.l_Length;
				}
			}
			lMoveEnd = pMatch->l_EndOffset;
		}
		if (lMoveEnd < lOriginalLength)
			memmove (pMoveStart + lLastShift, pMoveStart, lOriginalLength - lMoveEnd);
	}

	if (lShift < 0)
		resize (length () + lShift);

	for (int i = 0; i < 2 * iPatternGroups; ++ i)
	{
		ReplaceInfo* pInfo = &(pReplaceInfo [i]);
		if (NULL != pInfo->pz_Text)
			delete[] pInfo->pz_Text;
	}

	delete[] pReplaceInfo;
    list <Match*>::iterator	ite;
	for (ite = lstMatches.begin ();
		ite != lstMatches.end ();
		++ ite)
	{
		Match* pMatch = *ite;
		delete[] pMatch->p_SubMatches;
		delete pMatch;
	}
	lstMatches.clear ();

	regfree (&regexp);
}


//----------------------------------------------------------------------------------------------
void String::Join (const list <String>& _rList, char _cSeparator)
{
    list <String>::iterator ite = ((list <String>&)_rList).begin ();
    list <String>::iterator ite_end = ((list <String>&)_rList).end ();
    if (ite == ite_end)
        return;

    append ((string&) *ite);
    ++ ite;
    for (; ite != ite_end; ++ ite)
    {
        append (1, _cSeparator);
        append ((string&) *ite);
    }
}


//----------------------------------------------------------------------------------------------
void String::Join (const list <string>& _rList, char _cSeparator)
{
    list <string>::iterator ite = ((list <string>&)_rList).begin ();
    list <string>::iterator ite_end = ((list <string>&)_rList).end ();
    if (ite == ite_end)
        return;

    append (*ite);
	++ ite;
    for (; ite != ite_end; ++ ite)
    {
        append (1, _cSeparator);
        append (*ite);
    }
}


//----------------------------------------------------------------------------------------------
void String::Split (list <String>& _rList, char _cSeparator, int _iTotalSplits)
{
	_rList.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...

		// -----------------------------------------------------------------------------------------
		// In the statement below, checking (_cSeparator == *pStart) is possibly problematic.		
		// It eats up contiguous separater characters even when we're not splitting by whitespace.	
        // while ((('\0' == _cSeparator) && ((' ' == *pStart) || ('\t' == *pStart) || ('\n' == *pStart) || ('\r' == *pStart)))
        //       || (_cSeparator == *pStart))
		// -----------------------------------------------------------------------------------------

        while (('\0' == _cSeparator) && ((' ' == *pStart) || ('\t' == *pStart) || ('\n' == *pStart) || ('\r' == *pStart)))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if ((('\0' == _cSeparator) && ((' ' == *pEnd) || ('\t' == *pEnd) || ('\n' == *pEnd) || ('\r' == *pEnd)))
                || (_cSeparator == *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
			if (('\0' != _cSeparator) || (pStart < pEnd))
				_rList.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rList.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rList.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitByString (list <String>& _rList, const char* _zSeparator, int _iTotalSplits)
{
	_rList.clear ();

    int iSplitCount = 0;
    register long lSeparatorLength = strlen (_zSeparator);
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        pEnd = strstr (pStart, _zSeparator);
        if (NULL == pEnd)
        {
            _rList.push_back (pStart);
            return;
        }

        long lLength = (long)(pEnd - pStart);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rList.push_back (pzText);
        delete[] pzText;
        
        pStart = pEnd + lSeparatorLength;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rList.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitByRegexp (list <String>& _rList, const char* _zPattern, int _iTotalSplits)
{
	_rList.clear ();

    regex_t regexp;
    int ret = regcomp (&regexp, _zPattern, REG_EXTENDED);
    if (0 != ret)
    {
        char zError [1000];
        regerror (ret, &regexp, zError, 1000);
        cerr << "Regular expression '" << _zPattern
             << "' failed to compile. Error : "
             << zError << endl;
        return;
    }

    int iSplitCount = 0;
    char* pStart = (char*) data ();
    char* pTextEnd = pStart + length ();

    regmatch_t match;
    while (0 == regexec (&regexp, pStart, 1, &match, 0))
    {
        char* pzText = new char [match.rm_so + 1];
        strncpy (pzText, pStart, match.rm_so);
        pzText [match.rm_so] = '\0';
        _rList.push_back (pzText);
        delete[] pzText;

        pStart = pStart + match.rm_eo;
        if (pStart > pTextEnd)
            break;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
            break;
    }

    _rList.push_back (pStart);
	regfree (&regexp);
}


//----------------------------------------------------------------------------------------------
void String::SplitBySeparatorList (list <String>& _rList, const char* _zSeparatorList, int _iTotalSplits)
{
	_rList.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while (NULL != strchr (_zSeparatorList, *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if (NULL != strchr (_zSeparatorList, *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            _rList.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart + 1);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rList.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rList.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitBySeparatorListSkippingQuoted (list <String>& _rList, const char* _zSeparatorList, int _iTotalSplits)
{
	_rList.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while (NULL != strchr (_zSeparatorList, *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
		bool bInQuotes = false;
		char cQuote = ' ';
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
			if (true == bInQuotes)
			{
				if (cQuote == *pEnd)
				{
					bInQuotes = false;
					cQuote = ' ';
				}
				++ pEnd;
				continue;
			}
			if (('"' == *pEnd) || ('\'' == *pEnd))
			{
				cQuote = *pEnd;
				bInQuotes = true;
				++ pEnd;
				continue;
			}

            if (NULL != strchr (_zSeparatorList, *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            _rList.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart + 1);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rList.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rList.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitLines (list <String>& _rList, char _cSeparator, int _iTotalSplits)
{
	_rList.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while ((('\0' == _cSeparator) && (('\n' == *pStart) || ('\r' == *pStart)))
               || (_cSeparator == *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if ((('\0' == _cSeparator) && (('\n' == *pEnd) || ('\r' == *pEnd)))
                || (_cSeparator == *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            if (strlen (pStart) > 0)
                _rList.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart);
        if (lLength > 0)
        {
            char* pzText = new char [lLength + 1];
            strncpy (pzText, pStart, lLength);
            pzText [lLength] = '\0';
            _rList.push_back (pzText);
            delete[] pzText;
        }

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rList.push_back (pStart);
            return;
        }
    }
}


//-- static method -----------------------------------------------------------------------------
zchar_dq_t String::DestructiveSplit (char* _pzString, char _cTerminator, int _iTotalSplits)
{
	zchar_dq_t dq;

	int iSplits = 0;
	char* pValue = _pzString;
	while ((NULL != pValue) && ('\0' != *pValue))
	{
		dq.push_back (pValue);
		++ iSplits;
		if ((0 != _iTotalSplits) && (iSplits > _iTotalSplits))
			break;

		pValue = strchr (pValue, _cTerminator);
		if (NULL != pValue)
		{
			*pValue = '\0';
			++ pValue;
		}
	}

	return dq;
}


//----------------------------------------------------------------------------------------------
void String::Join (const deque <String>& _rDq, char _cSeparator)
{
    deque <String>::iterator ite = ((deque <String>&)_rDq).begin ();
    deque <String>::iterator ite_end = ((deque <String>&)_rDq).end ();
    if (ite == ite_end)
        return;

    append ((string&) *ite);
    ++ ite;
    for (; ite != ite_end; ++ ite)
    {
        append (1, _cSeparator);
        append ((string&) *ite);
    }
}


//----------------------------------------------------------------------------------------------
void String::Join (const deque <string>& _rDq, char _cSeparator)
{
    deque <string>::iterator ite = ((deque <string>&)_rDq).begin ();
    deque <string>::iterator ite_end = ((deque <string>&)_rDq).end ();
    if (ite == ite_end)
        return;

    append (*ite);
	++ ite;
    for (; ite != ite_end; ++ ite)
    {
        append (1, _cSeparator);
        append (*ite);
    }
}


//----------------------------------------------------------------------------------------------
void String::Join (const set <String>& _rSet, char _cSeparator)
{
    set <String>::iterator ite = ((set <String>&)_rSet).begin ();
    set <String>::iterator ite_end = ((set <String>&)_rSet).end ();
    if (ite == ite_end)
        return;

    append (*ite);
	++ ite;
    for (; ite != ite_end; ++ ite)
    {
        append (1, _cSeparator);
        append (*ite);
    }
}


//----------------------------------------------------------------------------------------------
void String::Split (deque <String>& _rDq, char _cSeparator, int _iTotalSplits)
{
	_rDq.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
	
		// -----------------------------------------------------------------------------------------
		// In the statement below, checking (_cSeparator == *pStart) is possibly problematic.		
		// It eats up contiguous separater characters even when we're not splitting by whitespace.	
        // while ((('\0' == _cSeparator) && ((' ' == *pStart) || ('\t' == *pStart) || ('\n' == *pStart) || ('\r' == *pStart)))
        //        || (_cSeparator == *pStart))
		// -----------------------------------------------------------------------------------------

        while (('\0' == _cSeparator) && ((' ' == *pStart) || ('\t' == *pStart) || ('\n' == *pStart) || ('\r' == *pStart)))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if ((('\0' == _cSeparator) && ((' ' == *pEnd) || ('\t' == *pEnd) || ('\n' == *pEnd) || ('\r' == *pEnd)))
                || (_cSeparator == *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
			if (('\0' != _cSeparator) || (pStart < pEnd))
				_rDq.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rDq.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rDq.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitByString (deque <String>& _rDq, const char* _zSeparator, int _iTotalSplits)
{
	_rDq.clear ();

    int iSplitCount = 0;
    register long lSeparatorLength = strlen (_zSeparator);
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        pEnd = strstr (pStart, _zSeparator);
        if (NULL == pEnd)
        {
            _rDq.push_back (pStart);
            return;
        }

        long lLength = (long)(pEnd - pStart);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rDq.push_back (pzText);
        delete[] pzText;
        
        pStart = pEnd + lSeparatorLength;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rDq.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitByRegexp (deque <String>& _rDq, const char* _zPattern, int _iTotalSplits)
{
	_rDq.clear ();

    regex_t regexp;
    int ret = regcomp (&regexp, _zPattern, REG_EXTENDED);
    if (0 != ret)
    {
        char zError [1000];
        regerror (ret, &regexp, zError, 1000);
        cerr << "Regular expression '" << _zPattern
             << "' failed to compile. Error : "
             << zError << endl;
        return;
    }

    int iSplitCount = 0;
    char* pStart = (char*) data ();
    char* pTextEnd = pStart + length ();

    regmatch_t match;
    while (0 == regexec (&regexp, pStart, 1, &match, 0))
    {
        char* pzText = new char [match.rm_so + 1];
        strncpy (pzText, pStart, match.rm_so);
        pzText [match.rm_so] = '\0';
        _rDq.push_back (pzText);
        delete[] pzText;

        pStart = pStart + match.rm_eo;
        if (pStart > pTextEnd)
            break;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
            break;
    }

    _rDq.push_back (pStart);
	regfree (&regexp);
}


//----------------------------------------------------------------------------------------------
void String::SplitBySeparatorList (deque <String>& _rDq, const char* _zSeparatorList, int _iTotalSplits)
{
	_rDq.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while (NULL != strchr (_zSeparatorList, *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if (NULL != strchr (_zSeparatorList, *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            _rDq.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart + 1);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rDq.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rDq.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitBySeparatorListSkippingQuoted (deque <String>& _rDq, const char* _zSeparatorList, int _iTotalSplits)
{
	_rDq.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while (NULL != strchr (_zSeparatorList, *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
		bool bInQuotes = false;
		char cQuote = ' ';
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
			if (true == bInQuotes)
			{
				if (cQuote == *pEnd)
				{
					bInQuotes = false;
					cQuote = ' ';
				}
				++ pEnd;
				continue;
			}
			if (('"' == *pEnd) || ('\'' == *pEnd))
			{
				cQuote = *pEnd;
				bInQuotes = true;
				++ pEnd;
				continue;
			}

            if (NULL != strchr (_zSeparatorList, *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            _rDq.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart + 1);
        char* pzText = new char [lLength + 1];
        strncpy (pzText, pStart, lLength);
        pzText [lLength] = '\0';
        _rDq.push_back (pzText);
        delete[] pzText;

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rDq.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::SplitLines (deque <String>& _rDq, char _cSeparator, int _iTotalSplits)
{
	_rDq.clear ();

    int iSplitCount = 0;
    register char* pStart = (char*) data ();
    register char* pEnd;
    while (true)
    {
        // eat up any initial separator characters...
        while ((('\0' == _cSeparator) && (('\n' == *pStart) || ('\r' == *pStart)))
               || (_cSeparator == *pStart))
        {
            if ('\0' == *pStart)
                return;
            ++ pStart;
        }

        // find next word break...
        bool bLastWord = true;
        pEnd = pStart;
        while ('\0' != *pEnd)
        {
            if ((('\0' == _cSeparator) && (('\n' == *pEnd) || ('\r' == *pEnd)))
                || (_cSeparator == *pEnd))
            {
                bLastWord = false;
                break;
            }
            ++ pEnd;
        }

        if (true == bLastWord)
        {
            if (strlen (pStart) > 0)
                _rDq.push_back (pStart);
            return;
        }

        long lLength = (pEnd - pStart);
        if (lLength > 0)
        {
            char* pzText = new char [lLength + 1];
            strncpy (pzText, pStart, lLength);
            pzText [lLength] = '\0';
            _rDq.push_back (pzText);
            delete[] pzText;
        }

        pStart = pEnd + 1;

        ++ iSplitCount;
        if ((_iTotalSplits > 0) && (iSplitCount >= _iTotalSplits))
        {
            _rDq.push_back (pStart);
            return;
        }
    }
}


//----------------------------------------------------------------------------------------------
void String::Strip (void)
{
	int iLength = size ();
	int iStart = 0;
	while (iStart < iLength)
	{
		char ch = (*(string*)this) [iStart];
		if ((' ' != ch) && ('\t' != ch) && ('\r' != ch) && ('\n' != ch))
			break;
		++ iStart;
	}

	int iEnd = iLength - 1;
	while (iEnd >= 0)
	{
		char ch = (*(string*)this) [iEnd];
		if ((' ' != ch) && ('\t' != ch) && ('\r' != ch) && ('\n' != ch))
			break;
		-- iEnd;
	}

	erase (iEnd + 1);
	erase (0, iStart);
}




//----------------------------------------------------------------------------------------------
void String::PennTokenize (void)
{
	// attempt to get correct directional quotes
	ReplacePattern ("^\"", "`` ");
	ReplacePattern ("([ ([{<])\"", "\\1 `` ");
	// close quotes is handled at end

    Replace ("...", " ... ");
    ReplacePattern ("([,;:@#$%&])", " \\1 ");

    // Assume sentence tokenization has been done first, so split FINAL periods only.
    ReplacePattern ("([^.])([.])([]\\)}>\"\']*)[ \t]*$", "\\1 \\2\\3 ");

    // however, we may as well split ALL question marks and exclamation points,
    // since they shouldn't have the abbrev.-marker ambiguity problem
    Replace ("?", " ? ");
    Replace ("!", " ! ");

    // parentheses, brackets, etc.
    ReplacePattern (" ([\\[\\]()\\{\\}<>])", " \\1 ");
    Replace ("]", " ] ");
    Replace ("[", " [ ");
    Replace (")", " ) ");
    Replace ("(", " ( ");
    Replace ("}", " } ");
    Replace ("{", " { ");
    Replace (">", " > ");
    Replace ("<", " < ");

    Replace ("--", " -- ");
    Replace ("-", " ");

    // First off, add a space to the beginning and end of each line, to reduce
    // necessary number of regexps.
    String s (" ");
    s << *this << ' ';
    *this = s;
    Replace ("''", " '' ");
    Replace ("``", " `` ");
    Replace ("\"", " '' ");

    // possessive or close-single-quote
    ReplacePattern ("([^'])' ", "\\1 ' ");

    // as in it's, I'm, we'd
    ReplacePattern ("'([sSmMdD])", " '\\1 ");
    Replace ("'ll ", " 'll ");
    Replace ("'LL ", " 'LL ");
    Replace ("'re ", " 're ");
    Replace ("'RE ", " 'RE ");
    Replace ("'ve ", " 've ");
    Replace ("'VE ", " 'VE ");
    Replace ("n't ", " n't ");
    Replace ("N'T ", " N'T ");

    ReplacePattern ("([Cc])annot ", " \\1an not ");
    ReplacePattern ("([Dd])'ye ", " \\1' ye ");
    ReplacePattern ("([Gg])imme ", " \\1im me ");
    ReplacePattern ("([Gg])onna ", " \\1on na ");
    ReplacePattern ("([Gg])otta ", " \\1ot ta ");
    ReplacePattern ("([Ll])emme ", " \\1em me ");
    ReplacePattern ("([Mm])ore'n ", " \\1ore 'n ");
    ReplacePattern (" '([Tt])is ", " '\\1 is ");
    Replace (" '([Tt])was ", " '\\1 was ");
    ReplacePattern ("([Ww])anna ", " \\1an na ");

    ReplacePattern ("[ \t]+$", "");
    ReplacePattern ("^[ \t]+", "");
    ReplacePattern ("[ \t]+", " ");
}


//----------------------------------------------------------------------------------------------
void String::PennTokenizeForMxpost (void)
{
	PennTokenize ();

    // Some taggers, such as Adwait Ratnaparkhi's MXPOST, use the parsed-file version of these symbols.
    // UNCOMMENT THE FOLLOWING 6 LINES if you're using MXPOST.
    Replace ("(", "-LRB-");
    Replace (")", "-RRB-");
    Replace ("[", "-LSB-");
    Replace ("]", "-RSB-");
    Replace ("{", "-LCB-");
    Replace ("}", "-RCB-");

	int iLength = length ();
	if (iLength > 0)
	{
		char chEnd = (*this).at (iLength - 1);
		if (NULL == strchr (".!?;:", chEnd))
			(*this) << " .";
	}
}



//----------------------------------------------------------------------------------------------
long String::LevensteinDistance (const char* _zLeft, const char* _zRight)
{
	// d is a table with m+1 rows and n+1 columns
	long lLengthLeft = strlen (_zLeft) + 1;
	long lLengthRight = strlen (_zRight) + 1;

	long* d = new long [lLengthLeft * lLengthRight];
	for (long l = 0; l < lLengthLeft; ++ l)
		d [l * lLengthRight] = l;
	for (long r = 0; r < lLengthRight; ++ r)
		d [r] = r;

	for (long l = 1; l < lLengthLeft; ++ l)
	{
		for (long r = 1; r < lLengthRight; ++ r)
		{
			long a = d [(l-1) * lLengthRight +  r ] + 1;
			long b = d [  l   * lLengthRight + r-1] + 1;
			long c = d [(l-1) * lLengthRight + r-1] + ((_zLeft [l-1] == _zRight [r-1])? 0 : 1);

			if (a > b)
				a = b;
			if (a > c)
				a = c;
			d [l * lLengthRight + r] = a;
		}
	}

	long lDistance = d [lLengthLeft * lLengthRight - 1];
	delete[] d;

	return lDistance;
}


//----------------------------------------------------------------------------------------------
void String::InitPorterStemmer (void)
{
	pg_Stemmer = PorterStemmer::CreateStemmer ();
}


//----------------------------------------------------------------------------------------------
void String::DestroyPorterStemmer (void)
{
	if (NULL != pg_Stemmer)
		PorterStemmer::FreeStemmer (pg_Stemmer);
	pg_Stemmer = NULL;
}


//----------------------------------------------------------------------------------------------
String String::GetPorterStem (void)
{
	String sWord;
	if (NULL == pg_Stemmer)
	{
		cout << "[ERROR]  PorterStemmer not initialized on String::GetPorterStem () call.\n"
				"         String::InitPorterStemmer () needs to be called first, and\n"
				"         String::DestroyPorterStemmer () has to be called once you're done."
			 << endl;
		return sWord;
	}

	size_t iLength = length ();
	char* pzWord = new char [iLength + 10];
	strncpy (pzWord, (const char*)*this, iLength);
	pzWord [iLength] = '\0';

	size_t ret = PorterStemmer::Stem (pg_Stemmer, pzWord, iLength);
	pzWord [(ret < iLength)? ret + 1 : iLength] = '\0';
	sWord = pzWord;
	delete[] pzWord;

	return sWord;
}


//----------------------------------------------------------------------------------------------
void String::ConvertToPorterStem (char* _zWord)
{
	if (NULL == pg_Stemmer)
	{
		cout << "[ERROR]  PorterStemmer not initialized on String::GetPorterStem () call.\n"
				"         String::InitPorterStemmer () needs to be called first, and\n"
				"         String::DestroyPorterStemmer () has to be called once you're done."
			 << endl;
		return;
	}

	size_t iLength = strlen (_zWord);
	int ret = PorterStemmer::Stem (pg_Stemmer, _zWord, iLength);
	_zWord [ret + 1] = '\0';
}
