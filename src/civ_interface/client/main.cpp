extern "C" {
#include "message.h"
#define SKIP_SHARED
#include "fc_types.h"

unsigned int i_Status;
unsigned int game_won;
unsigned int game_lost;
unsigned int full_reset;
unsigned int enable_early_end;
extern char user_name[512];
extern char server_host[512];
extern char server_service[512];


extern bool gui_interactive_mode;
extern bool unit_selection_clears_orders;
extern bool ask_city_name;

extern int client_main (int argc, char **argv);
extern void input_from_server (int fd, bool _bReadReady, bool _bWriteReady, bool _bError);
extern bool get_turn_done_button_state(void);
extern bool is_server_busy(void);
extern bool can_client_issue_orders(void);
extern bool can_client_change_view(void);
extern struct unit *game_find_unit_by_number(int id);
extern struct city *game_find_city_by_number(int id);
extern struct government* government_by_number (int id);
extern struct government* government_of_player (struct player* _pPlayer);
extern void set_unit_focus(struct unit *punit);
extern bool is_client_preparing (void);

extern void key_end_turn (void);
extern void key_unit_auto_explore (void);
extern void key_unit_auto_settle (void);
extern void key_unit_build_city (void);
extern void key_unit_fortify (void);
extern void key_unit_homecity (void);
extern void key_unit_irrigate (void);
extern void key_unit_mine (void);
extern void key_unit_road (void);
extern void key_unit_sentry (void);
extern void key_unit_transform (void);

extern int send_chat(const char *message);
extern void send_chat_printf(const char *format, ...);
extern int connect_to_server(const char *username, const char *hostname, const char* service, char *errbuf, int errbufsize);
extern void disconnect_from_server (void);
extern void attribute_flush (void);
extern void make_connection(int socket, const char *username);
extern void init_request_ids (void);

extern struct tile *native_pos_to_tile(int nat_x, int nat_y);
extern bool is_valid_goto_draw_line(struct tile *dest_tile);
extern bool goto_get_turns(int *min, int *max);
extern struct pf_path* pf_map_get_path (struct pf_map* _pMap, struct tile* _pTile);
extern void pf_path_destroy (struct pf_path* _pPath);

extern enum unit_activity GetUnitActivity (struct unit* _pUnit);
extern void input_from_server_till_request_got_processed(int fd, int expected_request_id);

extern void game_init(void);
extern void game_free(void);
extern void game_ruleset_free(void);

struct pf_position
{
	struct tile *tile;
	int turn, moves_left, fuel_left;

	int total_MC;
	int total_EC;

	enum direction8 dir_to_next_pos;
	enum direction8 dir_to_here;
};

struct pf_path
{
	int length;
	struct pf_position *positions;
};



// functions for remote machine interface 
extern void GetGameScore (struct Message* _pResponse);
extern void MeasureState (struct Message* _pResponse);
extern void UnitGotoMI (struct Message* _pResponse, struct unit* _pUnit, int _x, int _y);
extern void UnitMoveMI (struct Message* _pResponse, struct unit* _pUnit, const char* _zDirection);

extern void SetCityGovernorMI (struct Message* _pResponse, struct city* _pCity, int _iGovernor);
extern void SetCityBuildMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem);
extern void SetCityBuyMI (struct Message* _pResponse, struct city* _pCity, const char* _zItem);

extern void AvailableCityImprovements (struct Message* _pResponse, struct city* _pCity);

// functions for remote human interface 
extern void ListNations (struct Message* _pResponse);
extern void ListGovernment (struct Message* _pResponse);
extern void ListGrid (struct Message* _pResponse);
extern void ListUnits (struct Message* _pResponse);
extern void ListCities (struct Message* _pResponse);
extern void ListTechnologies (struct Message* _pResponse, bool _bAll);
extern void ListCityGovernors (struct Message* _pResponse);

extern void UnitGoto (struct Message* _pResponse, int _iId, int _x, int _y);
extern void UnitMove (struct Message* _pResponse, int _iId, const char* _zDirection);
extern void UnitActMisc (struct Message* _pResponse, int _iId, const char* _zAction);
extern void CityBuild (struct Message* _pResponse, int _iId, const char* _zAction, const char* _zItem);
extern void SetResearch (struct Message* _pResponse, int _iId);
extern void SetCityGovernor (struct Message* _pResponse, int _iCity, int _iGovernor);
extern void SetGovernment (struct Message* _pResponse, int _iId);
extern const char* GetTileLabels (int _x, int _y, struct unit* _pCurrentUnit);

void SendSocketMessage (const char* _zMessage);
void GetUnitGotoActions (struct Message* _pResponse,
						 int _iUnitId,
						 const char* _zUnitName,
						 struct pf_map* _pPathMap,
						 int _x, int _y);
void IdleCallbacks (void);
int GetLastRequestSent (void);
int GetLastRequestProcessed (void);
bool IsAlive (void);
void OpponentUnitDirections (int _x, int _y, char* _pOpponentPresense);
void ProcessLeftoverData (void);
}

#include "learner_comms.h"
#include "state.h"
#include <iostream>
#include <nlp_string.h>
#include <nlp_socket.h>
#include <nlp_macros.h>
#include <nlp_matrix.h>
#include <nlp_filesystem.h>
#include <nlp_time.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <queue>
using namespace std;

class CommandSocket;
class IdleCallback;
typedef list<IdleCallback*>	IdleCallback_lst_t;
typedef queue<String>		InternalCommands_que_t;



//													
int						i_ServerSocketFd;
CommandSocket*			psoc_CommandConnection = NULL;
ServerSocket 			soc_Server;
pthread_t				thr_CommandThread;
IdleCallback_lst_t		lst_IdleCallback;
InternalCommands_que_t	que_InternalCommands;

String				s_PreviousSaveGame;
String				s_LastGameLoaded;
bool				b_WaitingForTurn;
Time				o_Timer1;
long				i_Watchdog;
int					i_WatchdogTimeout;
int					i_GameStep;
String				s_LastCommand;
bool				b_WatchdogTriggered;

String GetInterfacePort (int argc, char* argv[]);
void* CommandLoop (void*);
void ProcessUserCommand (String& _rCommand);
void ProcessMachineCommand (String& _rCommand);
void SocketEventCallback (int _fdSocket, bool _bReadReady, bool _bWriteReady, bool _bError);
void WaitForRequestCompletion (void);
void ModifyGameSaveFile (const char* _zFileName, const char* _zUserName, bool _bEndCheck);



//													
class IdleCallback
{
	public:
		void (*pfn_Callback)(void*);
		void* p_Data;
};


//													
class CommandSocket : public ClientSocket
{
	private:
		Buffer	buf_Command;

	public:
		static ClientSocket* CreateClient (void);
		virtual void OnReceive (const void* _zData, long _lBytes);
		virtual void OnConnect (void);
		virtual void OnDisconnect (void);
};


//													
ClientSocket* CommandSocket::CreateClient (void)
{
	psoc_CommandConnection = new CommandSocket;
	return psoc_CommandConnection;
}

void CommandSocket::OnConnect (void)
{
	cout << "[..] Client connected from " << s_Server << ":" << s_Service << endl;
	gui_interactive_mode = false;
	Socket::SetNoDelay (true);
}

void CommandSocket::OnDisconnect (void)
{
	cout << "[..] Client disconnected from " << s_Server << ":" << s_Service << endl;
	gui_interactive_mode = true;

	// disconnect_from_server ();

	psoc_CommandConnection->Close ();
	delete psoc_CommandConnection;
	psoc_CommandConnection = NULL;

	i_Status = STATE_READY;
	b_WaitingForTurn = false;
}

void CommandSocket::OnReceive (const void* _zData, long _lBytes)
{
	i_Watchdog = 0;
	buf_Command.Append (_zData, _lBytes);

	while (true == buf_Command.HasTerminator ('\n'))
	{
		String sCommand = buf_Command.PopFirstMessageAsString ('\n');
		if (true == sCommand.Has ("."))
			ProcessUserCommand (sCommand);
		else
			ProcessMachineCommand (sCommand);
	}
}



/* signal handler for stack trace */
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/ucontext.h>

#if __WORDSIZE == 64 
#define IP_REGISTER	REG_RIP
#else
#define IP_REGISTER	REG_EIP
#endif


void stacktrace (int sig, siginfo_t *info, void *secret)
{
	void *trace[32];
	int trace_size = 0;
	trace_size = backtrace(trace, 32);
	backtrace_symbols_fd(trace, trace_size, STDERR_FILENO);

	exit(0);
}



//													
void OnSignal (int _iSignal)
{
	cout << "[..] Closing all sockets on signal " << _iSignal << "." << endl;
	if (NULL != psoc_CommandConnection)
	{
		psoc_CommandConnection->Close ();
		delete psoc_CommandConnection;
		psoc_CommandConnection = NULL;
	}
	AllSockets::CloseAndDeleteAllSockets ();
	exit (0);
}


//													
void OnExit (void)
{
	cout << "[..] Closing all sockets on exit." << endl;
	if (NULL != psoc_CommandConnection)
	{
		psoc_CommandConnection->Close ();
		delete psoc_CommandConnection;
		psoc_CommandConnection = NULL;
	}
	AllSockets::CloseAndDeleteAllSockets ();

	ITERATE (IdleCallback_lst_t, lst_IdleCallback, ite)
		delete *ite;
}


//													
int main(int argc, char **argv)
{
	cout << "Freeciv NLP client interface. Version "
		 << __DATE__ << ' ' << __TIME__ << endl;
	psoc_CommandConnection = NULL;

	b_WaitingForTurn = false;
	i_Status = STATE_READY;
	i_WatchdogTimeout = 30;

	game_won = false;
	game_lost = false;
	full_reset = false;
	enable_early_end = false;

	b_WatchdogTriggered = false;

	atexit (OnExit);
	signal (SIGINT, OnSignal);

	String sPort = GetInterfacePort (argc, argv);
	if ("" != sPort)
	{
		pthread_create (&thr_CommandThread, NULL, CommandLoop, NULL);
		pthread_detach (thr_CommandThread);

		soc_Server.SetAllowAddressReuse (true);
		if (true == sPort.IsDigit ())
		{
			cout << "[..] Starting remote interface on tcp/ip port " << sPort << endl;
			soc_Server.ListenNonBlocking (sPort, 5, CommandSocket::CreateClient);
		}
		else
		{
			cout << "[..] Starting remote interface on unix domain port " << sPort << endl;
			soc_Server.ListenUnixDomainNonBlocking (sPort, 5, CommandSocket::CreateClient);
		}
	}

	int ret = client_main(argc, argv);
	if ("" != sPort)
		cout << "[..] Closing all sockets on termination." << endl;

	AllSockets::CloseAndDeleteAllSockets ();

	return ret;
}



//													
String GetInterfacePort (int argc, char* argv[])
{
	String sPort;
	for (int i = 1; i < argc; ++ i)
	{
		if ((0 != strcmp ("--interface", argv [i-1])) &&
			(0 != strcmp ("-i", argv [i-1])))
			continue;
		sPort = argv [i];
		break;
	}
	return sPort;
}


//													
void* CommandLoop (void*)
{
	i_Watchdog = 0;
	Time o_WatchdogTimer;
	while (true)
	{
		if (true == b_WaitingForTurn)
		{
			if (0 == i_Watchdog)
				o_WatchdogTimer.StartTimer ();

			++ i_Watchdog;

			// if ((i_Watchdog > 10000) || (o_WatchdogTimer.SecondsToNow () > 10))
			if ((i_Watchdog > 10000000) || (o_WatchdogTimer.SecondsToNow () > i_WatchdogTimeout))
			{
				cout << "[WARNING] Watchdog timer triggered ["
					 << i_Status << "] ["
					 << o_WatchdogTimer.sTimeToNow () << ']' << endl;
				cout << "won/lost : " << game_won << '/' << game_lost << endl;
				cout << "triggering command : [" << s_LastCommand << ']' << endl;

				i_Status = STATE_READY;

				i_Watchdog = 0;
				b_WatchdogTriggered = true;
			}

			// if in ready state, 
			if ((STATE_READY == i_Status) && (false == is_client_preparing ()))
			{
				b_WaitingForTurn = false;

				// process any queued internal messages ...		
				if (false == que_InternalCommands.empty ())
				{
					String sCommand = que_InternalCommands.front ();
					que_InternalCommands.pop ();

					ProcessMachineCommand (sCommand);
				}

				// if we've won or lost the game, we need to 	
				// disconnect & reconnect from the server, or	
				// the server is not going to respond to any of	
				// our commands :-/								
				else if (NULL != psoc_CommandConnection)
				{
					// cout << "sending ack" << endl;
					// if ((true == game_won) || (true == game_lost))
					if (true == full_reset)
					{
						// The "/load" command below seems to be 
						// essential to restart the simulation	 
						// games after a win or loss. Otherwise  
						// the server will start a generic game, 
						// and then doesn't seem to allow us to  
						// control that game, leading to timeouts
						// of the watchdog...				     
						send_chat ("/endgame");
						send_chat_printf("/load %s", (const char*) s_LastGameLoaded);
						send_chat ("/start");

						psoc_CommandConnection->SendBlocking (LCP_GAMEFINISHED LCP_TERMINATOR, 
															 strlen (LCP_GAMEFINISHED LCP_TERMINATOR));
					}

					// if there are no internal commands queued,	
					// send acknowledgement to client...			
					else
						psoc_CommandConnection->SendBlocking ("." LCP_TERMINATOR, 2);
				}


				// reset watchdog command log...				
				if (false == b_WatchdogTriggered)
					s_LastCommand = "";
			}
		}

		AllSockets::ProcessEvents (100);
		
		// ProcessLeftoverData ();
	}

	return NULL;
}


//													
void IdleCallbacks (void)
{
	ITERATE (IdleCallback_lst_t, lst_IdleCallback, ite)
		(*ite)->pfn_Callback ((*ite)->p_Data);
}


//													
void SocketEventCallback (int _fdSocket, bool _bReadReady, bool _bWriteReady, bool _bError)
{
	if ((false == _bReadReady) && (false == _bWriteReady) && (false == _bError))
		return;

	// cout << _bReadReady << _bWriteReady << _bError << endl;
	input_from_server (_fdSocket, _bReadReady, true, _bError);
}


//													
void WaitForRequestCompletion (void)
{
	int iId = GetLastRequestSent ();
	input_from_server_till_request_got_processed (i_ServerSocketFd, iId);
}


//													
void Error (Message* _pReply, const char* _zMessage, const char* _zCommand, int _iParams)
{
	char zError [100];
	sprintf (zError, _zMessage, _zCommand, _iParams);
	append_to_message (_pReply, zError);
	append_to_message (_pReply, LCP_TERMINATOR);
}


//													
void ProcessMachineCommand (String& _rCommand)
{
	_rCommand.Strip ();
	String_dq_t	dqValues;
	_rCommand.Split (dqValues, ' ');

	// cout << "<<" << _rCommand << ">>  " << Time::CurrentTime () << endl;
	// if (true == game_lost)
	//	cout << '[' << _rCommand << ']' << endl;

	// force interactive mode off if we're getting commands 
	// from a machine...									
	gui_interactive_mode = true;

	struct unit* pUnit = NULL;
	struct city* pCity = NULL;
	enum unit_activity eUnitActivity = ACTIVITY_IDLE;

	// check if this is a unit command, and if so find the	
	// corresponding unit...								
	// if (true == _rCommand.StartsWith (LCP_UNIT_COMMAND))
	if (LCP_UNIT_COMMAND == _rCommand [0])
	{
		int iId = dqValues [1];
		pUnit = game_find_unit_by_number (iId);
		if (NULL == pUnit)
		{
			cout << "[ERROR] Invalid unit id [" << iId << "] in unit command [" 
				 << _rCommand << "] step " << i_GameStep << endl;
			return;
		}

		eUnitActivity = GetUnitActivity (pUnit);	
	}

	// check if this is a city command, and if so find the	
	// corresponding city...								
	// else if ((true == _rCommand.StartsWith (LCP_CITY_COMMAND)) &&
	//		(LCP_CITY_BUILD_CONTINUE != _rCommand))
	else if (LCP_CITY_COMMAND == (_rCommand [0] & ~1))
	{
		int iId = dqValues [1];
		pCity = game_find_city_by_number (iId);
		if (NULL == pCity)
		{
			cout << "[ERROR] Invalid city id [" << iId << "] in city command ["
				 << _rCommand << "] step " << i_GameStep << endl;
			return;
		}
	}

	if (false == b_WatchdogTriggered)
		s_LastCommand << _rCommand << '\n';

	Message* pReply = construct_message ();

	if (LCP_OBSERVE == _rCommand)
		MeasureState (pReply);


	//										
	else if (LCP_SET_TIMEOUT == dqValues [0])
		i_WatchdogTimeout = dqValues [1];

	//										
	else if (LCP_ENABLE_EARLY_END == _rCommand)
		enable_early_end = true;

	//										
	else if (LCP_SEND_WONLOST == _rCommand)
		psoc_CommandConnection->SendBlocking (LCP_GAMEFINISHED LCP_TERMINATOR, 
											 strlen (LCP_GAMEFINISHED LCP_TERMINATOR));


	//										
	else if ((LCP_DISCONNECT == _rCommand) ||
			(LCP_DISCONNECT_INTERNAL == _rCommand))
		disconnect_from_server ();


	//										
	else if ((LCP_CONNECT == _rCommand) ||
			(LCP_CONNECT_INTERNAL == _rCommand))
	{
		char zError [1001];
		while (0 != connect_to_server (user_name, server_host, server_service, zError, 1000))
		{
			cout << "[ERROR] Failed to connect to " << user_name << '@'
				 << server_host << ':' << server_service << '\n'
				 << " : " << zError << endl;
		}

		i_Status = STATE_WAITING_FOR_TURN;
		b_WaitingForTurn = true;

		int iRet = send_chat ("/start");
		if (0 > iRet)
			cout << "[ERROR] send_chat (\"/start\") failed? Ret = " << iRet << endl;

		WaitForRequestCompletion ();
	}

	//										
	else if (LCP_SAVEGAME == dqValues [0])
	{
		if (2 == dqValues.size ())
		{
			if ("" != s_PreviousSaveGame)
				Path::RemoveFile (s_PreviousSaveGame + ".sav");
			// send_chat_printf("/save %s", (const char*) dqValues [1]);
			s_PreviousSaveGame = dqValues [1];

			// i_Status = STATE_WAITING_FOR_TURN | STATE_WAITING_FOR_GAME_SAVE;
			i_Status = STATE_WAITING_FOR_GAME_SAVE;
			b_WaitingForTurn = true;

			String sInternalCommand;
			sInternalCommand << LCP_MOD_SAVEGAME << ' ' << dqValues [1];
			que_InternalCommands.push (sInternalCommand);

			send_chat_printf("/save %s", (const char*) dqValues [1]);
		}
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 2. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_MOD_SAVEGAME == dqValues [0])
	{
		ModifyGameSaveFile (dqValues [1], user_name, enable_early_end);
		psoc_CommandConnection->SendBlocking ("." LCP_TERMINATOR, 2);
	}


	//										
	else if (LCP_ENDGAME == dqValues [0])
	{
		i_Status = STATE_WAITING_FOR_GAME_END;
		b_WaitingForTurn = true;

		send_chat ("/endgame");
	}

	//										
	else if (LCP_LOADGAME == dqValues [0])
	{
		if (2 == dqValues.size ())
		{
			//[SB] Trying to fix bug where first_request_id > last_request_id	
			WaitForRequestCompletion ();
			init_request_ids ();
			//[SB-END]

			// if ((true == game_won) || (true == game_lost))
			if (true == full_reset)
			{
				// SB TEST
				// send_chat ("/endgame");
				// send_chat_printf("/load %s", (const char*) s_LastGameLoaded);
				// send_chat ("/start");
				// WaitForRequestCompletion ();
				// SB TEST

				disconnect_from_server ();
				// sleep (1);

				que_InternalCommands.push (LCP_ENDGAME);

				String sInternalCommand;
				sInternalCommand << LCP_LOADGAME_INTERNAL << ' ' << dqValues [1];
				que_InternalCommands.push (sInternalCommand);

				String sCommand = LCP_CONNECT_INTERNAL;
				ProcessMachineCommand (sCommand);
			}
			else
			{
				String sInternalCommand;
				sInternalCommand << LCP_LOADGAME_INTERNAL << ' ' << dqValues [1];
				que_InternalCommands.push (sInternalCommand);

				i_Status = STATE_WAITING_FOR_GAME_END;
				b_WaitingForTurn = true;

				send_chat ("/endgame");
			}
		}
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 2. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_LOADGAME_INTERNAL == dqValues [0])
	{
		// game_ruleset_free ();
		game_free ();
		game_init();
	
		game_won = false;
		game_lost = false;
		full_reset = false;

		i_GameStep = 0;
		i_Status = STATE_WAITING_FOR_TURN | STATE_WAITING_FOR_GAME_LOAD;
		b_WaitingForTurn = true;

		s_LastGameLoaded = dqValues [1];
		send_chat_printf("/load %s", (const char*) dqValues [1]);
		send_chat ("/start");
	}

	//										
	else if (LCP_GAMESCORE == _rCommand)
		GetGameScore (pReply);

	//										
	else if (LCP_TURNDONE == _rCommand)
	{
		if ((true == game_won) || (true == game_lost))
			psoc_CommandConnection->SendBlocking (LCP_GAMEFINISHED LCP_TERMINATOR, 
												 strlen (LCP_GAMEFINISHED LCP_TERMINATOR));

		else
		{
			WaitForRequestCompletion ();

			++ i_GameStep;
			i_Status = STATE_WAITING_FOR_TURN;
			b_WaitingForTurn = true;

			key_end_turn ();
		}
	}

	//										
	else if (LCP_SET_GOV == dqValues [0])
	{
		if (2 == dqValues.size ())
			SetGovernment (pReply, dqValues [1]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 2. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_RESEARCH == dqValues [0])
	{
		if (2 == dqValues.size ())
			SetResearch (pReply, dqValues [1]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 2. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_RESEARCH_CONTINUE == dqValues [0])
	{
		// nothing to do here...	
	}

	//										
	else if (LCP_UNIT_CONTINUE == dqValues [0])
	{
		// continue means the unit should just continue	
		// doing what it's doing now. So we don't need	
		// to do anything here...						
	}

	//										
	else if (LCP_UNIT_MOVE == dqValues [0])
	{
		if (3 == dqValues.size ())
			UnitMoveMI (pReply, pUnit, dqValues [2]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 3. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_UNIT_GOTO == dqValues [0])
	{
		if (4 == dqValues.size ())
			UnitGotoMI (pReply, pUnit, dqValues [2], dqValues [3]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 4. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_UNIT_AUTOEXPLORE == dqValues [0])
	{
		set_unit_focus (pUnit);
		key_unit_auto_explore ();
	}

	//										
	else if (LCP_UNIT_AUTOSETTLE == dqValues [0])
	{
		set_unit_focus (pUnit);
		key_unit_auto_settle ();
	}

	//										
	else if (LCP_UNIT_BUILDCITY == dqValues [0])
	{
		set_unit_focus (pUnit);
		ask_city_name = false;
		key_unit_build_city ();
	}

	//										
	else if (LCP_UNIT_FORTIFY == dqValues [0])
	{
		if ((ACTIVITY_FORTIFYING != eUnitActivity) &&
			(ACTIVITY_FORTIFIED != eUnitActivity))
		{
			set_unit_focus (pUnit);
			key_unit_fortify ();
		}
	}

	//										
	else if (LCP_UNIT_SETHOME == dqValues [0])
	{
		set_unit_focus (pUnit);
		key_unit_homecity ();
	}

	//										
	else if (LCP_UNIT_IRRIGATE == dqValues [0])
	{
		if (ACTIVITY_IRRIGATE != eUnitActivity)
		{
			set_unit_focus (pUnit);
			key_unit_irrigate ();
		}
	}

	//										
	else if (LCP_UNIT_MINE == dqValues [0])
	{
		if (ACTIVITY_MINE != eUnitActivity)
		{
			set_unit_focus (pUnit);
			key_unit_mine ();
		}
	}

	//										
	else if (LCP_UNIT_BUILDROAD == dqValues [0])
	{
		if ((ACTIVITY_ROAD != eUnitActivity) &&
			(ACTIVITY_RAILROAD != eUnitActivity))
		{
			set_unit_focus (pUnit);
			key_unit_road ();
		}
	}

	//										
	else if (LCP_UNIT_SENTRY == dqValues [0])
	{
		if (ACTIVITY_SENTRY != eUnitActivity)
		{
			set_unit_focus (pUnit);
			key_unit_sentry ();
		}
	}

	//										
	else if (LCP_UNIT_TRANSFORM == dqValues [0])
	{
		if (ACTIVITY_TRANSFORM != eUnitActivity)
		{
			set_unit_focus (pUnit);
			key_unit_transform ();
		}
	}

	//										
	else if (LCP_CITY_SET_GOV == dqValues [0])
	{
		if (3 == dqValues.size ())
			SetCityGovernorMI (pReply, pCity, dqValues [2]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 3. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_CITY_BUILD_CHANGE == dqValues [0])
	{
		if (3 == dqValues.size ())
			SetCityBuildMI (pReply, pCity, dqValues [2]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 3. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_CITY_BUILD_BUY == dqValues [0])
	{
		if (3 == dqValues.size ())
			SetCityBuyMI (pReply, pCity, dqValues [2]);
		else
			Error (pReply, "Invalid number of params in command [%s]. Should be 3. Was %d",
				   _rCommand, dqValues.size ());
	}

	//										
	else if (LCP_CITY_BUILD_CONTINUE == dqValues [0])
	{
		// nothing to do here...	
	}

	//										
	else
	{
		cout << "[ERROR] Unknown command : [" << _rCommand << "], step " << i_GameStep << endl;
	}

	if (0 != pReply->l_Bytes)
	{
		assert (NULL != memchr (pReply->z_Data, LCP_TERMINATOR_CHAR, pReply->l_Bytes));
		if (NULL != psoc_CommandConnection)
		{
			psoc_CommandConnection->SendBlocking (pReply->z_Data, pReply->l_Bytes);
			
			long lBytes = pReply->l_Bytes;
			//if (lBytes > 40)
			//	lBytes = 40;
			char zMsg [lBytes + 1];
			strncpy (zMsg, (char*)pReply->z_Data, lBytes);
			zMsg [lBytes] = '\0';
			// if (true == game_lost)
			//	cout << '[' << zMsg << ']' << endl;
		}
	}
	destroy_message (pReply);
}


//													
void ProcessUserCommand (String& _rCommand)
{
	_rCommand.Strip ();
	_rCommand.LowerCase ();

	Message* pReply = construct_message ();

	unit_selection_clears_orders = false;
	if (".quit" == _rCommand)
		exit (0);
	else if (".interactive mode on" == _rCommand)
		gui_interactive_mode = true;
	else if (".interactive mode off" == _rCommand)
		gui_interactive_mode = false;

	// save, reload & misc		
	else if (true == _rCommand.StartsWith (".set "))
		send_chat (_rCommand);

	else if (".connect" == _rCommand)
	{
		char zError [1001];
		connect_to_server (user_name, server_host, server_service, zError, 1000);
		send_chat ("/start");
		send_chat ("/set saveturns 10000");
	}

	else if (true == _rCommand.StartsWith (".save-game "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (2 == dqValues.size ())
			send_chat_printf("/save %s", dqValues [1]);
	}
	else if (true == _rCommand.StartsWith (".load-game "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (2 == dqValues.size ())
		{
			send_chat ("/endgame");
			send_chat ("/detach");
			disconnect_from_server ();

			char zError [1001];
			connect_to_server (user_name, server_host, server_service, zError, 1000);
			send_chat_printf("/load %s", dqValues [1]);
			send_chat ("/start");

			i_Status = STATE_WAITING_FOR_TURN;
			b_WaitingForTurn = true;
		}
	}

	// observation actions		
	else if (".list nations" == _rCommand)
		ListNations (pReply);
	else if (".list gov" == _rCommand)
		ListGovernment (pReply);
	else if (".list grid" == _rCommand)
		ListGrid (pReply);
	else if (".list units" == _rCommand)
		ListUnits (pReply);
	else if (".list cities" == _rCommand)
		ListCities (pReply);
	else if (".list techs" == _rCommand)
		ListTechnologies (pReply, false);
	else if (".list all techs" == _rCommand)
		ListTechnologies (pReply, true);
	else if (".list city gov" == _rCommand)
		ListCityGovernors (pReply);

	// general game control 	
	else if (".turn-done" == _rCommand)
		key_end_turn ();

	// player actions				
	else if (true == _rCommand.StartsWith (".set-gov "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (2 == dqValues.size ())
			SetGovernment (pReply, atoi (dqValues [1]));
	}
	else if (true == _rCommand.StartsWith (".research "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (2 == dqValues.size ())
			SetResearch (pReply, atoi (dqValues [1]));
	}
	
	// unit actions				
	else if (true == _rCommand.StartsWith (".unit-move "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (3 == dqValues.size ())
			UnitMove (pReply, atoi (dqValues [1]), dqValues [2]);
	}
	else if (true == _rCommand.StartsWith (".unit-goto "))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (4 == dqValues.size ())
			UnitGoto (pReply, atoi (dqValues [1]), atoi (dqValues [2]), atoi (dqValues [3]));
	}
	else if (true == _rCommand.StartsWith (".unit-"))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (2 == dqValues.size ())
			UnitActMisc (pReply, atoi (dqValues [1]), dqValues [0]);
		else if (1 == dqValues.size ())
			UnitActMisc (pReply, -1, dqValues [0]);
	}

	// city actions				
	else if (true == _rCommand.StartsWith (".city-set-gov"))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (3 == dqValues.size ())
			SetCityGovernor (pReply, atoi (dqValues [1]), atoi (dqValues [2]));
	}
	else if (true == _rCommand.StartsWith (".city-"))
	{
		zchar_dq_t dqValues = _rCommand.DestructiveSplit (' ');
		if (3 == dqValues.size ())
			CityBuild (pReply, atoi (dqValues [1]), dqValues [0], dqValues [2]);
		else if (2 == dqValues.size ())
			CityBuild (pReply, atoi (dqValues [1]), dqValues [0], "");
	}


	if (0 != pReply->l_Bytes)
	{
		if (NULL != psoc_CommandConnection)
			psoc_CommandConnection->SendBlocking (pReply->z_Data, pReply->l_Bytes);
	}
	destroy_message (pReply);
}


//													
void SendSocketMessage (const char* _zMessage)
{
	if (NULL != psoc_CommandConnection)
		psoc_CommandConnection->SendBlocking (_zMessage, strlen (_zMessage));
}


//													
void ModifyGameSaveFile (const char* _zFileName, const char* _zUserName, bool _bEndCheck)
{
	String sFileName;
	sFileName << _zFileName << ".sav";

	String_dq_t dqLines;
	if (false == File::ReadLines (sFileName, dqLines))
	{
		cout << "[ERROR] Unable to read save file [" << sFileName
			 << "]. ModifyGameSaveFile failed." << endl;
		return;
	}

	File file;
	if (false == file.Open (sFileName, ios_base::out))
	{
		cout << "[ERROR] Unable to open save file [" << sFileName
			 << "] for writing. ModifyGameSaveFile failed." << endl;
		return;
	}

	if (true == _bEndCheck)
	{
		bool bInUnitsSection = false;
		bool bInMySection = false;
		bool bInOpponentSection = false;
		bool bIHaveCities = false;
		bool bOpponentHasCities = false;
		ITERATE (String_dq_t, dqLines, ite)
		{
			if ((true == bIHaveCities) && (true == bOpponentHasCities))
				break;

			if (true == (*ite).StartsWith ("username=\""))
			{
				bInMySection = (*ite).Has (_zUserName);
				bInOpponentSection = ! bInMySection;
				continue;
			}

			// check if player has cities ...	
			if (true == (*ite).StartsWith ("ncities="))
			{
				if (true == bInMySection)
					bIHaveCities = ("ncities=0" != *ite);
				if (true == bInOpponentSection)
					bOpponentHasCities = ("ncities=0" != *ite);
				continue;
			}

			// units section...	
			if (true == (*ite).StartsWith ("u={"))
			{
				bInUnitsSection = true;
				continue;
			}
			if (false == bInUnitsSection)
				continue;
			if (true == (*ite).StartsWith ("}"))
			{
				bInUnitsSection = false;
				continue;
			}

			// check if player has settlers ...
			if (true == (*ite).Has ("\"Settlers\""))
			{
				if (true == bInMySection)
					bIHaveCities = true;
				else if (true == bInOpponentSection)
					bOpponentHasCities = true;
			}
		}

		if (false == bIHaveCities)
			game_lost = true;
		if (false == bOpponentHasCities)
			game_won = true;

		if ((true == game_won) || (true == game_lost))
			cout << "detected early end! g" << game_won << ", l" << game_lost << endl;
	}

	bool bInOpponentSection = false;
	ITERATE (String_dq_t, dqLines, ite)
	{
		if (true == (*ite).StartsWith ("username=\""))
		{
			if (false == (*ite).Has (_zUserName))
			{
				bInOpponentSection = true;
				file.WriteLine ("username=\"Unassigned\"");
			}
			else
				file.WriteLine (*ite);
			continue;
		}

		if (false == bInOpponentSection)
		{
			file.WriteLine (*ite);
			continue;
		}
		if ("ai.control=0" == *ite)
		{
			file.WriteLine ("ai.control=1");
			bInOpponentSection = false;
		}
		else
			file.WriteLine (*ite);
	}
	file.Close ();
}





struct tile
{
	int x, y;
};

typedef queue<tile*>	Tile_que_t;
typedef set<tile*>		Tile_set_t;

extern "C"
{
//													
void GetUnitGotoActions (struct Message* _pResponse,
						 int _iUnitId,
						 const char* _zUnitName,
						 struct pf_map* _pPathMap,
						 int _x, int _y)
{
	Tile_que_t	queTile;
	Tile_set_t	setTile;
	setTile.insert (native_pos_to_tile (_x, _y));

	queTile.push (native_pos_to_tile (_x - 1, _y - 1));
	queTile.push (native_pos_to_tile (_x - 1, _y));
	queTile.push (native_pos_to_tile (_x - 1, _y + 1));
	queTile.push (native_pos_to_tile (_x, _y - 1));
	queTile.push (native_pos_to_tile (_x, _y + 1));
	queTile.push (native_pos_to_tile (_x + 1, _y - 1));
	queTile.push (native_pos_to_tile (_x + 1, _y));
	queTile.push (native_pos_to_tile (_x + 1, _y + 1));

	char bOpponentPresent [9];
	OpponentUnitDirections (_x, _y, bOpponentPresent);

	while (false == queTile.empty ())
	{
		tile* pTile = queTile.front ();
		queTile.pop ();
		if (NULL == pTile)
			continue;

		if (setTile.end () != setTile.find (pTile))
			continue;
		setTile.insert (pTile);

		struct pf_path* pPath = pf_map_get_path (_pPathMap, pTile);
		if (NULL == pPath)
			continue;

		pf_position* pLastPos = &pPath->positions [pPath->length - 1];
		assert (pLastPos->tile == pTile);
		int iMax = pLastPos->turn;
		if (iMax > 1)
		{
			pf_path_destroy (pPath);
			continue;
		}
		pf_path_destroy (pPath);

		const char* zTileLabels = GetTileLabels (pTile->x, pTile->y, NULL);

		int x = pTile->x;
		int y = pTile->y;

		int dx = x - _x;
		int dy = y - _y;
		dx = (dx > 0)? 2 : ((dx < 0)? 0 : 1);
		dy = (dy > 0)? 2 : ((dy < 0)? 0 : 1);

		char zOpponentPresent [100];
		*zOpponentPresent = '\0';
		if (0 != bOpponentPresent [3 * dx + dy])
		{
			if ('\0' == *zTileLabels)
				sprintf (zOpponentPresent, "nmy_dir,nmy_dir_%s", _zUnitName);
				// pOpponentPresent = "nmy_dir";
			else
				sprintf (zOpponentPresent, "nmy_dir,nmy_dir_%s,", _zUnitName);
				// pOpponentPresent = "nmy_dir,";
		}
		char zText [10001];
		sprintf (zText, LCP_UNIT_GOTO " %d %d %d" 
						LCP_COMMAND_TERMINATOR " " LCP_COMMAND_TERMINATOR
						"goto;%s%s" LCP_ACTION_TERMINATOR, 
				 _iUnitId, x, y, zOpponentPresent, zTileLabels);

		assert (strlen (zText) < 10000);
		append_to_message (_pResponse, zText);

		queTile.push (native_pos_to_tile (x - 1, y - 1));
		queTile.push (native_pos_to_tile (x - 1, y));
		queTile.push (native_pos_to_tile (x - 1, y + 1));
		queTile.push (native_pos_to_tile (x, y - 1));
		queTile.push (native_pos_to_tile (x, y + 1));
		queTile.push (native_pos_to_tile (x + 1, y - 1));
		queTile.push (native_pos_to_tile (x + 1, y));
		queTile.push (native_pos_to_tile (x + 1, y + 1));
	}
}
}


