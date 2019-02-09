'''
fc_iomanager.py

This file has the FCIOManager class

@yashbonde 16.01.2019
'''

# import dependencies
import json

# custom
from .dummy_handler import DummyIOHandler

class FCIOManager():
    '''
    IOM Base has the conneci
    '''
    def __init__(self):
        self.mode = 'DUMMY' # right now this is only in dummy mode
        self.game_done = False


    def est_conn(self, username, server_ip, server_port):
        # function to establish the connection to server
        # here we also setup the IOManager
        self.IOManager = DummyIOHandler(username, server_ip, server_port)
        self.username = username
        self.server_ip = server_ip
        self.server_port = server_port

    '''
    Functions that IOM_Base must have
    '''
    def send(self, action_dict):
        # function to pass the input string to the server
        string = json.dumps(action_dict)
        self.IOManager.send_json(string)

    def get_observations(self):
        self.game_done = self.IOManager.game_done
        state_string = self.IOManager.get_latest_state()
        action_string = self.IOManager.get_latest_action()
        # return the JSON parsed state dict
        state_ = json.loads(state_string)
        action_ = json.loads(action_string)
        return state_, action_, self.game_done

    def save_state(self, path, key = None):
        if key == 'state':
            self.IOManager.save_state(path)
            return
        elif key == 'action':
            self.IOManager.save_action(path)
            return

        self.IOManager.save_state(path)
        self.IOManager.save_action(path)

    def reset(self):
        self.IOManager.reset()
        

