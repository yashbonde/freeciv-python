#ifndef __SB_LIBRARY_TYPES_STRING__
#define __SB_LIBRARY_TYPES_STRING__ 

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <iostream>
#include <string> 
#include <list>
#include <deque>
#include <set>
#include "PorterStemmer.h"


class String;
typedef std::deque <String>		String_dq_t;
typedef std::list <String>		String_lst_t;
typedef std::set <String>		String_set_t;
typedef std::deque <char*>		zchar_dq_t;



// ---------------------------------------------------------------------
class SubMatch
{
    public:
        long l_SourceOffset;
        long l_TargetOffset;
        long l_Length;
        char* p_ReplaceText;
};

class Match
{
    public:
        long l_StartOffset;
        long l_EndOffset;
        long l_Shift;
        SubMatch* p_SubMatches;
};



// ---------------------------------------------------------------------
class String : public std::string
{
private:
	static stemmer*	pg_Stemmer;

public:
	String (void);
	String (const char* _zText);
	String (const std::string& _sText);
	String (const String& _sText);
	String (const String_dq_t& _rDq);
	String (const std::deque <std::string>& _rDq);
	String (const String_lst_t& _rList);
	String (const std::list <std::string>& _rList);

	void Set (const char* _zText, long _lBytes);

	int Compare (const String& _rString);
	int Compare (const char* _zValue);
	int Compare (const char _cValue);
	
	int CaseCompare (const String& _rString);// compare string ignoring case.
	int CaseCompare (const char* _zValue);   // compare string ignoring case.
	int CaseCompare (const char _cValue);    // compare string ignoring case.
	
    // ---------------------------------------------------
    void Capitalize (void);
    void LowerCase (void);
    void UpperCase (void);
    void TitleCase (void);

    long Count (const char* _zSubString, long _lStart = 0, long _lEnd = -1);
    size_type Find (const char* _zSubString, long _lStart = 0, long _lEnd = -1);
    size_type ReverseFind (const char* _zSubString, long _lStart = 0, long _lEnd = -1);
    size_type FindPattern (const char* _zPattern, String& _sMatch, long _lStart = 0, long _lEnd = -1);
    long FindAllPattern (const char* _zPattern, String_dq_t& _dqMatchs, long _lStart = 0, long _lEnd = -1);
    void Replace (const char* _zSearch, const char* _zReplace);
    void ReplacePattern (const char* _zPattern, const char* _zReplace);

	inline bool Has (const std::string _sSubString, long _lStart = 0, long _lEnd = -1)
		{ return Has (_sSubString.c_str (), _lStart, _lEnd); }

	inline bool Has (const char* _zSubString, long _lStart = 0, long _lEnd = -1)
		{ return (std::string::npos != String::Find (_zSubString, _lStart, _lEnd)); }

	inline bool StartsWith (const std::string _sSubString, long _lStart = 0)
		{ return Has (_sSubString.c_str (), _lStart); }

	inline bool StartsWith (const char* _zSubString, long _lStart = 0)
		{
			size_type iPos = String::Find (_zSubString, _lStart, -1);
			if (std::string::npos == iPos)
				return false;
			return (_lStart == (long) iPos);
		}

	inline bool EndsWith (const std::string _sSubString, long _lEnd = -1)
		{ return EndsWith (_sSubString.c_str (), _lEnd); }

	inline bool EndsWith (const char* _zSubString, long _lEnd = -1)
		{ 
			size_type iPos = String::Find (_zSubString, 0, _lEnd);
			if (std::string::npos == iPos)
				return false;
			return ((length () - strlen (_zSubString)) == iPos);
		}

	inline bool HasPattern (const char* _zPattern, long _lStart = 0, long _lEnd = -1)
	{	
		String sUnused;
		return (std::string::npos != String::FindPattern (_zPattern, sUnused, _lStart, _lEnd)); 
	}


	// String Cut (int _iSegment, char _cSeparator = '\0');
    void Join (const String_set_t& _rSet, char _cSeparator = ' ');

    void Join (const String_dq_t& _rDq, char _cSeparator = ' ');
    void Join (const std::deque <std::string>& _rList, char _cSeparator = ' ');
    void Split (String_dq_t& _rDq, char _cSeparator = '\0', int _iTotalSplits = 0);
    void SplitByString (String_dq_t& _rDq, const char* _zSeparator, int _iTotalSplits = 0);
    void SplitByRegexp (String_dq_t& _rDq, const char* _zPattern, int _iTotalSplits = 0);
    void SplitBySeparatorList (String_dq_t& _rDq, const char* _zSeparatorList, int _iTotalSplits = 0);
	void SplitBySeparatorListSkippingQuoted (String_dq_t& _rDq, const char* _zSeparatorList, int _iTotalSplits = 0);
    void SplitLines (String_dq_t& _rDq, char _cSeparator = '\0', int _iTotalSplits = 0);

    void Join (const String_lst_t& _rList, char _cSeparator = ' ');
    void Join (const std::list <std::string>& _rList, char _cSeparator = ' ');
    void Split (String_lst_t& _rList, char _cSeparator = '\0', int _iTotalSplits = 0);
    void SplitByString (String_lst_t& _rList, const char* _zSeparator, int _iTotalSplits = 0);
    void SplitByRegexp (String_lst_t& _rList, const char* _zPattern, int _iTotalSplits = 0);
    void SplitBySeparatorList (String_lst_t& _rList, const char* _zSeparatorList, int _iTotalSplits = 0);
	void SplitBySeparatorListSkippingQuoted (String_lst_t& _rList, const char* _zSeparatorList, int _iTotalSplits = 0);
    void SplitLines (String_lst_t& _rList, char _cSeparator = '\0', int _iTotalSplits = 0);

	static zchar_dq_t DestructiveSplit (char* _zString, char _cTerminator, int _iTotalSplits = 0);
	zchar_dq_t DestructiveSplit (char _cTerminator, int _iTotalSplits = 0)
		{ return DestructiveSplit ((char*)(const char*)*this, _cTerminator, _iTotalSplits); }

	void Strip (void);
	void PennTokenize (void);
	void PennTokenizeForMxpost (void);

    bool IsWord (void);
    bool IsAlNum (void);
    bool IsAlpha (void);
    bool IsDigit (void);
    bool IsSpace (void);
    bool IsLower (void);
    bool IsUpper (void);
    bool IsTitle (void);

    bool HasAlNum (void);
    bool HasAlpha (void);
    bool HasDigit (void);
    bool HasSpace (void);

	static long LevensteinDistance (const char* _zLeft, const char* _zRight);
	static void InitPorterStemmer (void);
	static void DestroyPorterStemmer (void);
	static void ConvertToPorterStem (char* _zWord);
	String GetPorterStem (void);

    // operators -----------------------------------------
	operator bool (void) const;
	operator int (void) const;
	operator long (void) const;
	operator double (void) const;
	operator const char* (void) const;
	operator std::string (void) const;

	bool operator== (const bool _bValue) const;
	bool operator== (const char _cValue) const;
	bool operator== (const int _iValue) const;
	bool operator== (const long _lValue) const;
	bool operator== (const double _dValue) const;
	bool operator== (const char* _zValue) const;
	bool operator== (const std::string& _sValue) const;

	bool operator!= (const bool _bValue) const;
	bool operator!= (const char _cValue) const;
	bool operator!= (const int _iValue) const;
	bool operator!= (const long _lValue) const;
	bool operator!= (const double _dValue) const;
	bool operator!= (const char* _zValue) const;
	bool operator!= (const std::string& _sValue) const;

	String& operator= (const bool _bValue);
	String& operator= (const char _cValue);
	String& operator= (const int _iValue);
	String& operator= (const long _lValue);
	String& operator= (const double _dValue);
	String& operator= (const char* _zValue);
	String& operator= (const String& _sValue);
	String& operator= (const std::string& _sValue);
	inline String& operator= (const String_dq_t& _dqValue);
	inline String& operator= (const String_lst_t& _lstValue);

	String& operator+= (const int _iValue);
	String& operator+= (const char _cValue);
	String& operator+= (const long _lValue);
	String& operator+= (const double _dValue);
	String& operator+= (const char* _zValue);
	String& operator+= (const std::string& _sValue);

	String operator+ (const int _iValue);
	String operator+ (const char _cValue);
	String operator+ (const long _lValue);
	String operator+ (const double _dValue);
	String operator+ (const char* _zValue);
	String operator+ (const std::string& _sValue);

	bool operator< (const bool _bValue) const;
	bool operator< (const char _cValue) const;
	bool operator< (const int _iValue) const;
	bool operator< (const long _lValue) const;
	bool operator< (const double _dValue) const;
	bool operator< (const char* _zValue) const;
	bool operator< (const std::string& _sValue) const;

	bool operator> (const bool _bValue) const;
	bool operator> (const char _cValue) const;
	bool operator> (const int _iValue) const;
	bool operator> (const long _lValue) const;
	bool operator> (const double _dValue) const;
	bool operator> (const char* _zValue) const;
	bool operator> (const std::string& _sValue) const;

	bool operator<= (const bool _bValue) const;
	bool operator<= (const char _cValue) const;
	bool operator<= (const int _iValue) const;
	bool operator<= (const long _lValue) const;
	bool operator<= (const double _dValue) const;
	bool operator<= (const char* _zValue) const;
	bool operator<= (const std::string& _sValue) const;

	bool operator>= (const bool _bValue) const;
	bool operator>= (const char _cValue) const;
	bool operator>= (const int _iValue) const;
	bool operator>= (const long _lValue) const;
	bool operator>= (const double _dValue) const;
	bool operator>= (const char* _zValue) const;
	bool operator>= (const std::string& _sValue) const;
};


String& operator<< (String& _rLeft, const String& _rRight);
String& operator<< (String& _rLeft, const std::string& _rRight);
String& operator<< (String& _rLeft, const char* _zRight);
String& operator<< (String& _rLeft, const char _cRight);
String& operator<< (String& _rLeft, const bool _bRight);
String& operator<< (String& _rLeft, const int _iRight);
String& operator<< (String& _rLeft, const long _lRight);
String& operator<< (String& _rLeft, const double _dRight);
String& operator<< (String& _rLeft, const String_dq_t& _rDq);
String& operator<< (String& _rLeft, const std::deque <std::string>& _rList);
String& operator<< (String& _rLeft, const String_lst_t& _rList);
String& operator<< (String& _rLeft, const std::list <std::string>& _rList);

std::ostream& operator<< (std::ostream& _rStream, const String_dq_t& _rDq);
std::ostream& operator<< (std::ostream& _rStream, const std::deque <std::string>& _rList);
std::ostream& operator<< (std::ostream& _rStream, const String_lst_t& _rList);
std::ostream& operator<< (std::ostream& _rStream, const std::list <std::string>& _rList);


#endif
