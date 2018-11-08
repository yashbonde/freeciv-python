#ifndef __NLP_VECTOR__
#define __NLP_VECTOR__
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <iostream>
using namespace std;


template <class TMPL_CLASS>
class Vector
{
	private:
		TMPL_CLASS*	p_Data;
		size_t		l_Size;
		bool		b_AllocatedMemory;
	
	public:
		Vector (void);
		Vector (size_t _i0);
		Vector (const Vector& _rCopyFrom);
		~Vector (void);

		void Copy (const Vector& _rVector);
		void SetData (TMPL_CLASS* _pData, size_t _iSize);
		void CopyData (TMPL_CLASS* _pData, size_t _iSize);
		void InsertDataBlock (size_t _iInsertAt, TMPL_CLASS* _pData, size_t _iSize);
		void Reserve (size_t _iSize);
		void Create (size_t _i0);
		void Resize (size_t _iSize);
		void Resize (size_t _iSize, int _iValue);
		// void Resize (size_t _iSize, TMPL_CLASS _rValue);

		// void SetSize (size_t _iSize);
		size_t Size (void) const { return l_Size; };

		void Memset (size_t _iValue);
		void Initialize (TMPL_CLASS _tValue);

		operator void* (void);
		operator TMPL_CLASS* (void);
		inline TMPL_CLASS& operator() (const size_t _i0) const;
		inline TMPL_CLASS& operator[] (const size_t _i0) const;
		inline TMPL_CLASS& operator[] (const long _i0) const;
		inline TMPL_CLASS& operator[] (const int _i0) const;

		// Vector& operator+ (const Vector& _rRight);
		// Vector& operator- (const Vector& _rRight);
		inline Vector& operator+= (const Vector& _rRight);
		inline Vector& operator-= (const Vector& _rRight);

		inline bool operator== (const Vector& _rRight) const;
		inline bool operator< (const Vector& _rRight) const;
		inline bool operator> (const Vector& _rRight) const;

		inline TMPL_CLASS Max (void) const;
		inline TMPL_CLASS Min (void) const;
		inline TMPL_CLASS Sum (void) const;
		inline TMPL_CLASS Average (void) const;
		bool HasNan (void);

		void PrintToStream (std::ostream& _rStream) const;
};


//													
template <class TMPL_CLASS>
std::ostream& operator<< (std::ostream& _rStream, const Vector<TMPL_CLASS>& _rVector)
{
	_rVector.PrintToStream (_rStream);
	return _rStream;
}


//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::Vector (void)
{
	p_Data = NULL;
	l_Size = 0;
	b_AllocatedMemory = true;
}




//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::Vector (size_t _i0)
{
	l_Size = _i0;
	p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
	b_AllocatedMemory = true;
	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::Vector (size_t), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
}




//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::Vector (const Vector& _rCopyFrom)
{
	l_Size = ((Vector&)_rCopyFrom).l_Size;
	if (l_Size > 0)
	{
		p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
		b_AllocatedMemory = true;
		if (NULL == p_Data)
			cerr << "[ERROR]  Vector::Vector (Vector&), memory allocation of " 
				 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
		assert (NULL != p_Data);
		memcpy (p_Data, _rCopyFrom.p_Data, sizeof (TMPL_CLASS) * l_Size);
	}
	else
	{
		p_Data = NULL;
		b_AllocatedMemory = false;
	}
}




//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::~Vector (void)
{
	assert ((TMPL_CLASS*)0x1 != p_Data);
	if (true == b_AllocatedMemory)
		free (p_Data);
	p_Data = (TMPL_CLASS*)0x1;
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Reserve (size_t _iSize)
{
	assert (NULL == p_Data);
	l_Size = _iSize;
	p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
	b_AllocatedMemory = true;
	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::Reserve (size_t), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Create (size_t _i0)
{
	if (NULL == p_Data)
	{
		l_Size = _i0;
		p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
		b_AllocatedMemory = true;
		if (NULL == p_Data)
			cerr << "[ERROR]  Vector::Create (size_t), memory allocation of " 
				 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
		assert (NULL != p_Data);
	}
	else
	{
		assert (l_Size > _i0);
		l_Size = _i0;
	}
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Resize (size_t _iSize)
{
	if (_iSize == l_Size)
		return;

	#ifndef NDEBUG
	if (l_Size > _iSize)
		cerr << "[WARNING]  Vector::Resize () called with a smaller value ("
			 << _iSize << ") than current size (" << l_Size << endl;
	#endif

	l_Size = _iSize;
	p_Data = (TMPL_CLASS*) realloc (p_Data, sizeof (TMPL_CLASS) * l_Size);
	b_AllocatedMemory = true;
	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::Resize (size_t), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Resize (size_t _iSize, int _iValue)
{
	if (_iSize == l_Size)
		return;

	#ifndef NDEBUG
	if (l_Size > _iSize)
		cerr << "[WARNING]  Vector::Resize () called with a smaller value ("
			 << _iSize << ") than current size (" << l_Size << endl;
	#endif

	p_Data = (TMPL_CLASS*) realloc (p_Data, sizeof (TMPL_CLASS) * _iSize);
	memset (p_Data + l_Size,
			_iValue,
			sizeof (TMPL_CLASS) * (_iSize - l_Size));
	b_AllocatedMemory = true;
	l_Size = _iSize;

	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::Resize (size_t), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
}




//													
/*
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Resize (size_t _iSize, TMPL_CLASS _rValue)
{
	if (_iSize == l_Size)
		return;

	#ifndef NDEBUG
	if (l_Size > _iSize)
		cerr << "[WARNING]  Vector::Resize () called with a smaller value ("
			 << _iSize << ") than current size (" << l_Size << endl;
	#endif

	p_Data = (TMPL_CLASS*) realloc (p_Data, sizeof (TMPL_CLASS) * _iSize);
	for (size_t i = l_Size; i < _iSize; ++ i)
		*(p_Data + i) = _rValue;

	b_AllocatedMemory = true;
	l_Size = _iSize;

	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::Resize (size_t), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
}
*/




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Copy (const Vector& _rVector)
{
	assert (NULL == p_Data);

	l_Size = ((Vector&)_rVector).l_Size;
	if (l_Size > 0)
	{
		p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
		b_AllocatedMemory = true;
		if (NULL == p_Data)
			cerr << "[ERROR]  Vector::Copy (Vector&), memory allocation of " 
				 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
		assert (NULL != p_Data);
		memcpy (p_Data, ((Vector&)_rVector).p_Data, sizeof (TMPL_CLASS) * l_Size);
	}
	else
	{
		p_Data = NULL;
		b_AllocatedMemory = false;
	}
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::SetData (TMPL_CLASS* _pData, size_t _iSize)
{
	assert ((NULL == p_Data) || (false == b_AllocatedMemory));

	l_Size = _iSize;
	p_Data = _pData;
	b_AllocatedMemory = false;
}




//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::CopyData (TMPL_CLASS* _pData, size_t _iSize)
{
	assert (NULL == p_Data);

	l_Size = _iSize;
	p_Data = (TMPL_CLASS*) malloc (sizeof (TMPL_CLASS) * l_Size);
	b_AllocatedMemory = true;
	if (NULL == p_Data)
		cerr << "[ERROR]  Vector::CopyData (), memory allocation of " 
			 << sizeof (TMPL_CLASS) * l_Size << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, _pData, sizeof (TMPL_CLASS) * l_Size);
}


//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::InsertDataBlock (size_t _iInsertAt, TMPL_CLASS* _pData, size_t _iSize)
{
	assert (NULL != p_Data);
	assert (l_Size >= _iInsertAt + _iSize);
	memcpy (p_Data + _iInsertAt, _pData, sizeof (TMPL_CLASS) * _iSize);
}



//													
/*
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::SetSize (size_t _iSize)
{
	assert (l_Size >= _iSize);
	l_Size = _iSize;
}
*/


//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Memset (size_t _iValue)
{
	memset (p_Data, _iValue, sizeof (TMPL_CLASS) * l_Size);
}



//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::Initialize (TMPL_CLASS _tValue)
{
	for (size_t i = 0; i < l_Size; ++ i)
		p_Data [i] = _tValue;
}



//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::operator void* (void)
{
	return (void*)p_Data;
}


//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>::operator TMPL_CLASS* (void)
{
	return p_Data;
}


//													
template <class TMPL_CLASS>
TMPL_CLASS& Vector<TMPL_CLASS>::operator() (const size_t _i0) const
{
	return p_Data [_i0];
}


//													
template <class TMPL_CLASS>
TMPL_CLASS& Vector<TMPL_CLASS>::operator[] (const size_t _i0) const
{
	return p_Data [_i0];
}


//													
template <class TMPL_CLASS>
TMPL_CLASS& Vector<TMPL_CLASS>::operator[] (const long _i0) const
{
	return p_Data [_i0];
}


//													
template <class TMPL_CLASS>
TMPL_CLASS& Vector<TMPL_CLASS>::operator[] (const int _i0) const
{
	return p_Data [_i0];
}


//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>& Vector<TMPL_CLASS>::operator+= (const Vector& _rRight)
{
	for (size_t i = 0; i < l_Size; ++ i)
		p_Data [i] += _rRight.p_Data [i];
	return *this;
}


//													
template <class TMPL_CLASS>
Vector<TMPL_CLASS>& Vector<TMPL_CLASS>::operator-= (const Vector& _rRight)
{
	for (size_t i = 0; i < l_Size; ++ i)
		p_Data [i] -= _rRight.p_Data [i];
	return *this;
}


//													
template <class TMPL_CLASS>
bool Vector<TMPL_CLASS>::operator== (const Vector& _rRight) const
{
	if (l_Size != _rRight.l_Size)
		return false;
	return (0 == memcmp (p_Data, _rRight.p_Data, sizeof (TMPL_CLASS) * l_Size));
}


//													
template <class TMPL_CLASS>
bool Vector<TMPL_CLASS>::operator< (const Vector& _rRight) const
{
	if (l_Size < _rRight.l_Size)
		return true;
	if (l_Size > _rRight.l_Size)
		return false;
	return (0 > memcmp (p_Data, _rRight.p_Data, sizeof (TMPL_CLASS) * l_Size));
}


//													
template <class TMPL_CLASS>
bool Vector<TMPL_CLASS>::operator> (const Vector& _rRight) const
{
	if (l_Size > _rRight.l_Size)
		return true;
	if (l_Size < _rRight.l_Size)
		return false;
	return (0 < memcmp (p_Data, _rRight.p_Data, sizeof (TMPL_CLASS) * l_Size));
}


//													
template <class TMPL_CLASS>
TMPL_CLASS Vector<TMPL_CLASS>::Max (void) const
{
	if (0 == l_Size)
		return 0;

	TMPL_CLASS oMax = p_Data [0];
	for (size_t i = 1; i < l_Size; ++ i)
	{
		if (oMax < p_Data [i])
			oMax = p_Data [i];
	}
	return oMax;
}


//													
template <class TMPL_CLASS>
TMPL_CLASS Vector<TMPL_CLASS>::Min (void) const
{
	if (0 == l_Size)
		return 0;

	TMPL_CLASS oMin = p_Data [0];
	for (size_t i = 1; i < l_Size; ++ i)
	{
		if (oMin > p_Data [i])
			oMin = p_Data [i];
	}
	return oMin;
}


//													
template <class TMPL_CLASS>
TMPL_CLASS Vector<TMPL_CLASS>::Sum (void) const
{
	TMPL_CLASS oSum = 0;
	for (size_t i = 0; i < l_Size; ++ i)
		oSum += p_Data [i];
	return oSum;
}


//													
template <class TMPL_CLASS>
TMPL_CLASS Vector<TMPL_CLASS>::Average (void) const
{
	TMPL_CLASS oSum = 0;
	for (size_t i = 0; i < l_Size; ++ i)
		oSum += p_Data [i];

	return oSum / (double) l_Size;
}


//													
template <class TMPL_CLASS>
bool Vector<TMPL_CLASS>::HasNan (void)
{
	for (size_t i = 0; i < l_Size; ++ i)
		if (true == isnan (p_Data [i]))
			return true;

	return false;
}


//													
template <class TMPL_CLASS>
void Vector<TMPL_CLASS>::PrintToStream (std::ostream& _rStream) const
{
	_rStream << p_Data [0];
	for (size_t i = 1; i < l_Size; ++ i)
		_rStream << ", " << p_Data [i];
}



#endif

