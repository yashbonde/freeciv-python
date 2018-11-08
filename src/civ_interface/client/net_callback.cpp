#include <iostream>
#include <nlp_string.h>
#include <nlp_socket.h>
#include <nlp_macros.h>
#include <signal.h>
#include <stdio.h>
using namespace std;



//													
class IdleCallback
{
	public:
		void (*pfn_Callback)(void*);
		void* p_Data;
};


typedef set<int>    SocketFd_set_t;
typedef list<IdleCallback*> IdleCallback_lst_t;

// extern SocketFd_set_t  set_SocketFd;
extern int i_ServerSocketFd;
extern IdleCallback_lst_t  lst_IdleCallback;
extern void SocketEventCallback (int _fdSocket, bool _bReadReady, bool _bWriteReady, bool _bError);


//													
extern "C" {
	void add_net_input(int sock)
	{
		i_ServerSocketFd = sock;
		AllSockets::ManageSocket (i_ServerSocketFd, SocketEventCallback);
	}

	void remove_net_input(void)
	{
		AllSockets::StopManagingSocket (i_ServerSocketFd);
		i_ServerSocketFd = -1;
	}

	void add_idle_callback (void (callback)(void*), void* _pData)
	{
		IdleCallback* pCallback = new IdleCallback;
		pCallback->pfn_Callback = callback;
		pCallback->p_Data = _pData;
	}
}
