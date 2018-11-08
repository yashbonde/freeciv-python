#include <nlp_string.h>
#include <nlp_filesystem.h>
#include <assert.h>
#include <fstream>
#include <map>
using namespace std;


extern "C"
{
#define SPECLIST_TAG datafile
#define SPECLIST_TYPE struct datafile
#include "speclist.h"
}



class FileCache;
typedef map <String, FileCache*>		FileNameToCache_map_t;
typedef map <String, String>			FileNameToTarget_map_t;
typedef map <String, bool>				FileNameToAccess_map_t;
typedef map <String, datafile_list*>	FileNameToFileList_map_t;

FileNameToCache_map_t		map_FileNameToCache;
FileNameToTarget_map_t		map_FileNameToTarget;
FileNameToAccess_map_t		map_FileNameToAccess;
FileNameToFileList_map_t	map_FileNameToFileList;

class FileCache
{
	private:
		char*	p_Buffer;
		size_t	i_FileSize;
		char*	p_EOF;
		char*	p_Current;

	public:
		FileCache (void);
		~FileCache (void);

		bool LoadFile (const char* _zFileName);
		void Reset (void);
		char* Gets (char* _zBuffer, int _iSize);
};


//										
FileCache::FileCache (void)
{
	p_Buffer = NULL;
	p_EOF = NULL;
	p_Current = NULL;
	i_FileSize = 0;
}

FileCache::~FileCache (void)
{
	if (NULL != p_Buffer)
		delete[] p_Buffer;
	p_Buffer = (char*)0x1;

	p_EOF = NULL;
	p_Current = NULL;
	i_FileSize = 0;
}


//										
void FileCache::Reset (void)
{
	p_Current = p_Buffer;
	p_EOF = p_Buffer + i_FileSize;
}


//										
bool FileCache::LoadFile (const char* _zFileName)
{
	File file;
	if (false == file.Open (_zFileName))
		return false;
	file.seekg (0, ios_base::end);
	size_t iSize = file.tellg ();
	file.seekg (0, ios_base::beg);

	p_Buffer = new char [iSize + 20];
	file.read (p_Buffer, iSize + 10);
	i_FileSize = file.gcount ();
	assert (i_FileSize == iSize);
	bool bRet (false == file.bad ());

	p_Current = p_Buffer;
	p_EOF = p_Buffer + i_FileSize;

	file.Close ();
	return bRet;
}


//										
char* FileCache::Gets (char* _zBuffer, int _iSize)
{
	if (p_Current >= p_EOF)
		return NULL;
	if (0 == _iSize)
	{
		_zBuffer [0] = '\0';
		return _zBuffer;
	}

	char* pEnd = p_Current;
	while (pEnd < p_EOF)
	{
		if ('\n' == *pEnd)
		{
			++ pEnd;
			break;
		}
		++ pEnd;
	}
	size_t iRemainder = (pEnd - p_Current);
	size_t iRead = ((size_t)_iSize - 1 <= iRemainder)? _iSize - 1: iRemainder;

	memcpy (_zBuffer, p_Current, iRead);
	_zBuffer [iRead] = '\0';
	p_Current += iRead;

	return _zBuffer;
}



extern "C"
{
#include "memory_file_cache.h"

//										
void* GetFileInMemoryCache (const char* _zFileName)
{
	FileNameToCache_map_t::iterator	ite;
	ite = map_FileNameToCache.find (_zFileName);
	if (map_FileNameToCache.end () != ite)
	{
		// cout << "File cache hit : " << _zFileName << endl;
		FileCache* pCache = ite->second;
		pCache->Reset ();
		return pCache;
	}

	FileCache* pCache = new FileCache;
	if (false == pCache->LoadFile (_zFileName))
	{
		delete pCache;
		pCache = NULL;
	}

	// cout << "File cache load : " << _zFileName << endl;
	map_FileNameToCache.insert (make_pair (_zFileName, pCache));
	return pCache;
}


//										
void ClearFileInMemoryCache (const char* _zFileName)
{
	// cout << "File cache clear : " << _zFileName << endl;

	FileNameToCache_map_t::iterator	ite;
	ite = map_FileNameToCache.find (_zFileName);
	if (map_FileNameToCache.end () == ite)
		return;

	delete ite->second;
	map_FileNameToCache.erase (ite);
}


//										
char* FileInMemoryCacheGets (void* _pFileCache, char* _zBuffer, int _iSize)
{
	FileCache* pCache = (FileCache*)_pFileCache;
	return pCache->Gets (_zBuffer, _iSize);
}


//										
int CheckFileInMemoryCacheError (void* _pFileCache)
{
	return ((NULL == _pFileCache)? 1 : 0);
}


//										
bool DataFileNameCacheGet (const char* _zFileName, char** _ppTarget)
{
	FileNameToTarget_map_t::iterator	ite;
	ite = map_FileNameToTarget.find (_zFileName);
	if (map_FileNameToTarget.end () == ite)
		return false;

	*_ppTarget = (char*)(const char*)ite->second;
	return true;
}


//										
void DataFileNameCacheSet (const char* _zFileName, char* _zTarget)
{
	map_FileNameToTarget.insert (make_pair (_zFileName, _zTarget));
}


//										
bool DataFileNameCacheGetList (const char* _zPath, 
							   const char* _zInfix,
							   bool _bNoDups,
							   void** _ppList)
{
	char zName [1000];
	sprintf (zName, "%s\x01%s\x01%d", _zPath, _zInfix, _bNoDups);

	FileNameToFileList_map_t::iterator	ite;
	ite = map_FileNameToFileList.find (zName);
	if (map_FileNameToFileList.end () == ite)
		return false;

	datafile_list** ppList = (datafile_list**)_ppList;
	*ppList = datafile_list_copy (ite->second);
	return true;
}


//										
void DataFileNameCacheSetList (const char* _zPath, 
							   const char* _zInfix,
							   bool _bNoDups,
							   void* _pList)
{
	char zName [1000];
	sprintf (zName, "%s\x01%s\x01%d", _zPath, _zInfix, _bNoDups);
	datafile_list* pLocalList = datafile_list_copy ((datafile_list*)_pList);
	map_FileNameToFileList.insert (make_pair (zName, pLocalList));
}


//										
bool FileAccessCacheGet (const char* _zFileName, bool* _pAccess)
{
	FileNameToAccess_map_t::iterator	ite;
	ite = map_FileNameToAccess.find (_zFileName);
	if (map_FileNameToAccess.end () == ite)
		return false;

	*_pAccess = ite->second;
	return true;
}


//										
void FileAccessCacheSet (const char* _zFileName, bool _bAccess)
{
	map_FileNameToAccess.insert (make_pair (_zFileName, _bAccess));
}




}

