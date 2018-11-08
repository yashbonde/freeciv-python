#include "nlp_socket.h"
#include <nlp_time.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>



//																
Socket::Socket (void)
{
	fd_Socket = SOCKET_FD_UNDEF;
	e_ConnectionStatus = NotConnected;
	e_BlockingType = Undefined;
	b_Standalone = false;
	b_UnixDomain = false;
	l_CurrentTimeout = 0;

	AllSockets::Add (this);
}




//																
Socket::~Socket (void)
{
	assert (SOCKET_FD_DESTROYED != fd_Socket);

	// We need to remove the socket from the global list	
	// before closing it to avoid race conditions ...		
	AllSockets::Remove (this);

	if ((SOCKET_FD_CLOSED == fd_Socket) || (SOCKET_FD_UNDEF == fd_Socket))
		return;

	// close the socket ...									
	if (-1 == shutdown (fd_Socket, SHUT_RDWR))
	{
		if (ENOTCONN != errno)
		{
			String sError (strerror (errno));
			cerr << "[ERROR] Failed to shutdown socket [" 
				 << s_Server << ':' << s_Service << "]. "
				 << sError << endl;
		}
	}
	close (fd_Socket);

	e_ConnectionStatus = NotConnected;
	fd_Socket = SOCKET_FD_DESTROYED;
}




//																
void Socket::SetStandalone (bool _bStandalone)
{
	b_Standalone = _bStandalone;
}



//																
bool Socket::SetNoDelay (bool _bNoDelay)
{
	if (true == b_UnixDomain)
		return false;

	int iNoDelayFlag = 1;
	if (false == _bNoDelay)
		iNoDelayFlag = 0;

	return SetSocketOptions (IPPROTO_TCP, 
							 TCP_NODELAY, 
							 (char *)&iNoDelayFlag, 
							 sizeof(iNoDelayFlag));
}



//																
bool Socket::SetSocketOptions (int _iLevel, 
								int _iOptionName, 
								const void* _pOptionValue, 
								socklen_t _iOptionLength)
{
	if (SOCKET_FD_UNDEF == fd_Socket)
	{
		cerr << "[ERROR] SetSocketOptions called on uninitilized socket."
			 << endl;
		return false;
	}

	int ret = setsockopt (fd_Socket, 
						  _iLevel, 
						  _iOptionName,
						  _pOptionValue, 
						  _iOptionLength);
	if (0 == ret)
		return true;

	String sError (strerror (errno));
	cerr << "[ERROR] Failed to set socket options in call Socket::SetSocketOptions. " << endl
		 << "        socket [" << s_Server << ':' << s_Service << "]. " << endl
		 << "        " << sError << endl;
	return false;
}



//																
bool Socket::IsConnected (void)
{
	return (Connected == e_ConnectionStatus);
}



//																
bool Socket::Close (void)
{
	if (SOCKET_FD_DESTROYED == fd_Socket)
	{
		cerr << "[ERROR] Calling Close () on a deleted socket." << endl;
		return true;
	}
	if ((SOCKET_FD_CLOSED == fd_Socket) || (SOCKET_FD_UNDEF == fd_Socket))
		return true;

	if (-1 == shutdown (fd_Socket, SHUT_RDWR))
	{
		if (ENOTCONN == errno)
			return true;
		String sError (strerror (errno));
		cerr << "[ERROR] Failed to shutdown socket [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return false;
	}

	close (fd_Socket);
	e_ConnectionStatus = NotConnected;
	fd_Socket = SOCKET_FD_CLOSED;
	return true;
}



//																
#define BUFFER_SIZE		81920
void Socket::OnData (void)
{
	if (Undefined == e_BlockingType)
	{
		cerr << "[ERROR] Socket::OnData called on a socket of undefined blocking type." << endl;
		abort ();
	}
	if (Connected != e_ConnectionStatus)
		return;
	if ((SOCKET_FD_CLOSED == fd_Socket) || (SOCKET_FD_DESTROYED == fd_Socket))
		return;

	char* zBuffer [BUFFER_SIZE + 1];
	if (NonBlocking == e_BlockingType)
	{
		ssize_t lBytesReceived = recv (fd_Socket, zBuffer, BUFFER_SIZE, MSG_DONTWAIT);
		while (0 < lBytesReceived)
		{
			o_ReceiveBuffer.Append (zBuffer, (unsigned long)lBytesReceived);
			lBytesReceived = recv (fd_Socket, zBuffer, BUFFER_SIZE, MSG_DONTWAIT);
		}
		if ((-1 == lBytesReceived) && (EAGAIN != errno) && (EWOULDBLOCK != errno))
		{
			String sError (strerror (errno));
			cerr << "[ERROR] Error in Socket::OnData during recv on non-blocking socket.\n"
					"        " << sError << endl;
			OnError ();
		}


		// The check below for 0 bytes is not incorrect.				
		// the recv call above will return 0 *only* in the case where	
		// the peer has performed an orderly shutdown, so we should		
		// close the port.  Under any other condition, recv returns -1	
		// and errno is set to EAGAIN if there is no data present.		
		if (0 == lBytesReceived)
		{
			if (NotConnected != e_ConnectionStatus)
			{
				e_ConnectionStatus = NotConnected;
				Close ();
				OnDisconnect ();
				return;
			}
		}

		// The overloaded functionality of the virtual methods called above	
		// may result in this socket being destroyed before we get here,	
		// So we need to double check ...									
		if ((SOCKET_FD_CLOSED != fd_Socket) && (SOCKET_FD_DESTROYED != fd_Socket))
		{
			// In some cases (I'm not entirely sure of the specifics)	
			// OnData seems to get called even when there is no data	
			// to be received. This triggering is not the same as a 	
			// peer disconnect which is specifically handled above.		
			// I.e., OnData gets called, the peer is still connected,	
			// but recv returns a EAGAIN or EWOULDBLOCK.  In this case	
			// we should return from this point since the remainder of	
			// this method would consider this case as an error, and	
			// disconnect the socket!...								
			if (0 == o_ReceiveBuffer.Length ())
				return;
		}
	}
	else
	{
		while (1 == recv (fd_Socket, zBuffer, 1, MSG_DONTWAIT | MSG_PEEK))
		{
			ssize_t lBytesReceived = recv (fd_Socket, zBuffer, BUFFER_SIZE, 0);
			o_ReceiveBuffer.Append (zBuffer, (unsigned long)lBytesReceived);
		}
	}

	// The overloaded functionality of the virtual methods called above	
	// may result in this socket being destroyed before we get here,	
	// So we need to double check ...									
	if ((SOCKET_FD_CLOSED == fd_Socket) || (SOCKET_FD_DESTROYED == fd_Socket))
		return;

	unsigned long lBytes = o_ReceiveBuffer.Length ();
	if (0 != lBytes)
		OnReceive (o_ReceiveBuffer.GetData (), lBytes);

	else 
	{
		if (NotConnected != e_ConnectionStatus)
		{
			e_ConnectionStatus = NotConnected;
			Close ();
			OnDisconnect ();
		}
	}

	// The overloaded functionality of the virtual methods called above	
	// may result in this socket being destroyed before we get here,	
	// So we need to double check ...									
	if ((SOCKET_FD_CLOSED != fd_Socket) && (SOCKET_FD_DESTROYED != fd_Socket))
		o_ReceiveBuffer.Clear ();
}



//																
void Socket::OnError (void)
{
	cerr << "[ERROR]  Socket to [" << s_Server << ':' << s_Service
		 << "] in error list" << endl;
}



//																
long Socket::SendBlocking (const void* _zData, long _lBytes)
{
	if (SOCKET_FD_UNDEF == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a socket not yet created."
			 << endl;
		return -1;
	}
	if (SOCKET_FD_CLOSED == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a closed socket."
			 << endl;
		return -1;
	}
	if (SOCKET_FD_DESTROYED == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a deleted socket."
			 << endl;
		abort ();
		return -1;
	}

	// if (_lBytes < 50)
	//	cout << "Socket::SendBlocking (" << _lBytes << ", " << (const char*)_zData << ')' << endl;
	// else
	//	cout << "Socket::SendBlocking (" << _lBytes << ')' << endl;

	ssize_t lSentBytes = send (fd_Socket, _zData, _lBytes, 0);
	if (-1 == lSentBytes)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Error sending data on socket ["
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
	}

	// this can be changed to true/false if the send call	
	// blocks until the peer has received all sent data,	
	// and doesn't return with half the data sent.			
	return lSentBytes;
}



//																
bool Socket::SendNonBlocking (const void* _zData, long _lBytes)
{
	if (SOCKET_FD_UNDEF == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a socket not yet created."
			 << endl;
		return false;
	}
	if (SOCKET_FD_CLOSED == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a closed socket."
			 << endl;
		return false;
	}
	if (SOCKET_FD_DESTROYED == fd_Socket)
	{
		cerr << "[ERROR]  calling Socket::Send on a deleted socket."
			 << endl;
		abort ();
		return false;
	}

	// if (_lBytes < 50)
	//	cout << "Socket::SendNonBlocking (" << _lBytes << ", " << (const char*)_zData << ')' << endl;
	// else
	//	cout << "Socket::SendNonBlocking (" << _lBytes << ')' << endl;

	o_SendBuffer.Append (_zData, _lBytes);

	// what does it mean to do do error checking here?		
	return true;
}



//																
long Socket::ReceiveBlocking (const void* _zData, long _lBytes, long _lMillisecTimeout)
{
	if (false == b_Standalone)
	{
		cerr << "[WARNING] Calling Socket::ReceiveBlocking on a non-standalone socket.\n"
				"          This can cause race conditions with OnData in a threaded environment."
			 << endl;
	}

	// set timeout ...	
	if (l_CurrentTimeout != _lMillisecTimeout)
	{
		l_CurrentTimeout = _lMillisecTimeout;

		timeval oTimeout;
		// setting the timeout value to 0, disables the timeout.
		// i.e. recv will then block forever.					
		memset (&oTimeout, 0, sizeof (oTimeout));
		if (_lMillisecTimeout > 0)
		{
			oTimeout.tv_sec = _lMillisecTimeout / 1000;
			oTimeout.tv_usec = (_lMillisecTimeout * 1000) % 1000000;
		}
		setsockopt (fd_Socket, SOL_SOCKET, SO_RCVTIMEO, (timeval*)&oTimeout, sizeof (oTimeout));
	}


	// receive data ...	
	ssize_t lBytesReceived = recv (fd_Socket, (void*)_zData, _lBytes, 0);
	if (-1 == lBytesReceived)
	{
		// If a timeout was set, and we get here, then the recv 
		// call timed out.  So we shouldn't report an error...	
		if (_lMillisecTimeout > 0)
		{
			// If the recv call timed out, errno is supposed	
			// to get set to EAGAIN or EWOULDBLOCK ...			
			// EINTR check was retrofitted since recv was 		
			// returning that in some cases...					
			if ((EAGAIN != errno) && (EWOULDBLOCK != errno) && (EINTR != errno))
			{
				String sError (strerror (errno));
				cerr << "[ERROR] recv with a timeout set returned -1, "
						"but errno was not the expected value ["
					 << s_Server << ':' << s_Service << "]. "
					 << sError << endl;
			}
		}

		// If no timeout was set, we have some error!			
		else
		{
			String sError (strerror (errno));
			cerr << "[ERROR] Error receiving data on socket ["
				 << s_Server << ':' << s_Service << "]. "
				 << sError << endl;
		}
	}
	if (0 == lBytesReceived)
	{
		if (NotConnected != e_ConnectionStatus)
		{
			e_ConnectionStatus = NotConnected;
			Close ();
			OnDisconnect ();
		}
	}

	return lBytesReceived;
}



//																
void Socket::OnCanSend (void)
{
	ssize_t lSentBytes = send (fd_Socket, 
								o_SendBuffer.GetData (), 
								o_SendBuffer.Length (),
								MSG_DONTWAIT);
	if (lSentBytes > 0)
		o_SendBuffer.DropFront (lSentBytes);

	if ((-1 == lSentBytes) && (EAGAIN != errno) && (EWOULDBLOCK != errno))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Error sending data on socket ["
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
	}
}



//															
void Socket::ProcessEvents (unsigned long _lMillisecTimeout)
{
	if (false == b_Standalone)
	{
		cerr << "[ERROR]  Calling Socket::ProcessEvent directly on a non-standalone socket!"
			 << endl;
		return;
	}

	fd_set	fsReadReady;
	fd_set	fsWriteReady;
	fd_set	fsError;

	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);


	if ((SOCKET_FD_UNDEF == fd_Socket) ||
		(SOCKET_FD_CLOSED == fd_Socket) ||
		(SOCKET_FD_DESTROYED == fd_Socket))
	{
		usleep (_lMillisecTimeout * 1000);
		return;
	}

	if ((Socket::InProgress != e_ConnectionStatus) ||
		(NonBlocking != e_BlockingType))
		FD_SET (fd_Socket, &fsReadReady);
	FD_SET (fd_Socket, &fsError);

	// check for write ready only for sockets waiting	
	// to send data ...									
	if (o_SendBuffer.Length () > 0)
		FD_SET (fd_Socket, &fsWriteReady);

	// checking for write ready on non-blocking sockets	
	// in the process of being connected tell us when	
	// they do get connected ...						
	if (Socket::InProgress == e_ConnectionStatus)
		FD_SET (fd_Socket, &fsWriteReady);


	timeval oTimeout;
	oTimeout.tv_sec = _lMillisecTimeout / 1000;
	oTimeout.tv_usec = (_lMillisecTimeout * 1000) % 1000000;

	select (fd_Socket + 1, &fsReadReady, &fsWriteReady, &fsError, &oTimeout);


	// Note that a socket might become disconnected through	
	// the functionality of inherited classes in response	
	// to the virtual method calls made here. So we need	
	// to check after each call if the socket is still		
	// connected. 											


	// check for sockets that are read-ready ...			
	if ((SOCKET_FD_UNDEF == fd_Socket) ||
		(SOCKET_FD_CLOSED == fd_Socket) ||
		(SOCKET_FD_DESTROYED == fd_Socket))
		return;

	if (true == FD_ISSET (fd_Socket, &fsReadReady))
	{
		// OnData is a virtual method overloaded by	
		// ServerSocket to accept incomming socket	
		// connections ...							
		OnData ();
	}


	// check for sockets that are write-ready ...			
	if ((SOCKET_FD_UNDEF == fd_Socket) ||
		(SOCKET_FD_CLOSED == fd_Socket) ||
		(SOCKET_FD_DESTROYED == fd_Socket))
		return;

	if (true == FD_ISSET (fd_Socket, &fsWriteReady))
	{
		if (Socket::Connected == e_ConnectionStatus)
			OnCanSend ();

		else if (Socket::InProgress == e_ConnectionStatus)
		{
			int iOptionValue = -1;
			socklen_t iOptionLength = sizeof (iOptionLength);
			if (0 != getsockopt (fd_Socket, SOL_SOCKET, SO_ERROR, 
								 &iOptionValue, &iOptionLength))
			{
				String sError (strerror (errno));
				cerr << "[ERROR]  getsockopt returned error when checking "
						"socket connection in progress: " << sError
					 << endl;
			}
			if (0 == iOptionValue)
			{
				e_ConnectionStatus = Socket::Connected;
				OnConnect ();
			}
			else
			{
				String sError (strerror (iOptionValue));
				cerr << "[ERROR] Error on socket in connection progress: " << sError
					 << endl;
			}
		}
		else
		{
			cerr << "[WARNING]  Received write-ready notification on socket "
					"that is not connected."
				 << endl;
			OnError ();
			// abort ();
		}
	}


	// check for sockets that are in error ...				
	if ((SOCKET_FD_UNDEF == fd_Socket) ||
		(SOCKET_FD_CLOSED == fd_Socket) ||
		(SOCKET_FD_DESTROYED == fd_Socket))
		return;

	if (true == FD_ISSET (fd_Socket, &fsError))
	{
		if (EINTR != errno)
		{
			String sError (strerror (errno));
			cerr << "[WARNING]  Socket in error list. " << sError << endl;
			OnError ();
		}
	}


	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);
}



//															
void Socket::ProcessEvents (int _fdSocket, 
							unsigned long _lMillisecTimeout,
							SocketEventCallback_fn_t _fnCallback)
{
	fd_set	fsReadReady;
	fd_set	fsWriteReady;
	fd_set	fsError;

	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);


	if ((SOCKET_FD_UNDEF == _fdSocket) ||
		(SOCKET_FD_CLOSED == _fdSocket) ||
		(SOCKET_FD_DESTROYED == _fdSocket))
	{
		usleep (_lMillisecTimeout * 1000);
		return;
	}

	FD_SET (_fdSocket, &fsReadReady);
	FD_SET (_fdSocket, &fsError);

	// check for write ready only for sockets waiting	
	// to send data ...									
	// if (o_SendBuffer.Length () > 0)
	//	FD_SET (fd_Socket, &fsWriteReady);

	// checking for write ready on non-blocking sockets	
	// in the process of being connected tell us when	
	// they do get connected ...						
	// if (Socket::InProgress == e_ConnectionStatus)
	//	FD_SET (fd_Socket, &fsWriteReady);


	timeval oTimeout;
	oTimeout.tv_sec = _lMillisecTimeout / 1000;
	oTimeout.tv_usec = (_lMillisecTimeout * 1000) % 1000000;

	select (_fdSocket + 1, &fsReadReady, &fsWriteReady, &fsError, &oTimeout);


	// Note that a socket might become disconnected through	
	// the functionality of inherited classes in response	
	// to the virtual method calls made here. So we need	
	// to check after each call if the socket is still		
	// connected. 											


	// check for sockets that are read-ready ...			
	if ((SOCKET_FD_UNDEF == _fdSocket) ||
		(SOCKET_FD_CLOSED == _fdSocket) ||
		(SOCKET_FD_DESTROYED == _fdSocket))
		return;

	if (true == FD_ISSET (_fdSocket, &fsReadReady))
	{
		// OnData is a virtual method overloaded by	
		// ServerSocket to accept incomming socket	
		// connections ...							
		_fnCallback (_fdSocket, true, false, false);
	}


	// check for sockets that are write-ready ...			
	if ((SOCKET_FD_UNDEF == _fdSocket) ||
		(SOCKET_FD_CLOSED == _fdSocket) ||
		(SOCKET_FD_DESTROYED == _fdSocket))
		return;

	if (true == FD_ISSET (_fdSocket, &fsWriteReady))
	{
		_fnCallback (_fdSocket, false, true, false);
/*
		if (Socket::Connected == e_ConnectionStatus)
			_fnCallback (_fdSocket);

		else if (Socket::InProgress == e_ConnectionStatus)
		{
			int iOptionValue = -1;
			socklen_t iOptionLength = sizeof (iOptionLength);
			if (0 != getsockopt (_fdSocket, SOL_SOCKET, SO_ERROR, 
								 &iOptionValue, &iOptionLength))
			{
				String sError (strerror (errno));
				cerr << "[ERROR]  getsockopt returned error when checking "
						"socket connection in progress."
					 << endl;
			}
			if (0 == iOptionValue)
			{
				e_ConnectionStatus = Socket::Connected;
				_fnCallback (_fdSocket);
			}
		}
		else
		{
			cerr << "[ERROR]  Received write-ready notification on socket "
					"that is not connected."
				 << endl;
			abort ();
		}
*/
	}


	// check for sockets that are in error ...				
	if ((SOCKET_FD_UNDEF == _fdSocket) ||
		(SOCKET_FD_CLOSED == _fdSocket) ||
		(SOCKET_FD_DESTROYED == _fdSocket))
		return;

	if (true == FD_ISSET (_fdSocket, &fsError))
	{
		if (EINTR != errno)
		{
			String sError (strerror (errno));
			cerr << "[WARNING]  Socket in error list. " << sError << endl;
			_fnCallback (_fdSocket, false, false, true);
		}
	}


	FD_ZERO (&fsReadReady);
	FD_ZERO (&fsWriteReady);
	FD_ZERO (&fsError);
}



