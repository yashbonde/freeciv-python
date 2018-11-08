#include <iostream>
#include <fstream>
#include "nlp_filesystem.h"
#include <nlp_macros.h>
#include <assert.h>
using namespace std;



// -----------------------------------------------------------------
CsvFile::CsvFile (void)
	: File ()
{
	e_FileFormat = cff_Unknown;
	ul_LineNumber = 0;
}


// -----------------------------------------------------------------
CsvFile::CsvFile (const char* _zFileName, ios_base::openmode _eMode)
	: File (_zFileName, _eMode)
{
	e_FileFormat = cff_Unknown;
	ul_LineNumber = 0;
	
	if (0 == (_eMode & ios_base::in))
		return;
	if (false == CsvFile::CheckFormat ())
		e_FileStatus = en_fs_format_error;
}


// -----------------------------------------------------------------
CsvFile::~CsvFile (void)
{
	ul_LineNumber = 0;
}


bool CsvFile::Open (const char* _zFileName, std::ios_base::openmode _eMode)
{
	bool bStatus = File::Open (_zFileName, _eMode);
	if (false == bStatus)
		return false;

	if (0 == (_eMode & ios_base::in))
		return true;
	if (false == CsvFile::CheckFormat ())
	{
		e_FileStatus = en_fs_format_error;
		return false;
	}
	return true;
}


// -----------------------------------------------------------------
void CsvFile::SetLineValueCount (unsigned long _lCount)
{
	ul_FirstLineValueCount = _lCount;
}


// -----------------------------------------------------------------
unsigned long CsvFile::GetFirstLineValueCount (void)
{
	return ul_FirstLineValueCount;
}


// -----------------------------------------------------------------
void CsvFile::SetSparseFileFormat (bool _bSparse)
{
	if (0 == (ios_base::out & e_Mode))
	{
		cerr << "[WARNING]  SetSparseFileFormat called on file opened in read-only mode."
			 << endl;
		return;
	}

	if (true == _bSparse)
		e_FileFormat = cff_SparseFormat;
	else
		e_FileFormat = cff_NonSparseFormat;
}


// -----------------------------------------------------------------
bool CsvFile::IsSparseFileFormat (void)
{
	return (cff_SparseFormat == e_FileFormat);
}


// -----------------------------------------------------------------
bool CsvFile::CheckFormat (void)
{
	if (en_fs_open != e_FileStatus)
	{
		cerr << "[ERROR]  Unable to read from file '" << s_FileName 
			 << "' since it has not yet been opened !" << endl;
		return false;
	}
	if (0 == (ios_base::in & e_Mode))
	{
		cerr << "[ERROR]  Unable to read from file '" << s_FileName 
			 << "' since it was not opened in read mode !" << endl;
		return false;
	}

	// remember starting location & state ...
	ios::iostate iStartingState = ios::rdstate ();
	streampos iStartingPos = istream::tellg ();

	// go to start of file & find out number of values on the line.
	istream::seekg (0, ios::beg);
	String sLine;
	std::getline (*this, sLine);
	char* pStart = (char*)(const char*)sLine;

	if (true == sLine.HasPattern ("^##_COLUMN_COUNT_##[ \t]*=[ \t]*[0-9]*$"))
	{
		e_FileFormat = cff_SparseFormat;
		sLine.ReplacePattern ("^##_COLUMN_COUNT_##[ \t]*=[ \t]*", "");
		ul_FirstLineValueCount = (long) sLine;
		return true;
	}

	e_FileFormat = cff_Unknown;
	ul_FirstLineValueCount = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		++ ul_FirstLineValueCount;
		if (',' == *pStart)
		{
			++ pStart;
			continue;
		}
			
		char* pColon = strchr (pStart, ':');
		pStart = strchr (pStart, ',');

		if (NULL == pColon)
		{
			if (cff_SparseFormat == e_FileFormat)
			{
				cerr << "[ERROR]  File seems to have lines in both sparse & non-sparse format!. "
					    "Filename : " << s_FileName << endl;
				cerr << sLine << endl;
				return false;
			}
			e_FileFormat = cff_NonSparseFormat;
		}
		else if (pColon < pStart)
		{
			if (cff_NonSparseFormat == e_FileFormat)
			{
				cerr << "[ERROR]  File seems to have lines in both sparse & non-sparse format!. "
					    "Filename : " << s_FileName << endl;
				cerr << sLine << endl;
				return false;
			}
			e_FileFormat = cff_SparseFormat;
		}
		else if (NULL != pStart)
		{
			cerr << "[ERROR]  File seems to have lines in both sparse & non-sparse format!. "
					"Filename : " << s_FileName << endl;
			cerr << sLine << endl;
			return false;
		}

		if (NULL == pStart)
			break;
		++ pStart;
	}

	// reset to starting location & state ...
	istream::seekg (iStartingPos);
	clear (iStartingState);

	if (cff_SparseFormat == e_FileFormat)
	{
		ul_FirstLineValueCount = 0;
		assert (false);
	}

	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (double_dq_t& _dqValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	_dqValues.clear ();
	_dqValues.resize (ul_FirstLineValueCount, 0);

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_dqValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_dqValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	*_pulCount = ulValues;
	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (float_dq_t& _dqValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	_dqValues.clear ();
	_dqValues.resize (ul_FirstLineValueCount, 0);

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_dqValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_dqValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}
	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	*_pulCount = ulValues;
	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (long_dq_t& _dqValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	_dqValues.clear ();
	_dqValues.resize (ul_FirstLineValueCount, 0);

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_dqValues [ulValues] = atol (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_dqValues [atol (pStart)] = atol (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}
	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	*_pulCount = ulValues;
	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (int_dq_t& _dqValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	_dqValues.clear ();
	_dqValues.resize (ul_FirstLineValueCount, 0);

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_dqValues [ulValues] = atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_dqValues [atol (pStart)] = atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}
	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	*_pulCount = ulValues;
	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (double* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (double));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (float* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (float));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (long* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (long));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atol (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atol (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (unsigned long* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned long));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atol (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atol (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (int* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (int));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (unsigned int* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned int));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (char* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (char));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = (char) atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = (char) atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLine (unsigned char* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned char));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = (unsigned char) atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = (unsigned char) atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}



// ==================================================================
bool CsvFile::ReadLastLine (double* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (double));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (float* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (float));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atof (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atof (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (long* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (long));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atol (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atol (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (unsigned long* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned long));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atol (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atol (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (int* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (int));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (unsigned int* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned int));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (char* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (char));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = (char) atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = (char) atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::ReadLastLine (unsigned char* _pValues, unsigned long* _pulCount)
{
	String sLine;
	if (false == File::ReadLastLine (sLine))
		return false;
	char* pStart = (char*)(const char*)sLine;
	
	memset (_pValues, 0, *_pulCount * sizeof (unsigned char));

	unsigned long ulValues = 0;
	while ('\0' != *pStart)
	{
		while ((' ' == *pStart) || ('\t' == *pStart))
			++ pStart;

		if (',' == *pStart)
		{
			++ ulValues;
			++ pStart;
			continue;
		}

		if (cff_NonSparseFormat == e_FileFormat)
			_pValues [ulValues] = (unsigned char) atoi (pStart);
		else if (cff_SparseFormat == e_FileFormat)
		{
			char* pValue = strchr (pStart, ':');
			*pValue = '\0';
			++ pValue;
			while ((' ' == *pValue) || ('\t' == *pValue))
				++ pValue;
			_pValues [atol (pStart)] = (unsigned char) atoi (pValue);
			pStart = pValue;
		}

		++ ulValues;
		pStart = strchr (pStart, ',');
		if (NULL == pStart)
			break;
		++ pStart;
	}

	if (cff_NonSparseFormat == e_FileFormat)
	{
		if ((0 != *_pulCount) && (ulValues != *_pulCount))
		{
			cerr << "[ERROR]  CSV file " << s_FileName << " was expected to have " 
				 << *_pulCount << " values on line " << ul_LineNumber + 1
				 << " however, the line had " << ulValues << " values." << endl;
			++ ul_LineNumber;
			return false;
		}
		*_pulCount = ulValues;
	}
	else if (cff_SparseFormat == e_FileFormat)
		*_pulCount = ul_FirstLineValueCount;

	++ ul_LineNumber;
	return true;
}




// -----------------------------------------------------------------
bool CsvFile::WriteLine (double_vec_t& _vecValues)
{
	this->precision (15);
	this->setf (ios::scientific);

	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _vecValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (double_vec_t, _vecValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (float_vec_t& _vecValues)
{
	this->precision (15);
	this->setf (ios::scientific);

	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _vecValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (float_vec_t, _vecValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (long_vec_t& _vecValues)
{
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _vecValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (long_vec_t, _vecValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}

// -----------------------------------------------------------------
bool CsvFile::WriteLine (int_vec_t& _vecValues)
{
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _vecValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (int_vec_t, _vecValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (double_dq_t& _dqValues)
{
	this->precision (15);
	this->setf (ios::scientific);

	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _dqValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (double_dq_t, _dqValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (float_dq_t& _dqValues)
{
	this->precision (15);
	this->setf (ios::scientific);

	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _dqValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (float_dq_t, _dqValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (long_dq_t& _dqValues)
{
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _dqValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (long_dq_t, _dqValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}

// -----------------------------------------------------------------
bool CsvFile::WriteLine (int_dq_t& _dqValues)
{
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _dqValues.size () << endl;

	bool bFirst = true;
	long lIndex = -1;
	ITERATE (int_dq_t, _dqValues, ite)
	{
		++ lIndex;
		if ((cff_SparseFormat == e_FileFormat) && (0 == *ite))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << lIndex << ':' << *ite;
		else
			*this << *ite;
	}
	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (double* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	this->precision (15);
	this->setf (ios::scientific);

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (float* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	this->precision (15);
	this->setf (ios::scientific);

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (long* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (unsigned long* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (int* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (unsigned int* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;
	
	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << _pValues [i];
		else
			*this << _pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (char* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << (unsigned int)_pValues [i];
		else
			*this << (unsigned int)_pValues [i];
	}

	*this << endl;
	return true;
}


// -----------------------------------------------------------------
bool CsvFile::WriteLine (unsigned char* _pValues, unsigned long _ulCount)
{
	if (0 == _ulCount)
		return true;
	if ((cff_SparseFormat == e_FileFormat) && (0 == (long) tellp ()))
		*this << "##_COLUMN_COUNT_## = " << _ulCount << endl;

	bool bFirst = true;
	for (unsigned long i = 0; i < _ulCount; ++ i)
	{
		if ((cff_SparseFormat == e_FileFormat) && (0 == _pValues [i]))
			continue;

		if (false == bFirst)
			*this << ',';
		else
			bFirst = false;

		if (cff_SparseFormat == e_FileFormat)
			*this << i << ':' << (unsigned int)_pValues [i];
		else
			*this << (unsigned int)_pValues [i];
	}

	*this << endl;
	return true;
}


