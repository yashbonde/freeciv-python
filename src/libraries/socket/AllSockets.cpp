#include "nlp_socket.h"
#include <nlp_macros.h>
#include <assert.h>
#include <errno.h>


Socket_set_t			AllSockets::set_AllSockets;
SocketToCallback_map_t	AllSockets::map_SocketToCallback;
ManagementRequest_dq_t	AllSockets::dq_SocketManagementRequest;
pthread_mutex_t			AllSockets::mtx_ExternalSocketManagement = PTHREAD_MUTEX_INITIALIZER;


//															
void AllSockets::Add (Socket* _pSocket)
{
	assert (set_AllSockets.end () == set_AllSockets.find (_pSocket));
	set_AllSockets.insert (_pSocket);
}



//															
void AllSockets::Remove (Socket* _pSocket)
{
	assert (set_AllSockets.end () != set_AllSockets.find (_pSocket));
	set_AllSockets.erase (_pSocket);
}



//															
void AllSockets::CloseAndDeleteAllSockets (void)
{
	ITERATE (Socket_set_t, set_AllSockets, ite)
		(*ite)->Close ();
	set_AllSockets.clear ();
}



//															
void AllSockets::ManageSocket (int _iSocketFd, SocketEventCallback_fn_t _fnCallback)
{
	pthread_mutex_lock (&mtx_ExternalSocketManagement);
	dq_SocketManagementRequest.push_back (SocketManagementRequest (ADD_MANAGED_SOCKET,
																   _iSocketFd,
																   _fnCallback));
	pthread_mutex_unlock (&mtx_ExternalSocketManagement);
}


//															
void AllSockets::StopManagingSocket (int _iSocketFd)
{
	pthread_mutex_lock (&mtx_ExternalSocketManagement);
	dq_SocketManagementRequest.push_back (SocketManagementRequest (REMOVE_MANAGED_SOCKET,
																   _iSocketFd));
	pthread_mutex_unlock (&mtx_ExternalSocketManagement);
}


//															
void AllSockets::StopManagingAllExternalSockets (void)
{
	pthread_mutex_lock (&mtx_ExternalSocketManagement);
	dq_SocketManagementRequest.push_back (SocketManagementRequest (CLEAR_MANAGED_SOCKETS));
	pthread_mutex_unlock (&mtx_ExternalSocketManagement);
}


//															
void AllSockets::ProcessEvents (unsigned long _lMillisecTimeout)
{
	fd_set	fsReadReady;
	fd_set	fsWriteReady;
	fd_set	fsError;

	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);


	int iMaxFD = -1;
	// internal Socket-class sockets ...	
	ITERATE (Socket_set_t, set_AllSockets, ite)
	{
		Socket* pSocket = *ite;
		// standalone sockets are meant for multi-threading	
		// and have to do their own even processing.		
		if (true == pSocket->b_Standalone)
			continue;

		if ((SOCKET_FD_UNDEF == pSocket->fd_Socket) ||
			(SOCKET_FD_CLOSED == pSocket->fd_Socket) ||
			(SOCKET_FD_DESTROYED == pSocket->fd_Socket))
			continue;

		FD_SET (pSocket->fd_Socket, &fsReadReady);
		FD_SET (pSocket->fd_Socket, &fsError);

		// check for write ready only for sockets waiting	
		// to send data ...									
		if (pSocket->o_SendBuffer.Length () > 0)
			FD_SET (pSocket->fd_Socket, &fsWriteReady);

		// checking for write ready on non-blocking sockets	
		// in the process of being connected tell us when	
		// they do get connected ...						
		if (Socket::InProgress == pSocket->e_ConnectionStatus)
			FD_SET (pSocket->fd_Socket, &fsWriteReady);

		if (iMaxFD < pSocket->fd_Socket)
			iMaxFD = pSocket->fd_Socket;
	}


	// external sockets managed by AllSockets
	ITERATE (SocketToCallback_map_t, map_SocketToCallback, ite)
	{
		int fdSocket = ite->first;
		FD_SET (fdSocket, &fsReadReady);
		// FD_SET (fdSocket, &fsWriteReady);
		FD_SET (fdSocket, &fsError);

		if (iMaxFD < fdSocket)
			iMaxFD = fdSocket;
	}


	timeval oTimeout;
	oTimeout.tv_sec = _lMillisecTimeout / 1000;
	oTimeout.tv_usec = (_lMillisecTimeout * 1000) % 1000000;

	select (iMaxFD + 1, &fsReadReady, &fsWriteReady, &fsError, &oTimeout);

	ProcessExternalSockets (fsReadReady, fsWriteReady, fsError);
	ProcessInternalSockets (fsReadReady, fsWriteReady, fsError);

	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);
}


//															
void AllSockets::ProcessInternalSockets (fd_set& _fsRead, fd_set& _fsWrite, fd_set& _fsError)
{
	ITERATE (Socket_set_t, set_AllSockets, ite)
	{
		Socket* pSocket = *ite;
		// standalone sockets are meant for multi-threading	
		// and have to do their own even processing.		
		if (true == pSocket->b_Standalone)
			continue;

		// Note that a socket might become disconnected through	
		// the functionality of inherited classes in response	
		// to the virtual method calls made here. So we need	
		// to check after each call if the socket is still		
		// connected. 											


		// check for sockets that are read-ready ...			
		if ((SOCKET_FD_UNDEF == pSocket->fd_Socket) ||
			(SOCKET_FD_CLOSED == pSocket->fd_Socket) ||
			(SOCKET_FD_DESTROYED == pSocket->fd_Socket))
			continue;

		if (true == FD_ISSET (pSocket->fd_Socket, &_fsRead))
		{
			// OnData is a virtual method overloaded by	
			// ServerSocket to accept incomming socket	
			// connections ...							
			pSocket->OnData ();
		}


		// check for sockets that are write-ready ...			
		if ((SOCKET_FD_UNDEF == pSocket->fd_Socket) ||
			(SOCKET_FD_CLOSED == pSocket->fd_Socket) ||
			(SOCKET_FD_DESTROYED == pSocket->fd_Socket))
			continue;

		if (true == FD_ISSET (pSocket->fd_Socket, &_fsWrite))
		{
			if (Socket::Connected == pSocket->e_ConnectionStatus)
				pSocket->OnCanSend ();

			else if (Socket::InProgress == pSocket->e_ConnectionStatus)
			{
				int iOptionValue = -1;
				socklen_t iOptionLength = sizeof (iOptionLength);
				if (0 != getsockopt (pSocket->fd_Socket, SOL_SOCKET, SO_ERROR, 
									 &iOptionValue, &iOptionLength))
				{
					String sError (strerror (errno));
					cerr << "[ERROR]  getsockopt returned error when checking "
						    "socket connection in progress."
						 << endl;
				}
				if (0 == iOptionValue)
				{
					pSocket->e_ConnectionStatus = Socket::Connected;
					pSocket->OnConnect ();
				}
			}
			else
			{
				cerr << "[ERROR]  Received write-ready notification on socket "
						"that is not connected."
					 << endl;
				abort ();
			}
		}


		// check for sockets that are in error ...				
		if ((SOCKET_FD_UNDEF == pSocket->fd_Socket) ||
			(SOCKET_FD_CLOSED == pSocket->fd_Socket) ||
			(SOCKET_FD_DESTROYED == pSocket->fd_Socket))
			continue;

		if (true == FD_ISSET (pSocket->fd_Socket, &_fsError))
		{
			if ((EINTR != errno) && (EAGAIN != errno) && (EWOULDBLOCK != errno))
			{
				String sError (strerror (errno));
				cerr << "[WARNING]  Socket in error list. " << sError << endl;
				pSocket->OnError ();
			}
		}
	}
}



//															
void AllSockets::ProcessExternalSockets (fd_set& _fsRead, fd_set& _fsWrite, fd_set& _fsError)
{
	// process pending external socket add/remove requests ...	
	pthread_mutex_lock (&mtx_ExternalSocketManagement);
	ITERATE (ManagementRequest_dq_t, dq_SocketManagementRequest, ite)
	{
		if (CLEAR_MANAGED_SOCKETS == ite->c_Type)
			map_SocketToCallback.clear ();
		else if (REMOVE_MANAGED_SOCKET == ite->c_Type)
			map_SocketToCallback.erase (ite->i_Socket);
		else if (ADD_MANAGED_SOCKET == ite->c_Type)
			map_SocketToCallback.insert (make_pair (ite->i_Socket, ite->fn_Callback));
	}
	dq_SocketManagementRequest.clear ();
	pthread_mutex_unlock (&mtx_ExternalSocketManagement);

	
	// process exteral sockets ...								
	ITERATE (SocketToCallback_map_t, map_SocketToCallback, ite)
	{
		int fdSocket = ite->first;
		SocketEventCallback_fn_t fnCallback = ite->second;

		// Note that a socket might become disconnected through	
		// the functionality of inherited classes in response	
		// to the virtual method calls made here. So we need	
		// to check after each call if the socket is still		
		// connected. 											

		// check for sockets that are in error ...				
		if (true == FD_ISSET (fdSocket, &_fsError))
		{
			if ((EINTR != errno) && (EAGAIN != errno) && (EWOULDBLOCK != errno))
			{
				String sError (strerror (errno));
				cerr << "[WARNING]  Socket in error list. " << sError << endl;
				fnCallback (fdSocket, false, false, true);
			}
		}

		// check for read & write ready ...						
		bool bRead = FD_ISSET (fdSocket, &_fsRead);
		bool bWrite = FD_ISSET (fdSocket, &_fsWrite);

		fnCallback (fdSocket, bRead, bWrite, false);
	}
}





