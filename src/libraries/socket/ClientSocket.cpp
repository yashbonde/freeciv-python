#include "nlp_socket.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>


//																	
ClientSocket::ClientSocket (void)
	: Socket ()
{
	p_ServerSocket = NULL;
	e_ConnectionStatus = NotConnected;
}


ClientSocket::~ClientSocket (void)
{
	if (NULL != p_ServerSocket)
		p_ServerSocket->OnClientDestroy (this);
	p_ServerSocket = NULL;
}


ClientSocket* ClientSocket::CreateClient (void)
{
	return new ClientSocket;
}


//																	
bool ClientSocket::Connect (const char* _zServer,
							const char* _zService,
							BlockingType_e _eBlockingType)
{
	assert (SOCKET_FD_DESTROYED != fd_Socket);
	Close ();

	s_Server = _zServer;
	s_Service = _zService;

	// obtain addresses matching host/port 
	struct addrinfo hints;
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = 0;

	struct addrinfo* result;
	int iStatus = getaddrinfo (s_Server, s_Service, &hints, &result);
	if (0 != iStatus)
	{
		String sError (gai_strerror (iStatus));
		cerr << "[ERROR] Failed to translate address [" 
			 << s_Server << ':' << s_Service << "]. "
			 << sError << endl;
		return false;
	}

	// check each address returned by getaddrinfo	
	
	int fdSocket;
	struct addrinfo* rp;
	for (rp = result; NULL != rp; rp = rp->ai_next)
	{
		fdSocket = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == fdSocket)
			continue;
		if (NonBlocking == _eBlockingType)
		{
			fcntl (fdSocket, F_SETFL, O_NONBLOCK);
			if (-1 != connect (fdSocket, rp->ai_addr, rp->ai_addrlen))
			{
				e_ConnectionStatus = Connected;
				break;
			}
			else if (EINPROGRESS == errno)
			{
				e_ConnectionStatus = InProgress;
				break;
			}
		}
		else
		{
			if (-1 != connect (fdSocket, rp->ai_addr, rp->ai_addrlen))
			{
				e_ConnectionStatus = Connected;
				break;
			}
		}
		close (fdSocket);
	}

	if (NULL == rp)
	{
		fd_Socket = SOCKET_FD_CLOSED;
		cerr << "[ERROR] Unable to connect to [" 
			 << s_Server << ':' << s_Service << "]. "
			 << endl;
		return false;
	}

	freeaddrinfo (result);
	fd_Socket = fdSocket;

	if (Connected == e_ConnectionStatus)
		OnConnect ();

	if (NonBlocking == e_BlockingType)
		return true;
	else
		return (Connected == e_ConnectionStatus);
}



//																	
bool ClientSocket::UnixDomainConnect (const char* _zService,
									  BlockingType_e _eBlockingType)
{
	assert (SOCKET_FD_DESTROYED != fd_Socket);
	Close ();

	s_Server = "<unix-domain>";
	s_Service = _zService;
	b_UnixDomain = true;

	fd_Socket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (-1 == fd_Socket)
	{
		String sError (strerror (errno));
		cerr << "[ERROR] Unix domain creation failed on [" << s_Service 
			 << "] Error : " << sError << endl;
		return false;
	}

	sockaddr_un oAddress;
	oAddress.sun_family = AF_UNIX;
	strcpy (oAddress.sun_path, _zService);
	size_t iLen = strlen (oAddress.sun_path) + sizeof (oAddress.sun_family);

	if (NonBlocking == _eBlockingType)
	{
		fcntl (fd_Socket, F_SETFL, O_NONBLOCK);
		if (-1 != connect (fd_Socket, (sockaddr*)&oAddress, iLen))
			e_ConnectionStatus = Connected;
		
		else if (EINPROGRESS == errno)
			e_ConnectionStatus = InProgress;
	}
	else
	{
		if (-1 != connect (fd_Socket, (sockaddr*)&oAddress, iLen))
			e_ConnectionStatus = Connected;
	}


	if (Connected == e_ConnectionStatus)
		OnConnect ();

	if (NonBlocking == e_BlockingType)
		return true;
	else
		return (Connected == e_ConnectionStatus);
}



//																	
bool ClientSocket::ConnectBlocking (const char* _zServer, const char* _zService)
{
	e_BlockingType = Blocking;
	return Connect (_zServer, _zService, e_BlockingType);
}



//																	
bool ClientSocket::ConnectNonBlocking (const char* _zServer, const char* _zService)
{
	e_BlockingType = NonBlocking;
	return Connect (_zServer, _zService, e_BlockingType);
}



//																	
bool ClientSocket::ConnectUnixDomainBlocking (const char* _zService)
{
	e_BlockingType = Blocking;
	return UnixDomainConnect (_zService, e_BlockingType);
}



//																	
bool ClientSocket::ConnectUnixDomainNonBlocking (const char* _zService)
{
	e_BlockingType = NonBlocking;
	return UnixDomainConnect (_zService, e_BlockingType);
}



//																	
void ClientSocket::OnCanSend (void)
{
	Socket::OnCanSend ();
}
