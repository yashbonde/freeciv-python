#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include "nlp_socket.h"
#include <nlp_macros.h>


ServerSocket::ServerSocket (void)
	: Socket ()
{
	fn_CreateClient = NULL;
	b_AllowAddressReuse = false;
}


ServerSocket::~ServerSocket (void)
{
	CloseClients ();
	fn_CreateClient = NULL;
}


void ServerSocket::OnClientDestroy (Socket* _pClient)
{
	set_ClientSockets.erase (_pClient);
}


void ServerSocket::CloseClients (void)
{
	ITERATE (Socket_set_t, set_ClientSockets, ite)
		delete *ite;
}


//																					
void ServerSocket::SetAllowAddressReuse (bool _bAllowReuse)
{
	if (b_AllowAddressReuse != _bAllowReuse)
	{
		if (Connected == e_ConnectionStatus)
			cout << "[WARNING] Request to change AllowAddressReuse flag on a "
					"connected socket.  Flag change will take effect only the "
					"next ListenBlocking() or ListenNonBlocking() call."
				 << endl;
	}

	b_AllowAddressReuse = _bAllowReuse;
}


//																					
ClientSocket* ServerSocket::ListenBlocking (const char* _zService, 
											unsigned int _iBacklog,
											CreateClientFunction_t _fnCreateClient)
{
	CloseClients ();
	Close ();

	s_Service = _zService;
	e_BlockingType = Blocking;

	// translate service name ...	
	struct addrinfo hints;
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	struct addrinfo *result;
	int iStatus = getaddrinfo (NULL, s_Service, &hints, &result);
	if (0 != iStatus)
	{
		String sError (gai_strerror (iStatus));
		cerr << "[ERROR] Failed to resolve address [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return NULL;
	}


	struct addrinfo *rp;
	for (rp = result; rp != NULL; rp = rp->ai_next) 
	{
		fd_Socket = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == fd_Socket)
			continue;

		int iTrue = 0;
		if (true == b_AllowAddressReuse)
			iTrue = 1;
		if (-1 == setsockopt (fd_Socket, SOL_SOCKET, SO_REUSEADDR, &iTrue, sizeof(iTrue)))
		{
			String sError (strerror (errno));
			cerr << "[ERROR] setsockopt failed while trying to set SO_REUSEADDR flag. "
					"Error : " << sError << endl;
			return NULL;
		}

		if (0 == bind (fd_Socket, rp->ai_addr, rp->ai_addrlen))
			break;
		close (fd_Socket);
	}

	if (rp == NULL) 
	{
		fd_Socket = SOCKET_FD_CLOSED;
		cerr << "[ERROR] Failed to bind to address [" 
			 << s_Server << ':' << s_Service << "]. "
			 << endl;
		return NULL;
	}

	freeaddrinfo (result);

	// listen for incomming connection ...	
	if (0 != listen (fd_Socket, _iBacklog))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Listen failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return NULL;
	}


	// accept incomming connection ..		
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof (struct sockaddr_storage);
	int fdClient = accept (fd_Socket, (struct sockaddr*)&peer_addr, &peer_addr_len);
	if (-1 == fdClient)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Accept failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return NULL;
	}

	ClientSocket* pClient = _fnCreateClient ();
	pClient->fd_Socket = fdClient;
	pClient->e_BlockingType = Blocking;

	char zHostName [NI_MAXHOST];
	char zServiceName [NI_MAXSERV];
	iStatus = getnameinfo ((struct sockaddr *) &peer_addr,
							peer_addr_len, zHostName, NI_MAXHOST,
							zServiceName , NI_MAXSERV, NI_NUMERICSERV);
	if (0 == iStatus)
	{
		pClient->s_Server = zHostName;
		pClient->s_Service = zServiceName;
	}
	else
	{
		String sError (gai_strerror (iStatus));
		cerr << "[WARNING] Failed to get peer details on ["
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		pClient->s_Server = "[unknown]";
		pClient->s_Service = "[unknown]";
	}

	pClient->p_ServerSocket = this;
	set_ClientSockets.insert (pClient);
	pClient->e_ConnectionStatus = Connected;
	pClient->OnConnect ();
	return pClient;
}


//																					
bool ServerSocket::ListenNonBlocking (const char* _zService, 
									  unsigned int _iBacklog,
									  CreateClientFunction_t _fnCreateClient)
{
	CloseClients ();
	Close ();

	s_Service = _zService;
	fn_CreateClient = _fnCreateClient;
	e_BlockingType = NonBlocking;

	// translate service name ...	
	struct addrinfo hints;
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	struct addrinfo *result;
	int iStatus = getaddrinfo (NULL, s_Service, &hints, &result);
	if (0 != iStatus)
	{
		String sError (gai_strerror (iStatus));
		cerr << "[ERROR] Failed to resolve address [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return false;
	}


	struct addrinfo *rp;
	for (rp = result; rp != NULL; rp = rp->ai_next) 
	{
		fd_Socket = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == fd_Socket)
			continue;
		if (0 == bind (fd_Socket, rp->ai_addr, rp->ai_addrlen))
			break;
		close (fd_Socket);
	}

	if (rp == NULL) 
	{
		fd_Socket = SOCKET_FD_CLOSED;
		cerr << "[ERROR] Failed to bind to address [" 
			 << s_Server << ':' << s_Service << "]. "
			 << endl;
		return false;
	}

	freeaddrinfo (result);

	// Set the socket to non-blocking ...	
	fcntl (fd_Socket, F_SETFL, O_NONBLOCK);

	// listen for incomming connection ...	
	if (0 != listen (fd_Socket, _iBacklog))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Listen failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return false;
	}

	e_ConnectionStatus = Connected;
	return true;
}



//																					
ClientSocket* ServerSocket::ListenUnixDomainBlocking (const char* _zService, 
													unsigned int _iBacklog,
													CreateClientFunction_t _fnCreateClient)
{
	CloseClients ();
	Close ();
	unlink (_zService);

	s_Server = "<unix-domain>";
	s_Service = _zService;
	e_BlockingType = Blocking;
	b_UnixDomain = true;

	fd_Socket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (-1 == fd_Socket)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Unix domain creation failed on [" << s_Service 
			 << "] Error : " << sError << endl;
		return NULL;
	}

	sockaddr_un oAddress;
	oAddress.sun_family = AF_UNIX;
	strcpy (oAddress.sun_path, _zService);
	size_t iLen = strlen (oAddress.sun_path) + sizeof (oAddress.sun_family);

	if (-1 == bind (fd_Socket, (sockaddr*)&oAddress, iLen))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Unix domain bind failed on [" << s_Service 
			 << "] Error : " << sError << endl;
		close (fd_Socket);
		return NULL;
	}


	// listen for incomming connection ...	
	if (0 != listen (fd_Socket, _iBacklog))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Listen failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return NULL;
	}


	// accept incomming connection ..		
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof (struct sockaddr_storage);
	int fdClient = accept (fd_Socket, (struct sockaddr*)&peer_addr, &peer_addr_len);
	if (-1 == fdClient)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Accept failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return NULL;
	}

	ClientSocket* pClient = _fnCreateClient ();
	pClient->fd_Socket = fdClient;
	pClient->e_BlockingType = Blocking;

	char zHostName [NI_MAXHOST];
	char zServiceName [NI_MAXSERV];
	int iStatus = getnameinfo ((struct sockaddr *) &peer_addr,
								peer_addr_len, zHostName, NI_MAXHOST,
								zServiceName , NI_MAXSERV, NI_NUMERICSERV);
	if (0 == iStatus)
	{
		pClient->s_Server = zHostName;
		pClient->s_Service = zServiceName;
	}
	else
	{
		String sError (gai_strerror (iStatus));
		cerr << "[WARNING] Failed to get peer details on ["
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		pClient->s_Server = "[unknown]";
		pClient->s_Service = "[unknown]";
	}

	pClient->p_ServerSocket = this;
	set_ClientSockets.insert (pClient);
	pClient->e_ConnectionStatus = Connected;
	pClient->OnConnect ();
	return pClient;
}


//																					
bool ServerSocket::ListenUnixDomainNonBlocking (const char* _zService, 
												unsigned int _iBacklog,
												CreateClientFunction_t _fnCreateClient)
{
	CloseClients ();
	Close ();
	unlink (_zService);

	s_Server = "<unix-domain>";
	s_Service = _zService;
	fn_CreateClient = _fnCreateClient;
	e_BlockingType = NonBlocking;
	b_UnixDomain = true;

	fd_Socket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (-1 == fd_Socket)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Unix domain creation failed on [" << s_Service 
			 << "] Error : " << sError << endl;
		return NULL;
	}

	sockaddr_un oAddress;
	oAddress.sun_family = AF_UNIX;
	strcpy (oAddress.sun_path, _zService);
	size_t iLen = strlen (oAddress.sun_path) + sizeof (oAddress.sun_family);

	if (-1 == bind (fd_Socket, (sockaddr*)&oAddress, iLen))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Unix domain bind failed on [" << s_Service 
			 << "] Error : " << sError << endl;
		close (fd_Socket);
		return NULL;
	}


	// Set the socket to non-blocking ...	
	fcntl (fd_Socket, F_SETFL, O_NONBLOCK);

	// listen for incomming connection ...	
	if (0 != listen (fd_Socket, _iBacklog))
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Listen failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return false;
	}

	e_ConnectionStatus = Connected;
	return true;
}



//																					
void ServerSocket::OnData (void)
{
	if (Undefined == e_BlockingType)
	{
		cerr << "[ERROR] Socket::OnData called on a socket of undefined blocking type." << endl;
		abort ();
	}
	if (Blocking == e_BlockingType)
	{
		cerr << "[ERROR]  Asynchronous on accept (OnData) called on a blocking socket." 
			 << endl;
		abort ();
	}

	// accept incomming connection ..		
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof (struct sockaddr_storage);
	int fdClient = accept (fd_Socket, 
							(struct sockaddr*)&peer_addr,
							&peer_addr_len);

	if (-1 == fdClient)
	{
		if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			return;

		String sError (strerror (errno));
		cerr << "[ERROR] Accept failed on [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return;
	}

	// Set the socket to non-blocking ...	
	fcntl (fdClient, F_SETFL, O_NONBLOCK);

	ClientSocket* pClient = fn_CreateClient ();
	pClient->fd_Socket = fdClient;
	pClient->e_BlockingType = NonBlocking;
	pClient->b_UnixDomain = b_UnixDomain;

	char zHostName [NI_MAXHOST];
	char zServiceName [NI_MAXSERV];
	int iStatus = getnameinfo ((struct sockaddr *) &peer_addr,
								peer_addr_len, zHostName, NI_MAXHOST,
								zServiceName , NI_MAXSERV, NI_NUMERICSERV);
	if (0 == iStatus)
	{
		pClient->s_Server = zHostName;
		if (true == b_UnixDomain)
			pClient->s_Service = s_Service;
		else
			pClient->s_Service = zServiceName;
	}
	else
	{
		String sError (gai_strerror (iStatus));
		cerr << "[WARNING] Failed to get peer details on ["
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		pClient->s_Server = "[unknown]";
		pClient->s_Service = "[unknown]";
	}

	pClient->p_ServerSocket = this;
	set_ClientSockets.insert (pClient);
	pClient->e_ConnectionStatus = Connected;
	pClient->OnConnect ();
	OnAccept (pClient);
}



//																					
void ServerSocket::OnAccept (ClientSocket* _pClient)
{
}



//																					
void ServerSocket::OnConnect (void)
{
	cerr << "[ERROR]  OnConnect called on ServerSocket.  This should never happen."
		 << endl;
	abort ();
}



//																					
void ServerSocket::OnCanSend (void)
{
	cerr << "[ERROR]  OnCanSend called on ServerSocket.  This should never happen."
		 << endl;
	abort ();
}



//																					
void ServerSocket::OnReceive (const void* _zData, long _lBytes)
{
	cerr << "[ERROR]  OnReceive called on ServerSocket.  This should never happen."
		 << endl;
	abort ();
}



