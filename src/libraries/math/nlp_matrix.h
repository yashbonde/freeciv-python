#ifndef __NLP_MATRIX__
#define __NLP_MATRIX__
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
using namespace std;



//													
template <class TMPL_CLASS, int TMPL_DIM>
class Matrix
{
	friend class Matrix <TMPL_CLASS, 5>;
	friend class Matrix <TMPL_CLASS, 4>;
	friend class Matrix <TMPL_CLASS, 3>;
	friend class Matrix <TMPL_CLASS, 2>;
	friend class Matrix <TMPL_CLASS, 1>;


	private:
		TMPL_CLASS*	p_Data;
		size_t		l_AvailableMemory;
		long		l_Offsets [TMPL_DIM];
		long		l_FirstDimension;
		int			i_Dimensions;
		bool		b_IsSlice;
	
	public:
		Matrix (void);
		Matrix (int _i0);
		Matrix (int _i0, int _i1);
		Matrix (int _i0, int _i1, int _i2);
		Matrix (int _i0, int _i1, int _i2, int _i3);
		Matrix (int _i0, int _i1, int _i2, int _i3, int _i4);
		Matrix (const Matrix& _rMatrix, int _i0);
		Matrix (const Matrix& _rMatrix, int _i0, int _i1);
		Matrix (const Matrix& _rMatrix, int _i0, int _i1, int _i2);
		Matrix (const Matrix& _rMatrix, int _i0, int _i1, int _i2, int _i3);

		~Matrix (void);
		void Destroy (void);

		bool IsInitialized (void)
			{ return (0 != l_AvailableMemory); };

		void Create (int _i0);
		void Create (int _i0, int _i1);
		void Create (int _i0, int _i1, int _i2);
		void Create (int _i0, int _i1, int _i2, int _i3);
		void Create (int _i0, int _i1, int _i2, int _i3, int _i4);

		void Memset (int _iValue);
		void Initialize (TMPL_CLASS _tValue);

		void Copy (const Matrix <TMPL_CLASS, TMPL_DIM>& _rMatrix);

		void GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 1>& _rMatrix, int _i0);
		void GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 2>& _rMatrix, int _i0, int _i1);
		void GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 3>& _rMatrix, int _i0, int _i1, int _i2);
		void GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 4>& _rMatrix, int _i0, int _i1, int _i2, int _i3);

		void CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 1>& _rMatrix, int _i0);
		void CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 2>& _rMatrix, int _i0, int _i1);
		void CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 3>& _rMatrix, int _i0, int _i1, int _i2);
		void CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 4>& _rMatrix, int _i0, int _i1, int _i2, int _i3);

		operator void* (void);
		inline TMPL_CLASS& operator() (int _i0);
		inline TMPL_CLASS& operator() (int _i0, int _i1);
		inline TMPL_CLASS& operator() (int _i0, int _i1, int _i2);
		inline TMPL_CLASS& operator() (int _i0, int _i1, int _i2, int _i3);
		inline TMPL_CLASS& operator() (int _i0, int _i1, int _i2, int _i3, int _i4);

		Matrix& operator+= (const TMPL_CLASS _tValue);
		Matrix& operator-= (const TMPL_CLASS _tValue);

		bool operator== (const Matrix& _rRight) const;
		bool operator!= (const Matrix& _rRight) const
			{ return (false == (operator== (_rRight))); };

		bool HasNan (void);
		bool IsZero (void);
		TMPL_CLASS Max (void);
		TMPL_CLASS Min (void);

		void PrintToStream (std::ostream& _rStream) const;
};


//													
template <class TMPL_CLASS, int TMPL_DIM>
std::ostream& operator<< (std::ostream& _rStream, const Matrix<TMPL_CLASS, TMPL_DIM>& _rMatrix)
{
	_rMatrix.PrintToStream (_rStream);
	return _rStream;
}




//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (void)
{
	p_Data = NULL;
	l_AvailableMemory = 0;
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;
	memset (l_Offsets, 0, sizeof (long) * TMPL_DIM);
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::~Matrix (void)
{
	assert ((TMPL_CLASS*)0x1 != p_Data);
	if (false == b_IsSlice)
		free (p_Data);
	p_Data = (TMPL_CLASS*)0x1;
	l_AvailableMemory = -1;
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Destroy (void)
{
	if ((NULL != p_Data) && (false == b_IsSlice))
		free (p_Data);
	p_Data = NULL;
	l_AvailableMemory = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (int _i0)
{
	assert (1 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Matrix (int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (int _i0, int _i1)
{
	assert (2 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Matrix (int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1;
	l_Offsets [1] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (int _i0, int _i1, int _i2)
{
	assert (3 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Matrix (int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2;
	l_Offsets [1] = _i2;
	l_Offsets [2] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (int _i0, int _i1, int _i2, int _i3)
{
	assert (4 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2 * _i3;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Matrix (int, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2 * _i3;
	l_Offsets [1] = _i2 * _i3;
	l_Offsets [2] = _i3;
	l_Offsets [3] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (int _i0, int _i1, int _i2, int _i3, int _i4)
{
	assert (5 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2 * _i3 * _i4;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Matrix (int, int, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2 * _i3 * _i4;
	l_Offsets [1] = _i2 * _i3 * _i4;
	l_Offsets [2] = _i3 * _i4;
	l_Offsets [3] = _i4;
	l_Offsets [4] = -1;
	l_FirstDimension = _i0;
}






//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (const Matrix& _rMatrix, int _i0)
{
	assert (false);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 1));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [0];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 1];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (const Matrix& _rMatrix, int _i0, int _i1)
{
	assert (false);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 2));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [1];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 2];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (const Matrix& _rMatrix, int _i0, int _i1, int _i2)
{
	assert (false);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 3));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [2];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 3];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::Matrix (const Matrix& _rMatrix, int _i0, int _i1, int _i2, int _i3)
{
	assert (false);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 4));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [3];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2 +
							   _rMatrix.l_Offsets [3] * _i3];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}








//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Create (int _i0)
{
	assert (NULL == p_Data);
	assert (1 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Create (int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Create (int _i0, int _i1)
{
	assert (NULL == p_Data);
	assert (2 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Create (int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1;
	l_Offsets [1] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Create (int _i0, int _i1, int _i2)
{
	assert (NULL == p_Data);
	assert (3 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Create (int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2;
	l_Offsets [1] = _i2;
	l_Offsets [2] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Create (int _i0, int _i1, int _i2, int _i3)
{
	assert (NULL == p_Data);
	assert (4 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2 * _i3;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Create (int, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2 * _i3;
	l_Offsets [1] = _i2 * _i3;
	l_Offsets [2] = _i3;
	l_Offsets [3] = -1;
	l_FirstDimension = _i0;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Create (int _i0, int _i1, int _i2, int _i3, int _i4)
{
	assert (NULL == p_Data);
	assert (5 == TMPL_DIM);
	l_AvailableMemory = sizeof (TMPL_CLASS) * _i0 * _i1 * _i2 * _i3 * _i4;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Create (int, int, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [0] = _i1 * _i2 * _i3 * _i4;
	l_Offsets [1] = _i2 * _i3 * _i4;
	l_Offsets [2] = _i3 * _i4;
	l_Offsets [3] = _i4;
	l_FirstDimension = _i0;
}





//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Copy (const Matrix <TMPL_CLASS, TMPL_DIM>& _rMatrix)
{
	assert (NULL == p_Data);
	assert (TMPL_DIM == _rMatrix.i_Dimensions);

	l_AvailableMemory = _rMatrix.l_AvailableMemory;
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::Copy (Matrix&), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, _rMatrix.p_Data, l_AvailableMemory);

	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	for (int i = 0; i < TMPL_DIM; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i];
	l_FirstDimension = _rMatrix.l_FirstDimension;
}





//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 1>& _rMatrix,
											 int _i0)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 1));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [0];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 1];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 2>& _rMatrix,
											 int _i0, int _i1)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 2));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [1];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 2];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 3>& _rMatrix, 
											 int _i0, int _i1, int _i2)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 3));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [2];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 3];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::GetSlice (const Matrix <TMPL_CLASS, TMPL_DIM + 4>& _rMatrix,
											 int _i0, int _i1, int _i2, int _i3)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 4));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [3];
	assert (l_AvailableMemory > 0);
	p_Data = &_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2 +
							   _rMatrix.l_Offsets [3] * _i3];
	i_Dimensions = TMPL_DIM;
	b_IsSlice = true;

	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}






//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 1>& _rMatrix,
											 int _i0)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 1));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [0];
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::CopySlice (Matrix&, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, 
			&_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0],
			l_AvailableMemory);

	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 1];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 2>& _rMatrix,
											 int _i0, int _i1)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 2));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [1];
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::CopySlice (Matrix&, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, 
			&_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1],
			l_AvailableMemory);
	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 2];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 3>& _rMatrix, 
											 int _i0, int _i1, int _i2)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 3));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [2];
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::CopySlice (Matrix&, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, 
			&_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2],
			l_AvailableMemory);

	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	for (int i = 0; i < TMPL_DIM - 1; ++ i)
		l_Offsets [i] = _rMatrix.l_Offsets [i + 3];
	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = -1;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::CopySlice (const Matrix <TMPL_CLASS, TMPL_DIM + 4>& _rMatrix,
											 int _i0, int _i1, int _i2, int _i3)
{
	// assert (NULL == p_Data);
	assert (TMPL_DIM == (_rMatrix.i_Dimensions - 4));
	l_AvailableMemory = sizeof (TMPL_CLASS) * _rMatrix.l_Offsets [3];
	assert (l_AvailableMemory > 0);
	p_Data = (TMPL_CLASS*) malloc (l_AvailableMemory);
	if (NULL == p_Data)
		cerr << "[ERROR]  Matrix::CopySlice (Matrix&, int, int, int, int), memory allocation of " 
			 << l_AvailableMemory << " bytes failed." << endl;
	assert (NULL != p_Data);
	memcpy (p_Data, 
			&_rMatrix.p_Data [_rMatrix.l_Offsets [0] * _i0 + 
							   _rMatrix.l_Offsets [1] * _i1 +
							   _rMatrix.l_Offsets [2] * _i2 +
							   _rMatrix.l_Offsets [3] * _i3],
			l_AvailableMemory);

	i_Dimensions = TMPL_DIM;
	b_IsSlice = false;

	l_Offsets [TMPL_DIM - 1] = -1;
	l_FirstDimension = _rMatrix.l_Offsets [3];
}








//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Memset (int _iValue)
{
	// assert (false == b_IsSlice);
	// memset (p_Data, _iValue, sizeof (TMPL_CLASS) * l_Offsets [0] * l_FirstDimension);
	memset (p_Data, _iValue, l_AvailableMemory);
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::Initialize (TMPL_CLASS _tValue)
{
	assert (false == b_IsSlice);
	// for (long i = 0; i < l_Offsets [0] * l_FirstDimension; ++ i)
	for (long i = 0; i < l_AvailableMemory / sizeof(TMPL_CLASS); ++ i)
		p_Data [i] = _tValue;
}







//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>::operator void* (void)
{
	return (void*)p_Data;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS& Matrix<TMPL_CLASS, TMPL_DIM>::operator() (int _i0)
{
	assert (1 == TMPL_DIM);
	assert (l_AvailableMemory >= (size_t) _i0 * sizeof(TMPL_CLASS));

	return p_Data [_i0];
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS& Matrix<TMPL_CLASS, TMPL_DIM>::operator() (int _i0, int _i1)
{
	assert (2 == TMPL_DIM);
	assert (l_AvailableMemory >= (size_t) (l_Offsets [0] * _i0 + _i1)
										  * sizeof (TMPL_CLASS));

	return p_Data [l_Offsets [0] * _i0 +
				   _i1];
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS& Matrix<TMPL_CLASS, TMPL_DIM>::operator() (int _i0, int _i1, int _i2)
{
	assert (3 == TMPL_DIM);
	assert (l_AvailableMemory >= (size_t) (l_Offsets [0] * _i0 + 
										   l_Offsets [1] * _i1 + _i2)
										  * sizeof (TMPL_CLASS));

	return p_Data [l_Offsets [0] * _i0 + 
				   l_Offsets [1] * _i1 + 
				   _i2];
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS& Matrix<TMPL_CLASS, TMPL_DIM>::operator() (int _i0, int _i1, int _i2, int _i3)
{
	assert (4 == TMPL_DIM);
	assert (l_AvailableMemory >= (size_t) (l_Offsets [0] * _i0 + 
										   l_Offsets [1] * _i1 +
										   l_Offsets [2] * _i2 + _i3)
										  * sizeof (TMPL_CLASS));

	return p_Data [l_Offsets [0] * _i0 + 
				   l_Offsets [1] * _i1 +
				   l_Offsets [2] * _i2 +
				   _i3];
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS& Matrix<TMPL_CLASS, TMPL_DIM>::operator() (int _i0, int _i1, int _i2, int _i3, int _i4)
{
	assert (5 == TMPL_DIM);
	assert (l_AvailableMemory >= (size_t) (l_Offsets [0] * _i0 + 
										   l_Offsets [1] * _i1 +
										   l_Offsets [2] * _i2 +
										   l_Offsets [3] * _i3 + _i4)
										  * sizeof (TMPL_CLASS));

	return p_Data [l_Offsets [0] * _i0 + 
				   l_Offsets [1] * _i1 +
				   l_Offsets [2] * _i2 +
				   l_Offsets [3] * _i3 +
				   _i4];
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>& Matrix<TMPL_CLASS, TMPL_DIM>::operator+= (const TMPL_CLASS _tValue)
{
	assert (false == b_IsSlice);
	for (long i = 0; i < l_Offsets [0] * l_FirstDimension; ++ i)
		p_Data [i] += _tValue;

	return *this;
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
Matrix<TMPL_CLASS, TMPL_DIM>& Matrix<TMPL_CLASS, TMPL_DIM>::operator-= (const TMPL_CLASS _tValue)
{
	assert (false == b_IsSlice);
	for (long i = 0; i < l_Offsets [0] * l_FirstDimension; ++ i)
		p_Data [i] -= _tValue;

	return *this;
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
bool Matrix<TMPL_CLASS, TMPL_DIM>::operator== (const Matrix& _rRight) const
{
	if (i_Dimensions != _rRight.i_Dimensions)
		return false;
	if (l_AvailableMemory != _rRight.l_AvailableMemory)
		return false;
	for (int i = 0; i < i_Dimensions; ++ i)
	{
		if (l_Offsets [i] != _rRight.l_Offsets [i])
			return false;
	}
	if (0 != memcmp (p_Data, _rRight.p_Data, l_AvailableMemory))
		return false;
	return true;
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
bool Matrix<TMPL_CLASS, TMPL_DIM>::HasNan (void)
{
	for (size_t i = 0; i < l_AvailableMemory / sizeof(TMPL_CLASS); ++ i)
		if (true == isnan (p_Data [i]))
			return true;

	return false;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
bool Matrix<TMPL_CLASS, TMPL_DIM>::IsZero (void)
{
	for (size_t i = 0; i < l_AvailableMemory / sizeof(TMPL_CLASS); ++ i)
		if (0 != p_Data [i])
			return false;

	return true;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS Matrix<TMPL_CLASS, TMPL_DIM>::Max (void)
{
	TMPL_CLASS oMax = p_Data [0];
	for (size_t i = 1; i < l_AvailableMemory / sizeof(TMPL_CLASS); ++ i)
	{
		if (oMax < p_Data [i])
			oMax = p_Data [i];
	}

	return oMax;
}


//													
template <class TMPL_CLASS, int TMPL_DIM>
TMPL_CLASS Matrix<TMPL_CLASS, TMPL_DIM>::Min (void)
{
	TMPL_CLASS oMin = p_Data [0];
	for (size_t i = 1; i < l_AvailableMemory / sizeof(TMPL_CLASS); ++ i)
	{
		if (oMin > p_Data [i])
			oMin = p_Data [i];
	}

	return oMin;
}



//													
template <class TMPL_CLASS, int TMPL_DIM>
void Matrix<TMPL_CLASS, TMPL_DIM>::PrintToStream (std::ostream& _rStream) const
{
	if (1 != TMPL_DIM)
		return;

	_rStream << p_Data [0];
	for (size_t i = 1; i < l_AvailableMemory / sizeof (TMPL_CLASS); ++ i)
		_rStream << ", " << p_Data [i];
}




#endif

