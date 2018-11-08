//                                  
//  AUTHOR      : S.R.K. Branavan	
//  DATE        : 11/09/2006		
//  FILE        : String.cpp		
//                                  

#include "nlp_string.h"
#include <stdio.h>
using namespace std;


//==========================================================================
bool String::operator== (const char _cValue) const
{
	char zValue [2];
	zValue [0] = _cValue;
	zValue [1] = '\0';
	return (*(string*)this == zValue);
}

bool String::operator== (const bool _bValue) const
{
	return (_bValue == (bool)*this);
}

bool String::operator== (const int iValue) const
{
	return (iValue == (int)*this);
}

bool String::operator== (const long lValue) const
{
	return (lValue == (long)*this);
}

bool String::operator== (const double dValue) const
{
	return (dValue == (double)*this);
}

bool String::operator== (const char* zValue) const
{
	return ((*(string*)this) == zValue);
}

bool String::operator== (const string& sValue) const
{
	return ((*(string*)this) == sValue);
}

//==========================================================================
bool String::operator!= (const bool bValue) const
{
	return ! operator== (bValue);
}

bool String::operator!= (const char _cValue) const
{
	char zValue [2];
	zValue [0] = _cValue;
	zValue [1] = '\0';
	return (*(string*)this != zValue);
}

bool String::operator!= (const int iValue) const
{
	return (iValue != (int)*this);
}

bool String::operator!= (const long lValue) const
{
    return (lValue != (long)*this);
}

bool String::operator!= (const double dValue) const
{
	return (dValue != (double)*this);
}

bool String::operator!= (const char* zValue) const
{
	return ((*(string*)this) != zValue);
}

bool String::operator!= (const string& sValue) const
{
	return ((*(string*)this) != sValue);
}

//==========================================================================
String& String::operator= (const bool bValue)
{
	if (bValue)
		((string&)*this) = "true";
	else
		((string&)*this) = "false";
	return *this;
}

String& String::operator= (const char cValue)
{
	((string&)*this) = cValue;
	return *this;
}

String& String::operator= (const int iValue)
{
	char z [100];
	sprintf (z, "%d", iValue);
	((string&)*this) = z;
	return *this;
}

String& String::operator= (const long lValue)
{
	char z [100];
	sprintf (z, "%ld", lValue);
	((string&)*this) = z;
	return *this;
}

String& String::operator= (const double dValue)
{
	char z [1000];
	sprintf (z, "%0.02f", dValue);
	((string&)*this) = z;
	return *this;
}

String& String::operator= (const char* _zValue)
{
	(*(string*)this) = _zValue;
	return *this;
}

String& String::operator= (const String& _sValue)
{
	(*(string*)this) = _sValue.c_str ();
	return *this;
}

String& String::operator= (const string& _sValue)
{
	(*(string*)this) = _sValue.c_str ();
	return *this;
}

String& String::operator= (const String_dq_t& _dqValue)
{
	Join (_dqValue);
	return *this;
}

String& String::operator= (const String_lst_t& _lstValue)
{
	Join (_lstValue);
	return *this;
}


//==========================================================================
String& String::operator+= (const int iValue)
{
	char z [100];
	sprintf (z, "%d", iValue);
	((string&)*this) += z;
	return *this;
}

String& String::operator+= (const char cValue)
{
	((string&)*this) += cValue;
	return *this;
}

String& String::operator+= (const long lValue)
{
	char z [100];
	sprintf (z, "%ld", lValue);
	((string&) *this) += z;
	return *this;
}

String& String::operator+= (const double dValue)
{
	char z [1000];
	sprintf (z, "%0.02f", dValue);
	((string&) *this) += z;
	return *this;
}

String& String::operator+= (const char* _zValue)
{
	(*(string*)this) += _zValue;
	return *this;
}

String& String::operator+= (const string& _sValue)
{
	(*(string*)this) += _sValue;
	return *this;
}

//==========================================================================
String String::operator+ (const int iValue)
{
	String s (*this);
	s += iValue;
	return s;
}

String String::operator+ (const char cValue)
{
	String s (*this);
	s += cValue;
	return s;
}

String String::operator+ (const long lValue)
{
	String s (*this);
	s += lValue;
	return s;
}

String String::operator+ (const double dValue)
{
	String s (*this);
	s += dValue;
	return s;
}

String String::operator+ (const char* _zValue)
{
	String s (*this);
	s += _zValue;
	return s;
}

String String::operator+ (const string& _sValue)
{
	String s (*this);
	s += _sValue;
	return s;
}

//==========================================================================
bool String::operator< (const bool bValue) const
{
	String sTemp;
	sTemp = bValue;
	return ((string&) *this < (string&) sTemp);
}

bool String::operator< (const char cValue) const
{
	char zValue [2];
	zValue [0] = cValue;
	zValue [1] = '\0';
	return (*(string*)this < zValue);
}

bool String::operator< (const int iValue) const
{
	String sTemp;
	sTemp = iValue;
	return ((string&) *this < (string&) sTemp);
}

bool String::operator< (const long lValue) const
{
	String sTemp;
	sTemp = lValue;
	return ((string&) *this < (string&) sTemp);
}

bool String::operator< (const double dValue) const
{
	String sTemp;
	sTemp = dValue;
	return ((string&) *this < (string&) sTemp);
}

bool String::operator< (const char* _zValue) const
{
	return ((*(string*)this) < _zValue);
}

bool String::operator< (const string& _sValue) const
{
	return ((*(string*)this) < _sValue);
}

//========================================================================
bool String::operator> (const bool bValue) const
{
	String sTemp;
	sTemp = bValue;
	return ((string&) *this > (string&) sTemp);
}

bool String::operator> (const char cValue) const
{
	char zValue [2];
	zValue [0] = cValue;
	zValue [1] = '\0';
	return (*(string*)this > zValue);
}

bool String::operator> (const int iValue) const
{
	String sTemp;
	sTemp = iValue;
	return ((string&) *this > (string&) sTemp);
}

bool String::operator> (const long lValue) const
{
	String sTemp;
	sTemp = lValue;
	return ((string&) *this > (string&) sTemp);
}

bool String::operator> (const double dValue) const
{
	String sTemp;
	sTemp = dValue;
	return ((string&) *this > (string&) sTemp);
}

bool String::operator> (const char* _zValue) const
{
	return ((*(string*)this) > _zValue);
}

bool String::operator> (const string& _sValue) const
{
	return ((*(string*)this) > _sValue);
}


//========================================================================
bool String::operator<= (const bool bValue) const
{
	String sTemp;
	sTemp = bValue;
	return ((string&) *this <= (string&) sTemp);
}

bool String::operator<= (const char cValue) const
{
	char zValue [2];
	zValue [0] = cValue;
	zValue [1] = '\0';
	return (*(string*)this <= zValue);
}

bool String::operator<= (const int iValue) const
{
	String sTemp;
	sTemp = iValue;
	return ((string&) *this <= (string&) sTemp);
}

bool String::operator<= (const long lValue) const
{
	String sTemp;
	sTemp = lValue;
	return ((string&) *this <= (string&) sTemp);
}

bool String::operator<= (const double dValue) const
{
	String sTemp;
	sTemp = dValue;
	return ((string&) *this <= (string&) sTemp);
}

bool String::operator<= (const char* _zValue) const
{
	return ((*(string*)this) <= _zValue);
}

bool String::operator<= (const string& _sValue) const
{
	return ((*(string*)this) <= _sValue);
}

//==========================================================================
bool String::operator>= (const bool bValue) const
{
	String sTemp;
	sTemp = bValue;
	return ((string&) *this >= (string&) sTemp);
}

bool String::operator>= (const char cValue) const
{
	char zValue [2];
	zValue [0] = cValue;
	zValue [1] = '\0';
	return (*(string*)this >= zValue);
}

bool String::operator>= (const int iValue) const
{
	String sTemp;
	sTemp = iValue;
	return ((string&) *this >= (string&) sTemp);
}

bool String::operator>= (const long lValue) const
{
	String sTemp;
	sTemp = lValue;
	return ((string&) *this >= (string&) sTemp);
}

bool String::operator>= (const double dValue) const
{
	String sTemp;
	sTemp = dValue;
	return ((string&) *this >= (string&) sTemp);
}

bool String::operator>= (const char* _zValue) const
{
	return ((*(string*)this) <= _zValue);
}

bool String::operator>= (const string& _sValue) const
{
	return ((*(string*)this) <= _sValue);
}


//========================================================================
String:: operator bool (void) const
{
	return (((string&) *this) == "true");
}

String:: operator int (void) const
{
	return atoi (c_str ());
}

String:: operator long (void) const
{
	return atol (c_str ());
}

String:: operator double (void) const
{
	return atof (c_str ());
}

String:: operator const char* (void) const
{
	return c_str ();
}

String:: operator string (void) const
{
	return *(string*)this;
}


//========================================================================
String& operator<< (String& _rLeft, const String& _rRight)
{
	(string&) _rLeft += (string&) _rRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const string& _rRight)
{
	(string&) _rLeft += _rRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const char _cRight)
{
	_rLeft += (char) _cRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const char* _zRight)
{
	(string&) _rLeft += _zRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const bool _bRight)
{
	_rLeft += (true == _bRight)? "[true]" : "[false]";
	return _rLeft;
}

String& operator<< (String& _rLeft, const int _iRight)
{
	_rLeft += _iRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const long _lRight)
{
	_rLeft += _lRight;
	return _rLeft;
}

String& operator<< (String& _rLeft, const double _dRight)
{
	_rLeft += _dRight;
	return _rLeft;
}


// ---------------------------------------------------------------------
String& operator<< (String& _rLeft, const deque <String>& _rDq)
{
	deque <String>::iterator	ite = ((deque <String>&)_rDq).begin ();
	if (ite == ((deque <String>&)_rDq).end ())
		return _rLeft;

	_rLeft += *ite;
	++ ite;
	for (; ite != ((deque <String>&)_rDq).end ();
		++ ite)
	{
		_rLeft += ' ';
		_rLeft += *ite;
	}
	return _rLeft;
}


String& operator<< (String& _rLeft, const deque <string>& _rDq)
{
	deque <string>::iterator	ite = ((deque <string>&)_rDq).begin ();
	if (ite == ((deque <string>&)_rDq).end ())
		return _rLeft;

	_rLeft += *ite;
	++ ite;
	for (; ite != ((deque <string>&)_rDq).end ();
		++ ite)
	{
		_rLeft += ' ';
		_rLeft += *ite;
	}
	return _rLeft;
}


ostream& operator<< (ostream& _rStream, const deque <String>& _rDq)
{
	deque <String>::iterator	ite = ((deque <String>&)_rDq).begin ();
	if (ite == ((deque <String>&)_rDq).end ())
		return _rStream;

	_rStream << '[' << *ite << ']';
	++ ite;
	for (; ite != ((deque <String>&)_rDq).end ();
		++ ite)
	{
		_rStream << " [" << *ite << ']';
	}
	return _rStream;
}


ostream& operator<< (ostream& _rStream, const deque <string>& _rDq)
{
	deque <string>::iterator	ite = ((deque <string>&)_rDq).begin ();
	if (ite == ((deque <string>&)_rDq).end ())
		return _rStream;

	_rStream << '[' << *ite << ']';
	++ ite;
	for (; ite != ((deque <string>&)_rDq).end ();
		++ ite)
	{
		_rStream << " [" << *ite << ']';
	}
	return _rStream;
}


// ---------------------------------------------------------------------
String& operator<< (String& _rLeft, const list <String>& _rList)
{
	list <String>::iterator	ite = ((list <String>&)_rList).begin ();
	if (ite == ((list <String>&)_rList).end ())
		return _rLeft;

	_rLeft += *ite;
	++ ite;
	for (; ite != ((list <String>&)_rList).end ();
		++ ite)
	{
		_rLeft += ' ';
		_rLeft += *ite;
	}
	return _rLeft;
}


String& operator<< (String& _rLeft, const list <string>& _rList)
{
	list <string>::iterator	ite = ((list <string>&)_rList).begin ();
	if (ite == ((list <string>&)_rList).end ())
		return _rLeft;

	_rLeft += *ite;
	++ ite;
	for (; ite != ((list <string>&)_rList).end ();
		++ ite)
	{
		_rLeft += ' ';
		_rLeft += *ite;
	}
	return _rLeft;
}


ostream& operator<< (ostream& _rStream, const list <String>& _rList)
{
	list <String>::iterator	ite = ((list <String>&)_rList).begin ();
	if (ite == ((list <String>&)_rList).end ())
		return _rStream;

	_rStream << '[' << *ite << ']';
	++ ite;
	for (; ite != ((list <String>&)_rList).end ();
		++ ite)
	{
		_rStream << " [" << *ite << ']';
	}
	return _rStream;
}


ostream& operator<< (ostream& _rStream, const list <string>& _rList)
{
	list <string>::iterator	ite = ((list <string>&)_rList).begin ();
	if (ite == ((list <string>&)_rList).end ())
		return _rStream;

	_rStream << '[' << *ite << ']';
	++ ite;
	for (; ite != ((list <string>&)_rList).end ();
		++ ite)
	{
		_rStream << " [" << *ite << ']';
	}
	return _rStream;
}


