#ifndef __MEMORY_FILE_CACHE__
#define __MEMORY_FILE_CACHE__



void* GetFileInMemoryCache (const char* _zFileName);
void ClearFileInMemoryCache (const char* _zFileName);
char* FileInMemoryCacheGets (void* _pFileCache, char* _zBuffer, int _iSize);
int CheckFileInMemoryCacheError (void* _pFileCache);
bool DataFileNameCacheGet (const char* _zFileName, char** _ppTarget);
void DataFileNameCacheSet (const char* _zFileName, char* _zTarget);
bool DataFileNameCacheGetList (const char* _zPath, 
							   const char* _zInfix,
							   bool _bNoDups,
							   void** _ppList);
void DataFileNameCacheSetList (const char* _zPath, 
							   const char* _zInfix,
							   bool _bNoDups,
							   void* _pList);
bool FileAccessCacheGet (const char* _zFileName, bool* _pAccess);
void FileAccessCacheSet (const char* _zFileName, bool _bAccess);

#endif
