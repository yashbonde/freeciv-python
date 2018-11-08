#ifndef __NLP_FILE_SYSTEM__
#define __NLP_FILE_SYSTEM__

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ext/stdio_filebuf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nlp_string.h>
#include <deque>
#include <vector>
using namespace std;


typedef vector <double>	double_vec_t;
typedef vector <float>	float_vec_t;
typedef vector <long>	long_vec_t;
typedef vector <int>	int_vec_t;

typedef deque <double>	double_dq_t;
typedef deque <float>	float_dq_t;
typedef deque <long>	long_dq_t;
typedef deque <int>		int_dq_t;


// ------------------------------------------------------
class Path
{
	public:
		static void PrintError (int _iErrNo, const char* _zPath);

		static bool Exists (const char* _zPath);
		static bool Readable (const char* _zPath);
		static bool Writable (const char* _zPath);
		static bool Runable (const char* _zPath);
		static bool Access (const char* _zPath, int _iMode);

		static bool IsDirectory (const char* _zPath);
		static bool IsRegularFile (const char* _zPath);
		static bool IsSymbolicLink (const char* _zPath);
		static mode_t Type (const char* _zPath);
		// Value returned by Type is a combination of the following ...			
		// S_IFMT     0170000   bitmask for the file type bitfields				
		// S_IFSOCK   0140000   socket											
		// S_IFLNK    0120000   symbolic link									
		// S_IFREG    0100000   regular file									
		// S_IFBLK    0060000   block device									
		// S_IFDIR    0040000   directory										
		// S_IFCHR    0020000   character device								
		// S_IFIFO    0010000   FIFO											
		// S_ISUID    0004000   set UID bit										
		// S_ISGID    0002000   set-group-ID bit (see below)					
		// S_ISVTX    0001000   sticky bit (see below)							
		// S_IRWXU    00700     mask for file owner permissions					
		// S_IRUSR    00400     owner has read permission						
		// S_IWUSR    00200     owner has write permission						
		// S_IXUSR    00100     owner has execute permission					
		// S_IRWXG    00070     mask for group permissions						
		// S_IRGRP    00040     group has read permission						
		// S_IWGRP    00020     group has write permission						
		// S_IXGRP    00010     group has execute permission					
		// S_IRWXO    00007     mask for permissions for others (not in group)	
		// S_IROTH    00004     others have read permission						
		// S_IWOTH    00002     others have write permission					
		// S_IXOTH    00001     others have execute permission					

		static bool CreatePath (const char* _zPath, mode_t _iMode);
		static bool RemovePath (const char* _zPath);
		static bool MovePath (const char* _zPath);

		static bool CopyFile (const char* _zOldName, const char* _zNewName);
		static bool MoveFile (const char* _zOldName, const char* _zNewName);
		static bool RemoveFile (const char* _zFileName);

		static bool GetFileList (const char* _zPath, String_dq_t& _rdqFileNames);
		static bool GetPathList (const char* _zPath, String_dq_t& _rdqFileNames);

		static String GetCanonicalPath (const char* _zPath);
};


// ------------------------------------------------------
class File : public fstream
{
	protected:
		enum FileStatus_e
		{
			en_fs_initial,
			en_fs_open,
			en_fs_open_failed,
			en_fs_closed,
			en_fs_destroyed,
			en_fs_format_error
		};

		FILE*							p_File;
		__gnu_cxx::stdio_filebuf<char>*	p_FDBuf;
		String							s_FileName;
		std::ios_base::openmode			e_Mode;
		FileStatus_e					e_FileStatus;

	public:
		File (void);
		File (const char* _zFileName, std::ios_base::openmode _eMode = std::ios_base::in);
		~File (void);

		bool Open (const char* _zFileName, std::ios_base::openmode _eMode = std::ios_base::in);
		void Close (void);
		
		bool ReadLines (String_dq_t& _rdqLines);
		bool ReadLine (String& _rsLine);
		bool ReadLastLine (String& _rsLine);

		bool WriteLines (String_dq_t& _rdqLines);
		bool WriteLine (const char* _zLine);

		static bool ReadLines (const char* _zFileName, String_dq_t& _rdqLines);
		static bool ReadLines (const char* _zFileName, String_set_t& _rsetLines);
		static bool WriteLines (const char* _zFileName, String_dq_t& _rdqLines);
		static bool WriteLines (const char* _zFileName, String_set_t& _rsetLines);
};


// ------------------------------------------------------
enum CsvFileFormat_e
{
	cff_Unknown,
	cff_NonSparseFormat,
	cff_SparseFormat
};


// ------------------------------------------------------
class CsvFile : public File
{
	private:
		CsvFileFormat_e	e_FileFormat;
		unsigned long	ul_LineNumber;
		unsigned long	ul_FirstLineValueCount;

	public:
		CsvFile (void);
		CsvFile (const char* _zFileName, std::ios_base::openmode _eMode = std::ios_base::in);
		~CsvFile (void);

		bool Open (const char* _zFileName, std::ios_base::openmode _eMode = std::ios_base::in);
		bool CheckFormat (void);
		void SetLineValueCount (unsigned long _lCount);
		unsigned long GetFirstLineValueCount (void);

		void SetSparseFileFormat (bool _bSparse);
		bool IsSparseFileFormat (void);
	
		bool ReadLine (double_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLine (float_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLine (long_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLine (int_dq_t& _dqValues, unsigned long* _pulCount);

		bool ReadLine (double* _pValues, unsigned long* _pulCount);
		bool ReadLine (float* _pValues, unsigned long* _pulCount);
		bool ReadLine (long* _pValues, unsigned long* _pulCount);
		bool ReadLine (unsigned long* _pValues, unsigned long* _pulCount);
		bool ReadLine (int* _pValues, unsigned long* _pulCount);
		bool ReadLine (unsigned int* _pValues, unsigned long* _pulCount);
		bool ReadLine (char* _pValues, unsigned long* _pulCount);
		bool ReadLine (unsigned char* _pValues, unsigned long* _pulCount);
	

		bool ReadLastLine (double_vec_t& _vecValues, unsigned long* _pulCount);
		bool ReadLastLine (float_vec_t& _vecValues, unsigned long* _pulCount);
		bool ReadLastLine (long_vec_t& _vecValues, unsigned long* _pulCount);
		bool ReadLastLine (int_vec_t& _vecValues, unsigned long* _pulCount);

		bool ReadLastLine (double_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLastLine (float_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLastLine (long_dq_t& _dqValues, unsigned long* _pulCount);
		bool ReadLastLine (int_dq_t& _dqValues, unsigned long* _pulCount);

		bool ReadLastLine (double* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (float* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (long* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (unsigned long* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (int* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (unsigned int* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (char* _pValues, unsigned long* _pulCount);
		bool ReadLastLine (unsigned char* _pValues, unsigned long* _pulCount);
	
		bool WriteLine (double_vec_t& _vecValues);
		bool WriteLine (float_vec_t& _vecValues);
		bool WriteLine (long_vec_t& _vecValues);
		bool WriteLine (int_vec_t& _vecValues);

		bool WriteLine (double_dq_t& _dqValues);
		bool WriteLine (float_dq_t& _dqValues);
		bool WriteLine (long_dq_t& _dqValues);
		bool WriteLine (int_dq_t& _dqValues);

		bool WriteLine (double* _pValues, unsigned long _ulCount);
		bool WriteLine (float* _pValues, unsigned long _ulCount);
		bool WriteLine (long* _pValues, unsigned long _ulCount);
		bool WriteLine (unsigned long* _pValues, unsigned long _ulCount);
		bool WriteLine (int* _pValues, unsigned long _ulCount);
		bool WriteLine (unsigned int* _pValues, unsigned long _ulCount);
		bool WriteLine (char* _pValues, unsigned long _ulCount);
		bool WriteLine (unsigned char* _pValues, unsigned long _ulCount);
};


#endif
