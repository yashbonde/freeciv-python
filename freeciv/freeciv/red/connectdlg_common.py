# python version of client/connectdlg_common.c

# constants
WAIT_BETWEEN_TRIES = 100000 # microsecs
NUMBER_OF_TRIES = 500


'''
General chain of events
Two distinct paths are taken depeneding on the choice of mode:
* If the user selects multi-player mode, then a 'packet_req_join_game' packet
  is sent to the server. It is either successful or not. The End.
* If the user selects single-player mode (either new game or save game) then:
   1. the packet_req_join_game is sent
   2. on recipt, if we can join, then a challenge packet is sent to the server,
      so we can get hack level control
   3. if we can't get hack, then we got dumped to multi-player mode. If we can,
      then:
        a. for a new game, we send a series of packet_generic_messages packets
           with commands to start the game
        b. for a saved game, we send a load command with a packet_generic_messages
           then we send a PACKET_PLAYER_LIST_REQUEST. the response to this request
           will tell us if the game was loaded or not. if not, then we sent
           another load command. is so, then we send a series of packet_generic_messages
           packets wih commands to start the game.
'''

class connectldg_common():

    def can_client_access_hack(self):
        return self.client_has_hack

    def client_kill_server(self):
        '''
        Kills server if client has started it. if 'force' parameter is unset, we just
        do '/quit'. if it's set, then we will send signal to server to kil it (use this
        when the socket is already disconnected already)
        '''
        if self.server_quitting and self.server_pid > 0:
            # already asked to quit
            if waitpid(self.server_pid, None, WUNTRACED) <= 0:
                kill(self.server_pid, SIGTERM)
                waitpid(self.server_pid, None, WUNTRACED)
            self.server_pid = -1
            self.server_quitting = False

        if self.server_quitting and server_process != INVALID_HANDLE_VALUE:
            TerminateProcess(server_process, 0);
            CloseHandle(server_process)
            if self.loghandle != INVALID_HANDLE_VALUE:
                CloseHandle(self.loghandle)

            server_process = INVALID_HANDLE_VALUE
            self.loghandle = INVALID_HANDLE_VALUE
            self.server_quitting = False

        if is_server_running():
            if client.conn.used and client_has_hack:
                '''
                This is the 'soft' shutdown of server by sending /quit. This is useful
                when closing the client or disconnecting becuase it doesn't kill server
                prematurely. In particular, killing the server in middle of save can have
                disastrous results. This method tells the server to quit on it's own. This
                is safer from game perspective, but more dangerous because if we the kill
                failes the server will be left running.

                Another potential problem is because this function is called at exit
                it could potentially be called when we're connected to an unowned server.
                In this case we don't want to kill it.
                '''
                send_chat('/quit')
                self.server_quitting = True
            
            elif (force):
                '''
                Either we already disconnected, or we didn't get control of the 
                server. In either case, the only thign to do it a 'hard' kill of the
                server.
                '''
                kill(self.server_pid, SIGTERM)
                waitpid(self.server_pid, None, WUNTRACED)
                self.server_pid = -1
                TerminateProcess(server_process, 0)
                CloseHandle(server_process)
                if self.loghandle != INVALID_HANDLE_VALUE:
                    CloseHandle(self.loghandle)

                server_process = INVALID_HANDLE_VALUE
                self.loghandle = INVALID_HANDLE_VALUE
                self.server_quitting = False

        self.client_has_hack = False

    def client_start_sercer(self):
        '''
        Forks a server if it can.
        Returns:
            False if we find we couldn't start the server
        '''

        if not HAVE_USABLE_FORK and not FREECIV_MSWINDOWS:
            return False

        connect_tries = 0 # int
        buf = '' # char
        saves_dir = '' # char
        scens_dir = '' # char
        storage = None # char*
        ruleset = None # char*

