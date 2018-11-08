#ifndef __NLP_SOCKET__
#define __NLP_SOCKET__

#include <set>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <nlp_string.h>
using namespace std;


class Socket;
class AllSockets;
class ClientSocket;
class ServerSocket;
typedef deque <int>		int_dq_t;
typedef set <Socket*>	Socket_set_t;
typedef void(*SocketEventCallback_fn_t)(int _fdSocket, bool _bReadReady, bool _bWriteReady, bool _bError);
typedef map <int, SocketEventCallback_fn_t>	SocketToCallback_map_t;



#define SOCKET_FD_UNDEF			-1
#define SOCKET_FD_CLOSED		-2
#define SOCKET_FD_DESTROYED		-3

#define SOCKET_TYPE_UNDEF		0
#define SOCKET_TYPE_SERVER		1
#define SOCKET_TYPE_CLIENT		2


// ----------------------------------------------------------
class Buffer
{
	friend ostream& operator << (ostream& _rStream, const Buffer& _rBuffer);

	protected:
		void* 			z_Data;
		unsigned long	l_Bytes;

	public:
		Buffer (void);
		Buffer (const char* _zData);
		Buffer (const void* _zData, unsigned long _lBytes);
		Buffer (const Buffer& _rBuffer);
		Buffer (const String& _rString);
		~Buffer (void);

		void Append (const char* _zData);
		void Append (const void* _zData, unsigned long _lExtension);
		void DropFront (unsigned long _lDropBytes);
		void Clear (void);

		bool HasTerminator (char _cTerminator);
		String PopFirstMessageAsString (char _cTerminator);
		Buffer PopFirstMessageAsBuffer (char _cTerminator);

		void* GetData (void);
		unsigned long Length (void);

		operator String (void) const;
};

ostream& operator << (ostream& _rStream, const Buffer& _rBuffer);


// ----------------------------------------------------------
#define ADD_MANAGED_SOCKET		(char)'a'
#define REMOVE_MANAGED_SOCKET	(char)'r'
#define CLEAR_MANAGED_SOCKETS	(char)'c'

class SocketManagementRequest
{
	public:
		char						c_Type;
		int							i_Socket;
		SocketEventCallback_fn_t	fn_Callback;

		SocketManagementRequest (char _cType,
								 int _iSocket = -1,
								 SocketEventCallback_fn_t _fnCallback = NULL)
		{
			c_Type = _cType;
			i_Socket = _iSocket;
			fn_Callback = _fnCallback;
		}
};

typedef deque <SocketManagementRequest>		ManagementRequest_dq_t;


// ----------------------------------------------------------
class AllSockets
{
	friend class Socket;

	private:
		static Socket_set_t 			set_AllSockets;
		// The map below is used to manage sockets that are created	
		// externally (i.e., non-Socket-class-sockets).				
		static SocketToCallback_map_t	map_SocketToCallback;

		static pthread_mutex_t			mtx_ExternalSocketManagement;
		static ManagementRequest_dq_t	dq_SocketManagementRequest;

		static void Add (Socket* _pSocket);
		static void Remove (Socket* _pSocket);
		static void ProcessInternalSockets (fd_set& _fsRead, fd_set& _fsWrite, fd_set& _fsError);
		static void ProcessExternalSockets (fd_set& _fsRead, fd_set& _fsWrite, fd_set& _fsError);
	
	public:
		static void ManageSocket (int _iSocketFd, SocketEventCallback_fn_t _fnCallback);
		static void StopManagingSocket (int _iSocketFd);
		static void StopManagingAllExternalSockets (void);

		static void CloseAndDeleteAllSockets (void);
		static void ProcessEvents (unsigned long _lMillisecTimeout);
};



// ----------------------------------------------------------
class Socket
{
	friend class AllSockets;
	friend class ClientSocket;
	friend class ServerSocket;

	enum ConnectionStatus_e
	{
		NotConnected,
		InProgress,
		Connected
	};

	enum BlockingType_e
	{
		Undefined,
		Blocking,
		NonBlocking
	};

	protected:
		int					fd_Socket;
		String				s_Server;
		String				s_Service;
		BlockingType_e		e_BlockingType;
		bool				b_Standalone;
		bool				b_UnixDomain;
		ConnectionStatus_e	e_ConnectionStatus;
		long				l_CurrentTimeout;

		Buffer			o_SendBuffer;
		Buffer			o_ReceiveBuffer;

		Socket (void);

		virtual void OnData (void);

	public:
		virtual ~Socket (void);

		String GetServerId (void) { return s_Server; };
		String GetServiceId (void) { return s_Service; };

		void SetStandalone (bool _bStandalone);
		bool SetNoDelay (bool _bNoDelay);
		bool SetSocketOptions (int _iLevel, 
							   int _iOptionName, 
							   const void* _pOptionValue, 
							   socklen_t _iOptionLength);

		bool Close (void);
		long SendBlocking (const void* _zData, long _lBytes);
		bool SendNonBlocking (const void* _zData, long _lBytes);
		long ReceiveBlocking (const void* _zData, long _lBytes, long _lMillisecTimeout = 0);

		bool IsConnected (void);

		virtual void OnConnect (void) = 0;
		virtual void OnCanSend (void);
		virtual void OnReceive (const void* _zData, long _lBytes) = 0;
		virtual void OnDisconnect (void) = 0;
		virtual void OnError (void);

		void ProcessEvents (unsigned long _lMillisecTimeout);
		static void ProcessEvents (int _fdSocket, 
									unsigned long _lMillisecTimeout,
									SocketEventCallback_fn_t _fnCallback);
};



// ----------------------------------------------------------
class ClientSocket : public Socket
{
	friend class ServerSocket;

	protected:
		ServerSocket* p_ServerSocket;

		bool Connect (const char* _zServer, const char* _zService, BlockingType_e _eBlockingType);
		bool UnixDomainConnect (const char* _zService, BlockingType_e _eBlockingType);

	public:
		ClientSocket (void);
		~ClientSocket (void);

		static ClientSocket* CreateClient (void);
		bool ConnectBlocking (const char* _zServer, const char* _zService);
		bool ConnectNonBlocking (const char* _zServer, const char* _zService);
		bool ConnectUnixDomainBlocking (const char* _zService);
		bool ConnectUnixDomainNonBlocking (const char* _zService);

		virtual void OnConnect (void) {};
		virtual void OnCanSend (void);
		virtual void OnReceive (const void* _zData, long _lBytes) {};
		virtual void OnDisconnect (void) {};
};



// ----------------------------------------------------------
typedef ClientSocket* (*CreateClientFunction_t) (void);

class ServerSocket : public Socket
{
	protected:
		Socket_set_t			set_ClientSockets;
		CreateClientFunction_t	fn_CreateClient;
		bool					b_AllowAddressReuse;

		void CloseClients (void);
		void OnData (void);

	public:
		ServerSocket (void);
		~ServerSocket (void);

		void SetAllowAddressReuse (bool _bAllowReuse);

		ClientSocket* ListenBlocking (const char* _zService, 
									unsigned int _iBacklog, 
									CreateClientFunction_t _fnCreateClient = ClientSocket::CreateClient);
		bool ListenNonBlocking (const char* _zService, 
									unsigned int _iBacklog,
									CreateClientFunction_t _fnCreateClient);

		ClientSocket* ListenUnixDomainBlocking (const char* _zService, 
									unsigned int _iBacklog, 
									CreateClientFunction_t _fnCreateClient = ClientSocket::CreateClient);
		bool ListenUnixDomainNonBlocking (const char* _zService, 
									unsigned int _iBacklog,
									CreateClientFunction_t _fnCreateClient);

		void OnClientDestroy (Socket* _pClient);

		virtual void OnAccept (ClientSocket* _pClient);
		virtual void OnConnect (void);
		virtual void OnCanSend (void);
		virtual void OnReceive (const void* _zData, long _lBytes);
		virtual void OnDisconnect (void) {};
};



#endif
