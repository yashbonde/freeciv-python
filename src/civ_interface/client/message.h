#ifndef __CIV_INTERFACE_BUFFER__
#define __CIV_INTERFACE_BUFFER__


struct Message
{
	void* 			z_Data;
	unsigned long	l_Bytes;
};


struct Message* construct_message (void);
void append_to_message (struct Message* _pMessage, const char* _zData);
void clear_message (struct Message* _pMessage);
void destroy_message (struct Message* _pMessage);


#endif
