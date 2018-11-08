#include <iostream>
#include <signal.h>
#include "nlp_socket.h"
using namespace std;


class Server : public ServerSocket
{
	public:
		virtual void OnAccept (ClientSocket* _pClient) {};
};


class Client : public ClientSocket
{
	public:
		static ClientSocket* CreateClient (void);

		virtual void OnConnect (void);
		virtual void OnReceive (const void* _zData, long _lBytes);
};


ClientSocket* Client::CreateClient (void)
{
	return new Client;
}

void Client::OnConnect (void)
{
	cout << "connected" << endl;
}


void Client::OnReceive (const void* _zData, long _lBytes)
{
	char* zText = new char [_lBytes + 1];
	memcpy (zText, _zData, _lBytes);
	zText [_lBytes] = '\0';
	cout << "on receive" << endl;
	cout << zText << endl;

	SendNonBlocking (_zData, _lBytes);
}


//																		
void OnSignal (int _iSignal)
{
	cout << "closing all sockets on terminate." << endl;
	AllSockets::CloseAndDeleteAllSockets ();
	exit (1);
}



//																		
int main (int argc, char* argv[])
{
	signal (SIGINT, OnSignal);

	// test buffer code
	if (false)
	{
		Buffer buf1 ("abc\ndef\nghi\n");
		while (true == buf1.HasTerminator ('\n'))
		{
			// cout << "=>" << buf1 << "<=" << endl;
			String sMessage = buf1.PopFirstMessageAsString ('\n');
			cout << "->" << sMessage << "<-" << endl;
		}

		Buffer buf2 ("abc\x01""def\x01""ghi\x01");
		while (true == buf2.HasTerminator ('\x01'))
		{
			// cout << "=>" << buf2 << "<=" << endl;
			String sMessage = buf2.PopFirstMessageAsString ('\x01');
			cout << "->" << sMessage << "<-" << endl;
		}
	}

	// test blocking client 
	if (false)
	{
		ClientSocket soc;
		soc.SetStandalone (true);
		soc.ConnectBlocking (argv [1], argv [2]);
		soc.ProcessEvents (1000);

		while (true)
		{
			char zData [1024];
			long lBytes = soc.ReceiveBlocking (zData, 1024, 0);
			if (0 >= lBytes)
				break;
			zData [lBytes] = '\0';
			cout << zData << endl;
			soc.SendBlocking (zData, strlen (zData));
		}
	}

	// test blocking server	
	if (false)
	{
		ServerSocket soc;
		while (true)
		{
			ClientSocket* pClient = soc.ListenBlocking (argv [1], 1);

			while (true)
			{
				char zData [1024];
				long lBytes = pClient->ReceiveBlocking (zData, 1024, 0);
				if (0 >= lBytes)
					break;
				zData [lBytes] = '\0';
				cout << zData << endl;
				pClient->SendBlocking (zData, strlen (zData));
			}
		}
	}

	// test non-blocking client 
	if (true)
	{
		Client soc;
		soc.SetStandalone (true);
		soc.ConnectNonBlocking (argv [1], argv [2]);

		while (true)
		{
			// AllSockets::ProcessWaitingSockets (100);
			soc.ProcessEvents (100);
		}
	}

	// test non-blocking server 
	if (false)
	{
		Server soc;
		soc.ListenNonBlocking (argv [1], 1, Client::CreateClient);

		while (true)
		{
			AllSockets::ProcessEvents (100);
		}
	}
}


