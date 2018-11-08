#include "message.h"
#include "mem.h"
#include <string.h>
#include <assert.h>


//												
struct Message* construct_message (void)
{
	struct Message* pMessage = fc_malloc (sizeof (struct Message));
	pMessage->z_Data = NULL;
	pMessage->l_Bytes = 0;
};


//												
void append_to_message (struct Message* _pMessage, const char* _zData)
{
	long lExtension = strlen (_zData);
	_pMessage->z_Data = fc_realloc (_pMessage->z_Data, _pMessage->l_Bytes + lExtension);
	memcpy ((char*)_pMessage->z_Data + _pMessage->l_Bytes, _zData, lExtension);
	_pMessage->l_Bytes += lExtension;
}


//												
void clear_message (struct Message* _pMessage)
{
	_pMessage->l_Bytes = 0;
	if (NULL == _pMessage->z_Data)
		return;
	free (_pMessage->z_Data);
	_pMessage->z_Data = NULL;
}


//												
void destroy_message (struct Message* _pMessage)
{
	if (NULL == _pMessage)
		return;

	#ifndef NDEBUG
	assert ((void*)0x1 != _pMessage->z_Data);
	#endif
	
	if (NULL != _pMessage->z_Data)
		free (_pMessage->z_Data);

	#ifndef NDEBUG
	_pMessage->z_Data = (void*)0x1;
	_pMessage->l_Bytes = 0;
	#endif

	free (_pMessage);
};


