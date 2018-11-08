//                                  
//  AUTHOR      : S.R.K. Branavan	
//  DATE        : 11/09/2006		
//  FILE        : String.cpp		
//                                  

#include <iostream>
#include <assert.h>
#include "nlp_string.h"
using namespace std;


String::String (void)
	: string ()
{
}

String::String (const char* _zText)
	: string (_zText)
{
}

String::String (const String& _sText)
	: string (_sText.c_str ())
{
}

String::String (const string& _sText)
	: string (_sText.c_str ())
{
}

String::String (const deque <String>& _rDq)
{
	Join (_rDq);
}

String::String (const deque <string>& _rDq)
{
	Join (_rDq);
}

String::String (const list <String>& _rList)
{
	Join (_rList);
}

String::String (const list <string>& _rList)
{
	Join (_rList);
}



//==========================================================================
void String::Set (const char* _zText, long _lBytes)
{
	assert (_lBytes >= 0);

	string::clear ();
	if (0 == _lBytes)
		return;
	string::insert (0, _zText, _lBytes);
}



//==========================================================================
int String::Compare (const String& _rString)
{
	return (((string&)*this) == (string&) _rString);
}

int String::Compare (const char* _zValue)
{
	return (std::operator== (*this, _zValue));
}

int String::Compare (const char _cValue)
{
	const char* pThisString = c_str ();
	if (NULL == pThisString)
		return false;
	if ((pThisString [0] == _cValue) && (pThisString [1] == '\0'))
		return true;
	return false;
}

//==========================================================================
int String::CaseCompare (const String& _rString)
{
	return (strcasecmp (c_str (), _rString.c_str ()));
}

int String::CaseCompare (const char* _zValue)
{
	return (strcasecmp (c_str (), _zValue));
}

int String::CaseCompare (const char _cValue)
{
	const char* pThisString = c_str ();
	if (NULL == pThisString)
		return false;
	if (((0x20 | pThisString [0]) == (0x20 | _cValue)) && (pThisString [1] == '\0'))
		return true;
	return false;
}


//==========================================================================
bool String::IsWord (void)
{
    register const char* p = c_str ();
	if (('.' == *p) && ('\0' == *(p + 1)))
		return false;

    while ('\0' != *p)
	{
        if ((0 == isalnum (*p)) && ('\'' != *p) && ('.' != *p))
            return false;
		++ p;
	}
    return true;
}


bool String::IsAlNum (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 == isalnum (*(p++)))
            return false;
    return true;
}


bool String::IsAlpha (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 == isalpha (*(p++)))
            return false;
    return true;
}


bool String::IsDigit (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 == isdigit (*(p++)))
            return false;
    return true;
}


bool String::IsSpace (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 == isspace (*(p++)))
            return false;
    return true;
}


bool String::IsLower (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
	{
        if ((0 != isalpha (*p)) && (0 == islower (*p)))
            return false;
		++ p;
	}
    return true;
}


bool String::IsUpper (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
	{
        if ((0 != isalpha (*p)) && (0 == isupper (*p)))
            return false;
		++ p;
	}
    return true;
}


bool String::IsTitle (void)
{
    cerr << "String::IsTitle not implemented." << endl;
    abort ();
}


bool String::HasAlNum (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 != isalnum (*(p++)))
            return true;
    return false;
}


bool String::HasAlpha (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 != isalpha (*(p++)))
            return true;
    return false;
}


bool String::HasDigit (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 != isdigit (*(p++)))
            return true;
    return false;
}


bool String::HasSpace (void)
{
    register const char* p = c_str ();
    while ('\0' != *p)
        if (0 == isspace (*(p++)))
            return true;
    return false;
}








