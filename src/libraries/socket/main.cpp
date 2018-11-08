#include <iostream>
#include <signal.h>
#include "nlp_socket.h"
#include <nlp_time.h>
using namespace std;


class Client : public ClientSocket
{
	public:
		bool b_Sender;
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
	if (true == b_Sender)
		return;

	if (0 == strncmp ((const char*)_zData, "bye\n", 4))
	{
		cout << "done" << endl;
		Close ();
		exit (0);
	}
	if (NULL != memchr (_zData, '\n', _lBytes))
		SendBlocking ("\n", 1);
	// cout << '.' << flush;
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

	if (argc < 4)
	{
		cout << "command line: [s/c] [unix-domain/ip address] [service]" << endl;
		return 1;
	}


	if (0 == strcmp (argv [1], "s"))
	{
		ServerSocket socServer;
		socServer.SetStandalone (true);
		Client* pSender = NULL;
		if (0 == strcmp (argv [2], "unix-domain"))
		{
			cout << "unix domain server..." << endl;
			pSender = (Client*)socServer.ListenUnixDomainBlocking (argv [3], 3, Client::CreateClient);
		}
		else
		{
			cout << "ip domain server..." << endl;
			pSender = (Client*)socServer.ListenBlocking (argv [3], 3, Client::CreateClient);
		}
		if (NULL == pSender)
			return 1;

		pSender->SetStandalone (true);
		pSender->b_Sender = true;

		Time oTimer;
		oTimer.StartTimer ();
		for (int i = 0; i < 1000000; ++ i)
		{
			// cout << ':' << flush;
			pSender->SendBlocking ("hello........................................................................................................................................................................................................\n", 206);
			char* zData [1000];
			pSender->ReceiveBlocking (zData, 1000);
		}
		oTimer.StopTimer ();
		cout << oTimer.sStartStopTime () << endl;

		pSender->SendBlocking ("bye\n", 6);
		pSender->Close ();
		delete pSender;

		socServer.Close ();
	}
	else
	{
		Client socClient;
		socClient.SetStandalone (true);
		socClient.b_Sender = false;
		if (0 == strcmp (argv [2], "unix-domain"))
		{
			cout << "unix domain client..." << endl;
			socClient.ConnectUnixDomainBlocking (argv [3]);
		}
		else
		{
			cout << "ip domain client..." << endl;
			socClient.ConnectBlocking (argv [2], argv [3]);
		}

		while (true)
			socClient.ProcessEvents (100);
	}
}


