#include "nlp_filesystem.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
using namespace std;


// --------------------------------------------------------------------
bool Path::Exists (const char* _zPath)
{
	return Access (_zPath, F_OK);
}

bool Path::Readable (const char* _zPath)
{
	return Access (_zPath, R_OK);
}

bool Path::Writable (const char* _zPath)
{
	return Access (_zPath, W_OK);
}

bool Path::Runable (const char* _zPath)
{
	return Access (_zPath, X_OK);
}

// --------------------------------------------------------------------
bool Path::Access (const char* _zPath, int _iMode)
{
	if (0 == access (_zPath, _iMode))
		return true;

	// if we're checking for the existance of a path, 
	// ENOENT is not an error. Otherwise print error. 
	if ((ENOENT == errno) && (F_OK == _iMode))
		return false;
	if (EINVAL == errno)
	{
		cerr << "[ERROR]  Invalid mode parameter specified to Path::Access. Mode = "
			 << _iMode << "." << endl;
		return false;
	}

	Path::PrintError (errno, _zPath);
	return false;
}



// --------------------------------------------------------------------
bool Path::IsDirectory (const char* _zPath)
{
	mode_t mode = Path::Type (_zPath);
	if (0 != (S_IFDIR & mode))
		return true;
	return false;
}


bool Path::IsRegularFile (const char* _zPath)
{
	mode_t mode = Path::Type (_zPath);
	if (0 != (S_IFREG & mode))
		return true;
	return false;
}


bool Path::IsSymbolicLink (const char* _zPath)
{
	mode_t mode = Path::Type (_zPath);
	if (0 != (S_IFLNK & mode))
		return true;
	return false;
}


mode_t Path::Type (const char* _zPath)
{
	struct stat buffer;
	if (0 == stat (_zPath, &buffer))
		return buffer.st_mode;

	Path::PrintError (errno, _zPath);
	return 0;
}



// --------------------------------------------------------------------
bool Path::CreatePath (const char* _zPath, mode_t _iMode)
{
	// if (0 == access (_zPath, F_OK))
		// return true;
	if (0 == mkdir (_zPath, _iMode))
		return true;

	int iErrorNo = errno;
	cerr << "[ERROR]  Failed to create path " << _zPath 
		 << " . Error : " << strerror (iErrorNo) << endl;
	return false;
}


// --------------------------------------------------------------------
bool Path::RemovePath (const char* _zPath)
{
	// if path doesn't exist, we don't need to do anything ...
	if (0 != access (_zPath, F_OK))
		if (ENOENT == errno)
			return true;

	// delete path ...
	if (0 == rmdir (_zPath))
		return true;

	int iErrorNo = errno;
	cerr << "[ERROR]  Failed to delete path " << _zPath 
		 << " . Error : " << strerror (iErrorNo) << endl;
	return false;
}


// --------------------------------------------------------------------
bool Path::MovePath (const char* _zPath)
{
	return false;
}


// --------------------------------------------------------------------
bool Path::CopyFile (const char* _zOldName, const char* _zNewName)
{
	if (true == Exists (_zNewName))
	{
		cerr << "[ERROR]  Cannot copy file. A file with the name ["
			 << _zNewName << "] already exists. Delete it first."
			 << endl;
		return false;
	}

	String sCommand;
	sCommand << "cp -f " << _zOldName << ' ' << _zNewName;
	if (-1 != system (sCommand))
		return true;

	cerr << "[ERROR]  File move from [" << _zOldName 
		 << "] to [" << _zNewName << "] failed." << endl;
	return false;
}


// --------------------------------------------------------------------
bool Path::MoveFile (const char* _zOldName, const char* _zNewName)
{
	if (true == Exists (_zNewName))
	{
		cerr << "[ERROR]  Cannot move file. A file with the name ["
			 << _zNewName << "] already exists. Delete it first."
			 << endl;
		return false;
	}
	if (0 == rename (_zOldName, _zNewName))
		return true;
	if (EXDEV == errno)
	{
		String sCommand;
		sCommand << "mv -f " << _zOldName << ' ' << _zNewName;
		if (-1 != system (sCommand))
			return true;
	}

	int iErrorNo = errno;
	cerr << "[ERROR]  File move from [" << _zOldName 
		 << "] to [" << _zNewName << "] failed.  Error : "
		 << strerror (iErrorNo) << endl;
	return false;
}


// --------------------------------------------------------------------
bool Path::RemoveFile (const char* _zFileName)
{
	if (0 == unlink (_zFileName))
		return true;

	int iErrorNo = errno;
	cerr << "[ERROR]  Failed to delete file " << _zFileName 
		 << " . Error : " << strerror (iErrorNo) << endl;
	return false;
}


// --------------------------------------------------------------------
bool Path::GetFileList (const char* _zPath, String_dq_t& _rdqFileNames)
{
	DIR* pDirectory = opendir (_zPath);
	if (NULL == pDirectory)
	{
		Path::PrintError (errno, _zPath);
		return false;
	}

	dirent* pFile = readdir (pDirectory);
	if (NULL == pFile)
		Path::PrintError (errno, _zPath);

	while (NULL != pFile)
	{
		if ((0 != strcmp (".", pFile->d_name)) &&
			(0 != strcmp ("..", pFile->d_name)))
			_rdqFileNames.push_back (pFile->d_name);

		pFile = readdir (pDirectory);
	}
	closedir (pDirectory);

	return true;
}


// --------------------------------------------------------------------
bool Path::GetPathList (const char* _zPath, String_dq_t& _rdqFileNames)
{
	DIR* pDirectory = opendir (_zPath);
	if (NULL == pDirectory)
	{
		return false;
		Path::PrintError (errno, _zPath);
	}

	dirent* pFile = readdir (pDirectory);
	if (NULL == pFile)
		Path::PrintError (errno, _zPath);

	while (NULL != pFile)
	{
		if ((0 != strcmp (".", pFile->d_name)) &&
			(0 != strcmp ("..", pFile->d_name)))
		{
			String sPath;
			if ('/' == _zPath [strlen (_zPath) - 1])
				sPath << _zPath << pFile->d_name;
			else
				sPath << _zPath << '/' << pFile->d_name;
			_rdqFileNames.push_back (sPath);
		}

		pFile = readdir (pDirectory);
	}
	closedir (pDirectory);

	return true;
}


// --------------------------------------------------------------------
String Path::GetCanonicalPath (const char* _zPath)
{
	long lBufferLength = 0;
	long lPathLength = 0;
	char* pCanonicalPath = NULL;
	do
	{
		lBufferLength += 1000;
		pCanonicalPath = (char*) realloc (pCanonicalPath, lBufferLength);
		lPathLength = readlink (_zPath, pCanonicalPath, lBufferLength);
	}
	while ((lPathLength == lBufferLength) && (-1 != lPathLength));
	
	if (-1 == lPathLength)
	{
		int iError = errno;
		free (pCanonicalPath);

		// if the path doesn't have any symbolic links in it, readlink returns EINVAL! 
		if (EINVAL != iError)
			cerr << "[ERROR]  Canonicalization failed for path " << _zPath 
				 << " , Error : " << strerror (iError) << endl;
		return _zPath;
	}

	pCanonicalPath [lPathLength] = '\0';
	String sCanonicalPath (pCanonicalPath);
	free (pCanonicalPath);

	return sCanonicalPath;
}


// --------------------------------------------------------------------
void Path::PrintError (int _iErrNo, const char* _zPath)
{
	if (EACCES == _iErrNo)
		cerr << "[ERROR]  Access denied to path '" << _zPath 
			 << "'." << endl;
	else if (ELOOP == _iErrNo)
		cerr << "[ERROR]  Too many symbolic links were encountered in resolving path '"
			 << _zPath << "'." << endl;
	else if (ENAMETOOLONG == _iErrNo)
		cerr << "[ERROR]  Path name too long : '" << _zPath << "'." << endl;
	else if (ENOTDIR == _iErrNo)
		cerr << "[ERROR]  A component used as a directory in the path is not "
				"in fact a directory. Path : '" << _zPath << "'." << endl;
	else if (ENOENT == _iErrNo)
		cerr << "[ERROR]  Path does not exist, or is a dangling link. Path : '"
			 << _zPath << "'." << endl;
	else if (EFAULT == _iErrNo)
		cerr << "[ERROR]  Path points outside process' accessible address space."
			 << endl;
	else if (EIO == _iErrNo)
		cerr << "[ERROR]  An I/O error occurred while checking access to path '"
		 	 << _zPath << "'." << endl;
	else if (ENOMEM == _iErrNo)
		cerr << "[ERROR]  Insufficient kernel memory while checking access to path '"
			 << _zPath << "'." << endl;
	else if (EMFILE == _iErrNo)
		cerr << "[ERROR]  Failed to access path '" << _zPath
			 << "', too many file descriptors in use by process." << endl;
	else if (ENFILE == _iErrNo)
		cerr << "[ERROR]  Failed to access path '" << _zPath
			 << "', too many files are currently open in the system." << endl;
	else if (EBADF == _iErrNo)
		cerr << "[ERROR]  Failed to access path '" << _zPath
			 << "', bad address." << endl;
	else
		cerr << "[ERROR]  Failed to open path '" << _zPath
			 << "', unknown error." << endl;
}
