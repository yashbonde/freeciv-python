#include <iostream>
#include <fstream>
#include <errno.h>
#include "nlp_filesystem.h"
using namespace std;



// -----------------------------------------------------------------
File::File (void)
{
	p_File = NULL;
	p_FDBuf = NULL;
	e_FileStatus = en_fs_initial;
}


// -----------------------------------------------------------------
File::File (const char* _zFileName, ios_base::openmode _eMode)
{
	p_File = NULL;
	p_FDBuf = NULL;
	e_FileStatus = en_fs_initial;
	File::Open (_zFileName, _eMode);
}


// -----------------------------------------------------------------
File::~File (void)
{
	File::Close ();
	e_FileStatus = en_fs_destroyed;
	delete p_FDBuf;
}


// -----------------------------------------------------------------
bool File::Open (const char* _zFileName, ios_base::openmode _eMode)
{
	if (en_fs_open == e_FileStatus)
	{
		cerr << "[ERROR]  Unable to open file '" << _zFileName 
			 << "'. Object already has file '" << s_FileName 
			 << "' open for " 
			 << ((ios_base::out & e_Mode)? "writing" : "reading")
			 << " !" << endl;

		return false;
	}

	s_FileName = _zFileName;
	s_FileName.Strip ();
	String sMode = "r";
	if (_eMode & ios_base::app)
	{
		sMode = 'a';
		if (_eMode & ios_base::in)
			sMode += '+';
	}
	else if (_eMode & ios_base::out)
	{
		sMode = 'w';
		if (_eMode & ios_base::in)
			sMode += '+';
	}
	else if (ios_base::in != _eMode)
	{
		cerr << "[ERROR]  Unknown mode parameter in call File::Open ("
			 << _zFileName << ", " << _eMode << ")" << endl;
		return false;
	}

	p_File = fopen (s_FileName, sMode);
	if (NULL == p_File)
	{
		String sError (strerror (errno));
		// need to add actual reason for error to this message !
		cerr << "[ERROR]  Failed to open file '" << s_FileName 
			 << "' for " 
			 << ((ios_base::out & _eMode)? "writing" : "reading")
			 << " (fopen)!  Cause : " << sError << endl;

		e_FileStatus = en_fs_open_failed;
		return false;
	}

	p_FDBuf = new __gnu_cxx::stdio_filebuf<char> (p_File, _eMode);
	basic_ios<char>::rdbuf (p_FDBuf);

	e_Mode = _eMode;
	s_FileName = _zFileName;
	e_FileStatus = en_fs_open;
	return true;
}


// -----------------------------------------------------------------
void File::Close (void)
{
	if (en_fs_open == e_FileStatus)
	{
		fstream::close ();
		if ((ios_base::out & e_Mode) ||
			(ios_base::app & e_Mode))
			fflush (p_File);
		
		fclose (p_File);
	}

	fstream::clear ();
	e_FileStatus = en_fs_closed;
}


// -----------------------------------------------------------------
bool File::ReadLines (String_dq_t& _rdqLines)
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

	_rdqLines.clear ();

	String sLine;
	while ((false == fstream::eof ()) && (false == fstream::bad ()))
	{
		std::getline (*this, sLine);
		if (("" == sLine) && (false == fstream::good ()))
			continue;
		_rdqLines.push_back (sLine);
	}

	return true;
}


// -----------------------------------------------------------------
bool File::ReadLine (String& _rsLine)
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

	if (false == fstream::good ())
		return false;
	std::getline (*this, _rsLine);
	if ((false == fstream::good ()) &&
		("" == _rsLine))
		return false;
	return true;
}


// -----------------------------------------------------------------
bool File::ReadLastLine (String& _rsLine)
{
	// This method is especially useful for quickly reading	
	// the last line off a very large file.					
	
	// Do initial goodness checks ...						
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
	if (false == fstream::good ())
		return false;

	// Find the end of the file for use later ...			
	seekg (0, ios::end);
	long lFileEnd = tellg ();
	seekg (0, ios::beg);

	// Compute the total length of 10 lines ...				
	String sLine = " ";
	String sLastLine;
	int iLines = 0;
	long lTotalLineLength = 0;
	while ((iLines < 10) && (tellg () < lFileEnd))
	{
		std::getline (*this, sLine);

		String sCheck (sLine);
		sCheck.Strip ();
		if ("" == sCheck)
			continue;

		++ iLines;
		lTotalLineLength += sLine.length ();
		sLastLine = sLine;
	}

	// We might have hit the end of the file above ...		
	if (tellg () >= lFileEnd)
	{
		_rsLine = sLastLine;
		return true;
	}

	// Read last line ...									
	// We do this by seeking back the length of 10 lines as	
	// computed earlier, and then reading forward...		

	// seek backwards till we're no longer on the last line	
	long lOffsetFromEnd = lTotalLineLength;
	seekg (- lOffsetFromEnd, ios::end);
	std::getline (*this, sLine);
	while (tellg () >= lFileEnd)
	{
		lOffsetFromEnd += lTotalLineLength;
		seekg (- lOffsetFromEnd, ios::end);
		std::getline (*this, sLine);
	}

	// seek forwards till we're on the last line			
	std::getline (*this, sLine);
	while (tellg () < lFileEnd)
		std::getline (*this, sLine);

	_rsLine = sLine;

/*
	iLines = 0;
	while (iLines <= 1)
	{
		// Have to seek backwards till we find one complete	
		// line ...											
		iLines = 0;
		seekg (- lTotalLineLength, ios::cur);
		while ((true == fstream::good ()) && ("" != sLine))
		{
			std::getline (*this, sLine);
			String sCheck (sLine);
			sCheck.Strip ();
			if ("" == sCheck)
				continue;

			++ iLines;
			sLastLine = sLine;
		}
	}
	_rsLine = sLastLine;
*/
	
	return true;
}


// -----------------------------------------------------------------
bool File::WriteLines (String_dq_t& _rdqLines)
{
	if (en_fs_open != e_FileStatus)
	{
		cerr << "[ERROR]  Unable to write to file '" << s_FileName 
			 << "' since it has not yet been opened !" << endl;
		return false;
	}
	if (0 == (ios_base::out & e_Mode))
	{
		cerr << "[ERROR]  Unable to write to file '" << s_FileName 
			 << "' since it was not opened in write mode !" << endl;
		return false;
	}

	String_dq_t::iterator	ite = _rdqLines.begin ();
	String_dq_t::iterator	ite_end = _rdqLines.end ();
	for (; ite != ite_end; ++ ite)
		(*this) << *ite << endl;

	fstream::flush ();
	return true;
}


// -----------------------------------------------------------------
bool File::WriteLine (const char* _zLine)
{
	if (en_fs_open != e_FileStatus)
	{
		cerr << "[ERROR]  Unable to write to file '" << s_FileName 
			 << "' since it has not yet been opened !" << endl;
		return false;
	}
	if (0 == (ios_base::out & e_Mode))
	{
		cerr << "[ERROR]  Unable to write to file '" << s_FileName 
			 << "' since it was not opened in write mode !" << endl;
		return false;
	}

	(*this) << _zLine << endl;
	return true;
}


// -----------------------------------------------------------------
bool File::ReadLines (const char* _zFileName, String_dq_t& _rdqLines)
{
	ifstream file (_zFileName);
	if (false == file.is_open ())
	{
		cerr << "[ERROR]  Failed to open file '" << _zFileName 
			 << "' for reading !" << endl;
		return false;
	}

	_rdqLines.clear ();

	String sLine;
	while (false == file.eof ())
	{
		std::getline (file, sLine);
		if (("" == sLine) && (true == file.eof ()))
			continue;
		_rdqLines.push_back (sLine);
	}

	file.close ();
	return true;
}



// -----------------------------------------------------------------
bool File::ReadLines (const char* _zFileName, String_set_t& _rsetLines)
{
	ifstream file (_zFileName);
	if (false == file.is_open ())
	{
		cerr << "[ERROR]  Failed to open file '" << _zFileName 
			 << "' for reading !" << endl;
		return false;
	}

	_rsetLines.clear ();

	String sLine;
	while (false == file.eof ())
	{
		std::getline (file, sLine);
		if (("" == sLine) && (true == file.eof ()))
			continue;
		sLine.Strip ();
		_rsetLines.insert (sLine);
	}

	file.close ();
	return true;
}


// -----------------------------------------------------------------
bool File::WriteLines (const char* _zFileName, String_dq_t& _rdqLines)
{
	ofstream file (_zFileName);
	if (false == file.is_open ())
	{
		cerr << "[ERROR]  Failed to open file '" << _zFileName 
			 << "' for writing !" << endl;
		return false;
	}

	String_dq_t::iterator	ite = _rdqLines.begin ();
	String_dq_t::iterator	ite_end = _rdqLines.end ();
	for (; ite != ite_end; ++ ite)
		file << *ite << endl;

	file.close ();
	return true;
}


// -----------------------------------------------------------------
bool File::WriteLines (const char* _zFileName, String_set_t& _rsetLines)
{
	ofstream file (_zFileName);
	if (false == file.is_open ())
	{
		cerr << "[ERROR]  Failed to open file '" << _zFileName 
			 << "' for writing !" << endl;
		return false;
	}

	String_set_t::iterator	ite = _rsetLines.begin ();
	String_set_t::iterator	ite_end = _rsetLines.end ();
	for (; ite != ite_end; ++ ite)
		file << *ite << endl;

	file.close ();
	return true;
}


