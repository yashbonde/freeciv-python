//                                  
//  AUTHOR      : S.R.K. Branavan	
//  DATE        : 11/09/2006		
//  FILE        : String.cpp		
//                                  

#include <string.h>
#include <regex.h>
#include "nlp_string.h"
using namespace std;


//---------------------------------------------------------------------------------
long String::Count (const char* _zSubString, long _lStart, long _lEnd)
{
    register long lLength = length ();
    if (-1 == _lEnd)
        _lEnd = lLength;
    else if (_lEnd > lLength)
        _lEnd = lLength;
    else if (_lEnd <= 0)
        return 0;
    if (_lStart >= _lEnd)
        return 0;

    register long lSubStringLength = strlen (_zSubString);
    const char* pString = c_str ();
    const char* pStart = pString + _lStart;

    long lCount = 0;
    register const char* p = strstr (pStart, _zSubString);
    while (NULL != p)
    {
        if ((long) (p - pString) + lSubStringLength > lLength)
            return lCount;
        ++ lCount;
        p = strstr (p + lSubStringLength, _zSubString);
    }
    return lCount;
}


//---------------------------------------------------------------------------------
string::size_type String::Find (const char* _zSubString, long _lStart, long _lEnd)
{
    size_type iSubStringIndex = string::find (_zSubString, _lStart);
    if (iSubStringIndex == string::npos)
        return string::npos;
    if (iSubStringIndex > (string::size_type) _lEnd)
        return string::npos;
    
    return iSubStringIndex;
}


//---------------------------------------------------------------------------------
string::size_type String::ReverseFind (const char* _zSubString, long _lStart, long _lEnd)
{
    if (_lEnd < 0)
        _lEnd = length ();
    size_type iSubStringIndex = string::rfind (_zSubString, _lEnd);
    if (iSubStringIndex == string::npos)
        return string::npos;
    if (iSubStringIndex < (string::size_type) _lStart)
        return string::npos;
    
    return iSubStringIndex;
}


//---------------------------------------------------------------------------------
string::size_type String::FindPattern (const char* _zPattern, String& _sMatch, long _lStart, long _lEnd)
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
        return string::npos;
    }


    // ---------------------------------------------------------------------
    char* pSearch = (char*) data ();
	string::size_type iMatch = string::npos;
	regmatch_t oRegMatch;
    if (0 == regexec (&regexp, pSearch, 1, &oRegMatch, 0))
	{
		iMatch = oRegMatch.rm_so;
		_sMatch = this->substr (oRegMatch.rm_so,
								(oRegMatch.rm_eo - oRegMatch.rm_so));
	}

	regfree (&regexp);
	return iMatch;
}


//---------------------------------------------------------------------------------
long String::FindAllPattern (const char* _zPattern, 
							 String_dq_t& _dqMatches,
							 long _lStart,
							 long _lEnd)
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
        return string::npos;
    }


    // ---------------------------------------------------------------------
    char* pSearch = (char*) data ();
	string::size_type iOffset = 0;
	long lMatchCounts = 0;
	regmatch_t oRegMatch;
    while (0 == regexec (&regexp, pSearch, 1, &oRegMatch, 0))
	{
		_dqMatches.push_back (this->substr (oRegMatch.rm_so + iOffset,
											(oRegMatch.rm_eo - oRegMatch.rm_so)));
		iOffset += oRegMatch.rm_eo;
		pSearch += oRegMatch.rm_eo;
		++ lMatchCounts;
	}

	regfree (&regexp);
	return lMatchCounts;
}







